#include "screenshotrenderer.h"

#include "graph/graph.h"

#include "graphcomponentscene.h"
#include "graphoverviewscene.h"
#include "shared/utils/preferences.h"
#include "ui/document.h"
#include "ui/visualisations/elementvisual.h"

#include <QBuffer>
#include <QDir>

static const int TILE_SIZE = 1024;

static QMatrix4x4 subViewportMatrix(QRectF scaledDimensions, QSize screenshotSize)
{
    QMatrix4x4 projOffset;

    float xTranslation =
        (static_cast<float>(scaledDimensions.x() * 2 + scaledDimensions.width()) / screenshotSize.width()) - 1.0f;
    float yTranslation =
        (static_cast<float>(scaledDimensions.y() * 2 + scaledDimensions.height()) / screenshotSize.height()) - 1.0f;
    projOffset.translate(xTranslation, -yTranslation);

    float xScale = static_cast<float>(scaledDimensions.width()) / screenshotSize.width();
    float yScale = static_cast<float>(scaledDimensions.height()) / screenshotSize.height();
    projOffset.scale(xScale, yScale);

    return projOffset;
}

static QString fetchPreview(QSize screenshotSize)
{
    int pixelCount = screenshotSize.width() * screenshotSize.height() * 4;
    std::vector<GLubyte> pixels(pixelCount);
    glReadPixels(0, 0, screenshotSize.width(), screenshotSize.height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    QImage screenTile(pixels.data(), screenshotSize.width(), screenshotSize.height(), QImage::Format_RGBA8888);
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    screenTile.mirrored().save(&buffer, "PNG");

    // QML Can't load raw QImages so as a hack we just base64 encode a png
    return QString::fromLatin1(byteArray.toBase64().data());
}


static void fetchAndDrawTile(QPixmap& fullScreenshot, QSize screenshotSize, int currentTileX, int currentTileY)
{
    int pixelCount = TILE_SIZE * TILE_SIZE * 4;
    std::vector<GLubyte> pixels(pixelCount);
    glReadPixels(0, 0, TILE_SIZE, TILE_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    QImage screenTile(pixels.data(), TILE_SIZE, TILE_SIZE, QImage::Format_RGBA8888);

    QPainter painter(&fullScreenshot);
    painter.drawImage(currentTileX * TILE_SIZE,
                      (screenshotSize.height() - TILE_SIZE) - (currentTileY * TILE_SIZE),
                      screenTile.mirrored(false, true));
}

ScreenshotRenderer::ScreenshotRenderer()
{
    glGenFramebuffers(1, &_screenshotFBO);
    glGenTextures(1, &_screenshotTex);
    glGenTextures(1, &_sdfTexture);
}

ScreenshotRenderer::~ScreenshotRenderer()
{
    glDeleteTextures(1, &_sdfTexture);
    glDeleteTextures(1, &_screenshotTex);
    glDeleteFramebuffers(1, &_screenshotFBO);
}

void ScreenshotRenderer::requestPreview(const GraphRenderer& renderer, int width, int height, bool fillSize)
{
    copyState(renderer);

    QSize screenshotSize(width, height);

    float viewportAspectRatio = static_cast<float>(renderer.width()) / static_cast<float>(renderer.height());

    if(!fillSize)
        screenshotSize.setHeight(static_cast<float>(width) / viewportAspectRatio);

    if(!resize(screenshotSize.width(), screenshotSize.height()))
    {
        qWarning() << "Attempting to render incomplete FBO";
        return;
    }

    // Update Scene
    updateComponentGPUData(ScreenshotType::Preview, screenshotSize, {renderer.width(), renderer.height()});
    render();

    const QString& base64Image = fetchPreview(screenshotSize);
    emit previewComplete(base64Image);
}

void ScreenshotRenderer::render()
{
    glViewport(0, 0, this->width(), this->height());

    glBindTexture(GL_TEXTURE_2D, _screenshotTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width(), this->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    renderGraph();

    render2D();

    // Bind to screenshot FBO
    glBindFramebuffer(GL_FRAMEBUFFER, _screenshotFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _screenshotTex, 0);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, static_cast<GLenum*>(drawBuffers));

    renderToFramebuffer();
}

void ScreenshotRenderer::requestScreenshot(const GraphRenderer& renderer, int width, int height,
                                           const QString& path, int dpi, bool fillSize)
{
    copyState(renderer);

    float viewportAspectRatio = static_cast<float>(renderer.width()) / static_cast<float>(renderer.height());

    QSize screenshotSize(width, height);

    if(!fillSize)
    {
        screenshotSize.setHeight(static_cast<float>(width) / viewportAspectRatio);
        if(screenshotSize.height() > height)
        {
            screenshotSize.setWidth(static_cast<float>(height) * viewportAspectRatio);
            screenshotSize.setHeight(height);
        }
    }

    if(!resize(TILE_SIZE, TILE_SIZE))
    {
        qWarning() << "Attempting to render incomplete FBO";
        return;
    }

    // Need a pixmap to construct the full image
    auto fullScreenshot = QPixmap(screenshotSize.width(), screenshotSize.height());

    auto tileXCount = std::ceil(static_cast<float>(screenshotSize.width()) / this->width());
    auto tileYCount = std::ceil(static_cast<float>(screenshotSize.height()) / this->height());
    for(auto currentTileX = 0; currentTileX < tileXCount; currentTileX++)
    {
        for(auto currentTileY = 0; currentTileY < tileYCount; currentTileY++)
        {
            updateComponentGPUData(ScreenshotType::Tile, screenshotSize, {renderer.width(), renderer.height()},
                                   currentTileX, currentTileY);
            render();
            fetchAndDrawTile(fullScreenshot, screenshotSize, currentTileX, currentTileY);
        }
    }

    auto image = fullScreenshot.toImage();

    const double INCHES_PER_METER = 39.3700787;
    image.setDotsPerMeterX(dpi * INCHES_PER_METER);
    image.setDotsPerMeterY(dpi * INCHES_PER_METER);
    emit screenshotComplete(image, path);
}

void ScreenshotRenderer::updateComponentGPUData(ScreenshotType screenshotType, QSize screenshotSize, QSize viewportSize, int currentTileX, int currentTileY)
{
    std::vector<GLfloat> componentData;

    double scaleX = static_cast<double>(screenshotSize.width()) / viewportSize.width();
    double scaleY = static_cast<double>(screenshotSize.height()) / viewportSize.height();

    for(size_t componentIndex = 0; componentIndex < _componentCameras.size(); componentIndex++)
    {
        Camera& componentCamera = _componentCameras.at(componentIndex);
        QRectF& componentViewport = _componentViewports.at(componentIndex);

        QRectF scaledDimensions;
        scaledDimensions.setTopLeft({componentViewport.x() * scaleX, componentViewport.y() * scaleY});
        scaledDimensions.setWidth(componentViewport.width() * scaleX);
        scaledDimensions.setHeight(componentViewport.height() * scaleY);

        float aspectRatio =
            static_cast<float>(scaledDimensions.width()) / static_cast<float>(scaledDimensions.height());
        auto _fovy = 60.0f;
        componentCamera.setPerspectiveProjection(_fovy, aspectRatio, 0.3f, 50000.0f);
        componentCamera.setViewportWidth(scaledDimensions.width());
        componentCamera.setViewportHeight(scaledDimensions.height());

        // Model View
        for(int i = 0; i < 16; i++)
            componentData.push_back(componentCamera.viewMatrix().data()[i]);

        // Projection
        if(screenshotType == ScreenshotType::Tile)
        {
            // Calculate tile projection matrix
            QMatrix4x4 translation;
            QMatrix4x4 projMatrix;

            float xTranslation =
                (static_cast<float>(scaledDimensions.x() * 2.0 + scaledDimensions.width()) / TILE_SIZE) -
                (static_cast<float>(screenshotSize.width()) / TILE_SIZE);
            float yTranslation =
                (static_cast<float>(scaledDimensions.y() * 2.0 + scaledDimensions.height()) / TILE_SIZE) -
                (static_cast<float>(screenshotSize.height()) / TILE_SIZE);
            translation.translate(xTranslation, -yTranslation);

            float xScale = static_cast<float>(scaledDimensions.width()) / TILE_SIZE;
            float yScale = static_cast<float>(scaledDimensions.height()) / TILE_SIZE;
            translation.scale(xScale, yScale);
            projMatrix = translation * componentCamera.projectionMatrix();

            // Per-Tile translation for high res screenshots
            float tileWidthRatio = static_cast<float>(TILE_SIZE) / screenshotSize.width();
            float tileHeightRatio = static_cast<float>(TILE_SIZE) / screenshotSize.height();

            float tileTranslationX = ((1.0f / tileWidthRatio) - 1.0f) - (2.0f * currentTileX);
            float tileTranslationY = ((1.0f / tileHeightRatio) - 1.0f) - (2.0f * currentTileY);

            QMatrix4x4 tileTranslation;
            tileTranslation.translate(tileTranslationX, tileTranslationY, 0.0f);

            projMatrix = tileTranslation * projMatrix;

            for(int i = 0; i < 16; i++)
                componentData.push_back(projMatrix.data()[i]);
        }
        else
        {
            auto projectionMatrix = subViewportMatrix(scaledDimensions, screenshotSize) * componentCamera.projectionMatrix();

            // Normal projection
            for(int i = 0; i < 16; i++)
                componentData.push_back(projectionMatrix.data()[i]);
        }
    }

    glBindBuffer(GL_TEXTURE_BUFFER, componentDataTBO());
    glBufferData(GL_TEXTURE_BUFFER, componentData.size() * sizeof(GLfloat), componentData.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

bool ScreenshotRenderer::copyState(const GraphRenderer& renderer)
{
    _componentCameras.clear();
    _componentViewports.clear();

    for(size_t i = 0; i < renderer._gpuGraphData.size(); ++i)
        _gpuGraphData.at(i).copyState(renderer._gpuGraphData.at(i), _nodesShader, _edgesShader, _textShader);

    for(auto& componentRendererRef : renderer.componentRenderers())
    {
        const GraphComponentRenderer* componentRenderer = componentRendererRef;

        _componentCameras.emplace_back(*componentRenderer->camera());
        _componentViewports.emplace_back(componentRenderer->dimensions());
    }

    // Just copy the SDF texture
    GLuint textureFBO = 0;
    glGenFramebuffers(1, &textureFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, textureFBO);

    int renderWidth = 0;
    int renderHeight = 0;

    glBindTexture(GL_TEXTURE_2D_ARRAY, renderer.sdfTexture());
    glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &renderWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_HEIGHT, &renderHeight);

    if(!renderer._glyphMap->images().empty())
    {
        // SDF texture
        glBindTexture(GL_TEXTURE_2D_ARRAY, _sdfTexture);

        // Generate FBO texture
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, renderWidth, renderHeight,
                     static_cast<GLsizei>(renderer._glyphMap->images().size()), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        for(int layer = 0; layer < static_cast<int>(renderer._glyphMap->images().size()); layer++)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderer.sdfTexture(), 0, layer);

            GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, DrawBuffers);

            glBindFramebuffer(GL_FRAMEBUFFER, textureFBO);
            glViewport(0, 0, renderWidth, renderHeight);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, 0, 0, renderWidth, renderHeight);
        }
    }

    glDeleteFramebuffers(1, &textureFBO);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glReadBuffer(GL_BACK);

    uploadGPUGraphData();

    return true;
}

GLuint ScreenshotRenderer::sdfTexture() const { return _sdfTexture; }
