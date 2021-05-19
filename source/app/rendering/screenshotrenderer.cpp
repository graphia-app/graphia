/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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

// With some of the post-process fragment shaders, rendering at the edges may
// cause artefacts. Normally this isn't much of a problem, but when tile rendering,
// said artefacts show up at every tile boundary. The solution is to overscan the
// tile slightly, but only use the centre pixels when composing the final image.
static const int TILE_EXTRA = 2;
static const int TILE_SIZE_PLUS_EXTRA = TILE_SIZE + (2 * TILE_EXTRA);

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


static void fetchAndDrawTile(QPixmap& fullScreenshot, int tileX, int tileY)
{
    std::vector<GLubyte> pixelBytes(TILE_SIZE * TILE_SIZE * 4);
    glReadPixels(TILE_EXTRA, TILE_EXTRA, TILE_SIZE, TILE_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, pixelBytes.data());

    QImage screenTile(pixelBytes.data(), TILE_SIZE, TILE_SIZE, QImage::Format_RGBA8888);

    QPainter painter(&fullScreenshot);
    painter.drawImage(tileX * TILE_SIZE, tileY * TILE_SIZE, screenTile.mirrored(false, true));
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
        screenshotSize.setHeight(static_cast<int>(static_cast<float>(width) / viewportAspectRatio));

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

    renderToFramebuffer(GraphRendererCore::Type::Color);
}

void ScreenshotRenderer::requestScreenshot(const GraphRenderer& renderer, int width, int height,
                                           const QString& path, int dpi, bool fillSize)
{
    copyState(renderer);

    float viewportAspectRatio = static_cast<float>(renderer.width()) / static_cast<float>(renderer.height());

    QSize screenshotSize(width, height);

    if(!fillSize)
    {
        screenshotSize.setHeight(static_cast<int>(static_cast<float>(width) / viewportAspectRatio));
        if(screenshotSize.height() > height)
        {
            screenshotSize.setWidth(static_cast<int>(static_cast<float>(height) * viewportAspectRatio));
            screenshotSize.setHeight(height);
        }
    }

    if(!resize(TILE_SIZE_PLUS_EXTRA, TILE_SIZE_PLUS_EXTRA))
    {
        qWarning() << "Attempting to render incomplete FBO";
        return;
    }

    // Need a pixmap to construct the full image
    auto fullScreenshot = QPixmap(screenshotSize.width(), screenshotSize.height());

    auto tileXCount = static_cast<int>(std::ceil(static_cast<float>(screenshotSize.width()) / static_cast<float>(TILE_SIZE)));
    auto tileYCount = static_cast<int>(std::ceil(static_cast<float>(screenshotSize.height()) / static_cast<float>(TILE_SIZE)));
    for(auto tileY = 0; tileY < tileYCount; tileY++)
    {
        for(auto tileX = 0; tileX < tileXCount; tileX++)
        {
            updateComponentGPUData(ScreenshotType::Tile, screenshotSize,
                {renderer.width(), renderer.height()}, tileX, tileY);

            render();
            fetchAndDrawTile(fullScreenshot, tileX, tileY);
        }
    }

    auto image = fullScreenshot.toImage();

    const int DOTS_PER_METER = static_cast<int>(dpi * 39.3700787);
    image.setDotsPerMeterX(DOTS_PER_METER);
    image.setDotsPerMeterY(DOTS_PER_METER);
    emit screenshotComplete(image, path);
}

void ScreenshotRenderer::updateComponentGPUData(ScreenshotType screenshotType, QSize screenshotSize,
    QSize viewportSize, int tileX, int tileY)
{
    resetGPUComponentData();

    // We always scale to the Y axis
    double scale = static_cast<double>(screenshotSize.height()) / viewportSize.height();

    for(const auto& componentCameraAndLighting : _componentCameraAndLightings)
    {
        const Camera& componentCamera = componentCameraAndLighting._camera;
        QRectF componentViewport(
            componentCamera.viewport().topLeft() * scale,
            componentCamera.viewport().size() * scale);

        auto viewport = (screenshotType == ScreenshotType::Tile) ?
            QRect((tileX * TILE_SIZE) - TILE_EXTRA, (tileY * TILE_SIZE) - TILE_EXTRA,
                TILE_SIZE_PLUS_EXTRA, TILE_SIZE_PLUS_EXTRA) :
            QRect({0, 0}, screenshotSize);

        auto projectionMatrix = GraphComponentRenderer::subViewportMatrix(componentViewport, viewport) *
            componentCamera.projectionMatrix();

        appendGPUComponentData(componentCamera.viewMatrix(),
            projectionMatrix,
            componentCamera.distance(),
            componentCameraAndLighting._lightScale);
    }

    uploadGPUComponentData();
}

bool ScreenshotRenderer::copyState(const GraphRenderer& renderer)
{
    _componentCameraAndLightings.clear();

    for(size_t i = 0; i < renderer._gpuGraphData.size(); ++i)
        _gpuGraphData.at(i).copyState(renderer._gpuGraphData.at(i), _nodesShader, _edgesShader, _textShader);

    for(const auto& componentRendererRef : renderer.componentRenderers())
    {
        const GraphComponentRenderer* componentRenderer = componentRendererRef;

        // Skip invisible components
        if(!componentRenderer->visible())
            continue;

        // This order MUST match graphrenderer component order!
        _componentCameraAndLightings.emplace_back(*componentRenderer->cameraAndLighting());
    }

    setShading(renderer.shading());

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
