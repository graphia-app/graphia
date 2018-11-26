#include "screenshotrenderer.h"

#include "graph/graph.h"

#include "graphcomponentscene.h"
#include "graphoverviewscene.h"
#include "shared/utils/preferences.h"
#include "ui/document.h"
#include "ui/visualisations/elementvisual.h"

#include <QBuffer>
#include <QDir>


ScreenshotRenderer::ScreenshotRenderer(GraphRenderer &renderer) :
    GraphRendererCore(renderer),
    _graphModel(renderer.graphModel()),
    _viewportWidth(renderer.width()),
    _viewportHeight(renderer.height()),
    _componentCameras(renderer.graphModel()->graph()),
    _componentViewports(renderer.graphModel()->graph())
{
    for(ComponentId componentId : _graphModel->graph().componentIds())
    {
        Camera* camera = _componentCameras.at(componentId);
        *camera = *renderer.componentRendererForId(componentId)->camera();
        QRectF& rect = _componentViewports.at(componentId);
        rect = renderer.componentRendererForId(componentId)->dimensions();
    }

    glGenFramebuffers(1, &_screenshotFBO);
    glGenTextures(1, &_screenshotTex);

    // Just copy the SDF texture
    GLuint textureFBO = 0;
    glGenFramebuffers(1, &textureFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, textureFBO);

    _sdfTexture = 0;
    glGenTextures(1, &_sdfTexture);

    int renderWidth = 0;
    int renderHeight = 0;

    glBindTexture(GL_TEXTURE_2D_ARRAY, renderer.sdfTexture());
    glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &renderWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_HEIGHT, &renderHeight);

    if(renderer._glyphMap->images().size() > 0)
    {
        // SDF texture
        glBindTexture(GL_TEXTURE_2D_ARRAY, _sdfTexture);

        // Generate FBO texture
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
                     renderWidth, renderHeight, renderer._glyphMap->images().size(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        for(int layer = 0; layer < static_cast<int>(renderer._glyphMap->images().size()); layer++)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderer.sdfTexture(), 0, layer);

            GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, DrawBuffers);

            glBindFramebuffer(GL_FRAMEBUFFER, textureFBO);
            glViewport(0, 0, renderWidth, renderHeight);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, 0, 0, renderWidth, renderHeight);
        }
    }

    glDeleteFramebuffers(1, &textureFBO);

    uploadGPUGraphData();
}

ScreenshotRenderer::~ScreenshotRenderer()
{
    glDeleteFramebuffers(1, &_screenshotFBO);
    glDeleteTextures(1, &_sdfTexture);
    glDeleteTextures(1, &_screenshotTex);
}

void ScreenshotRenderer::onPreviewRequested(int width, int height, bool fillSize)
{
    _isPreview = true;

    _screenshotWidth = width;
    _screenshotHeight = height;

    float viewportAspectRatio = static_cast<float>(this->width()) / static_cast<float>(this->height());

    if(!fillSize)
        _screenshotHeight = static_cast<float>(width) / viewportAspectRatio;

    _FBOcomplete = resize(_screenshotWidth, _screenshotHeight);

    glBindTexture(GL_TEXTURE_2D, _screenshotTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width(), this->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    render();

    _isPreview = false;
}

void ScreenshotRenderer::render()
{
    if(!_FBOcomplete)
    {
        qWarning() << "Attempting to render incomplete FBO";
        return;
    }

    glViewport(0, 0, width(), height());

    // Update Scene
    updateComponentGPUData();

    renderGraph();

    render2D();

    // Bind to screenshot FBO
    glBindFramebuffer(GL_FRAMEBUFFER, _screenshotFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _screenshotTex, 0);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, static_cast<GLenum*>(drawBuffers));

    renderToFramebuffer();

    if(_isScreenshot)
    {
        int pixelCount = width() * height() * 4;
        std::vector<GLubyte> pixels(pixelCount);

        glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        QImage screenTile(pixels.data(), width(), height(), QImage::Format_RGBA8888);

        QPainter painter(&_fullScreenshot);
        painter.drawImage(_currentTileX * TILE_SIZE,
                          (_screenshotHeight - TILE_SIZE) - (_currentTileY * TILE_SIZE),
                          screenTile.mirrored(false, true));
    }
    else if(_isPreview)
    {
        int pixelCount = width() * height() * 4;
        std::vector<GLubyte> pixels(pixelCount);

        glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        QImage screenTile(pixels.data(), width(), height(), QImage::Format_RGBA8888);
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        screenTile.mirrored().save(&buffer, "PNG");
        // QML Can't load raw QImages so as a hack we just base64 encode a png
        emit previewComplete(QString::fromLatin1(byteArray.toBase64().data()));
    }
}

void ScreenshotRenderer::onScreenshotRequested(int width, int height, const QString& path, int dpi, bool fillSize)
{
    _isScreenshot = true;

    float viewportAspectRatio = static_cast<float>(_viewportWidth) / static_cast<float>(_viewportHeight);

    _screenshotWidth = width;
    _screenshotHeight = height;

    if(!fillSize)
    {
        _screenshotHeight = static_cast<float>(width) / viewportAspectRatio;
        if(_screenshotHeight > height)
        {
            _screenshotWidth = static_cast<float>(height) * viewportAspectRatio;
            _screenshotHeight = height;
        }
    }

    _FBOcomplete = resize(TILE_SIZE, TILE_SIZE);

    // Need a pixmap to construct the full image
    _fullScreenshot = QPixmap(_screenshotWidth, _screenshotHeight);

    glBindTexture(GL_TEXTURE_2D, _screenshotTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width(), this->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    _tileXCount = std::ceil(static_cast<float>(_screenshotWidth) / this->width());
    _tileYCount = std::ceil(static_cast<float>(_screenshotHeight) / this->height());
    for(_currentTileX = 0; _currentTileX < _tileXCount; _currentTileX++)
    {
        for(_currentTileY = 0; _currentTileY < _tileYCount; _currentTileY++)
        {
            render();
        }
    }

    auto image = _fullScreenshot.toImage();

    // Destroy the pixmap, not needed anymore!
    _fullScreenshot = {};

    const double INCHES_PER_METER = 39.3700787;
    image.setDotsPerMeterX(dpi * INCHES_PER_METER);
    image.setDotsPerMeterY(dpi * INCHES_PER_METER);
    emit screenshotComplete(image, path);

    _isScreenshot = false;
}

QMatrix4x4 ScreenshotRenderer::subViewportMatrix(QRectF scaledDimensions)
{
    QMatrix4x4 projOffset;

    float xTranslation = (static_cast<float>(scaledDimensions.x() * 2 + scaledDimensions.width()) / _screenshotWidth) - 1.0f;
    float yTranslation = (static_cast<float>(scaledDimensions.y() * 2 + scaledDimensions.height()) / _screenshotHeight) - 1.0f;
    projOffset.translate(xTranslation, -yTranslation);

    float xScale = static_cast<float>(scaledDimensions.width()) / _screenshotWidth;
    float yScale = static_cast<float>(scaledDimensions.height()) / _screenshotHeight;
    projOffset.scale(xScale, yScale);

    return projOffset;
}

void ScreenshotRenderer::updateComponentGPUData()
{
    //FIXME this doesn't necessarily need to be entirely regenerated and rebuffered
    // every frame, so it makes sense to do partial updates as and when required.
    // OTOH, it's probably not ever going to be masses of data, so maybe we should
    // just suck it up; need to get a profiler on it and see how long we're spending
    // here transfering the buffer, when there are lots of components
    std::vector<GLfloat> componentData;

    double scaleX = static_cast<double>(_screenshotWidth) / _viewportWidth;
    double scaleY = static_cast<double>(_screenshotHeight) / _viewportHeight;

    for(ComponentId componentId : _graphModel->graph().componentIds())
    {
        Camera* componentCamera = _componentCameras.at(componentId);
        QRectF componentViewport = _componentViewports.at(componentId);

        QRectF scaledDimensions;
        scaledDimensions.setTopLeft({componentViewport.x() * scaleX, componentViewport.y() * scaleY});
        scaledDimensions.setWidth(componentViewport.width() * scaleX);
        scaledDimensions.setHeight(componentViewport.height() * scaleY);

        float aspectRatio = static_cast<float>(scaledDimensions.width()) / static_cast<float>(scaledDimensions.height());
        auto _fovy = 60.0f;
        componentCamera->setPerspectiveProjection(_fovy, aspectRatio, 0.3f, 50000.0f);
        componentCamera->setViewportWidth(scaledDimensions.width());
        componentCamera->setViewportHeight(scaledDimensions.height());

        if(componentCamera == nullptr)
        {
            qWarning() << "null component camera";
            continue;
        }

        // Model View
        for(int i = 0; i < 16; i++)
            componentData.push_back(componentCamera->viewMatrix().data()[i]);

        // Projection
        if(_isScreenshot)
        {
            // Calculate tile projection matrix
            QMatrix4x4 translation;
            QMatrix4x4 projMatrix;

            float xTranslation = (static_cast<float>(scaledDimensions.x() * 2.0 + scaledDimensions.width() ) / TILE_SIZE) - (static_cast<float>(_screenshotWidth) / TILE_SIZE);
            float yTranslation = (static_cast<float>(scaledDimensions.y() * 2.0 + scaledDimensions.height()) / TILE_SIZE) - (static_cast<float>(_screenshotHeight) / TILE_SIZE);
            translation.translate(xTranslation, -yTranslation);

            float xScale = static_cast<float>(scaledDimensions.width()) / TILE_SIZE;
            float yScale = static_cast<float>(scaledDimensions.height()) / TILE_SIZE;
            translation.scale(xScale, yScale);
            projMatrix = translation * componentCamera->projectionMatrix();

            // Per-Tile translation for high res screenshots
            float tileWidthRatio = static_cast<float>(TILE_SIZE) / _screenshotWidth;
            float tileHeightRatio = static_cast<float>(TILE_SIZE) / _screenshotHeight;

            float tileTranslationX = ((1.0f / tileWidthRatio) - 1.0f) - (2.0f * _currentTileX);
            float tileTranslationY = ((1.0f / tileHeightRatio) - 1.0f) - (2.0f * _currentTileY);

            QMatrix4x4 tileTranslation;
            tileTranslation.translate(tileTranslationX, tileTranslationY, 0.0f);

            projMatrix = tileTranslation * projMatrix;

            for(int i = 0; i < 16; i++)
                componentData.push_back(projMatrix.data()[i]);
        }
        else
        {
            auto projectionMatrix = subViewportMatrix(scaledDimensions) * componentCamera->projectionMatrix();

            // Normal projection
            for(int i = 0; i < 16; i++)
                componentData.push_back(projectionMatrix.data()[i]);
        }
    }

    glBindBuffer(GL_TEXTURE_BUFFER, componentDataTBO());
    glBufferData(GL_TEXTURE_BUFFER, componentData.size() * sizeof(GLfloat), componentData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

GLuint ScreenshotRenderer::sdfTexture() const
{
    return _sdfTexture;
}
