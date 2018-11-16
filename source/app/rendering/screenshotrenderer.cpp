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
    _componentRenderers(renderer.graphModel()->graph()),
    _selectionManager(renderer._selectionManager),
    _hiddenNodes(_graphModel->graph()),
    _hiddenEdges(_graphModel->graph()),
    _mainRenderer(renderer)
{
    for(ComponentId componentId : _graphModel->graph().componentIds())
        *componentRendererForId(componentId) = *renderer.componentRendererForId(componentId);

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

        for(int layer = 0; layer < renderer._glyphMap->images().size(); layer++)
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

    uploadGPUGraphData();
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
    updateGPUDataIfRequired();
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

    float viewportAspectRatio = static_cast<float>(this->width()) / static_cast<float>(this->height());

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

void ScreenshotRenderer::updateGPUDataIfRequired()
{
    std::unique_lock<std::recursive_mutex> nodePositionsLock(_graphModel->nodePositions().mutex());

    int componentIndex = 0;

    auto& nodePositions = _graphModel->nodePositions();

    resetGPUGraphData();

    NodeArray<QVector3D> scaledAndSmoothedNodePositions(_graphModel->graph());

    float textScale = u::pref("visuals/textSize").toFloat();
    auto textAlignment = static_cast<TextAlignment>(u::pref("visuals/textAlignment").toInt());
    auto textColor = Document::contrastingColorForBackground();
    auto showNodeText = static_cast<TextState>(u::pref("visuals/showNodeText").toInt());
    auto showEdgeText = static_cast<TextState>(u::pref("visuals/showEdgeText").toInt());
    auto edgeVisualType = static_cast<EdgeVisualType>(u::pref("visuals/edgeVisualType").toInt());

    // Ignore the setting if the graph is undirected
    if(!_graphModel->directed())
        edgeVisualType = EdgeVisualType::Cylinder;

    for(auto& componentRendererRef : _componentRenderers)
    {
        GraphComponentRenderer* componentRenderer = componentRendererRef;
        if(!componentRenderer->visible())
            continue;

        const float UnhighlightedAlpha = 0.15f;

        for(auto nodeId : componentRenderer->nodeIds())
        {
            if(_hiddenNodes.get(nodeId))
                continue;

            const QVector3D nodePosition = nodePositions.getScaledAndSmoothed(nodeId);
            scaledAndSmoothedNodePositions[nodeId] = nodePosition;

            auto& nodeVisual = _graphModel->nodeVisual(nodeId);

            // Create and add NodeData
            GPUGraphData::NodeData nodeData;
            nodeData._position[0] = nodePosition.x();
            nodeData._position[1] = nodePosition.y();
            nodeData._position[2] = nodePosition.z();
            nodeData._component = componentIndex;
            nodeData._size = nodeVisual._size;
            nodeData._outerColor[0] = nodeVisual._outerColor.redF();
            nodeData._outerColor[1] = nodeVisual._outerColor.greenF();
            nodeData._outerColor[2] = nodeVisual._outerColor.blueF();
            nodeData._innerColor[0] = nodeVisual._innerColor.redF();
            nodeData._innerColor[1] = nodeVisual._innerColor.greenF();
            nodeData._innerColor[2] = nodeVisual._innerColor.blueF();

            QColor outlineColor = nodeVisual._state.test(VisualFlags::Selected) ?
                Qt::white : Qt::black;

            nodeData._outlineColor[0] = outlineColor.redF();
            nodeData._outlineColor[1] = outlineColor.greenF();
            nodeData._outlineColor[2] = outlineColor.blueF();

            auto* gpuGraphData = gpuGraphDataForAlpha(componentRenderer->alpha(),
                nodeVisual._state.test(VisualFlags::Unhighlighted) ? UnhighlightedAlpha : 1.0f);

            if(gpuGraphData != nullptr)
            {
                gpuGraphData->_nodeData.push_back(nodeData);

                if(showNodeText == TextState::Off || nodeVisual._state.test(VisualFlags::Unhighlighted))
                    continue;

                if(showNodeText == TextState::Selected && !nodeVisual._state.test(VisualFlags::Selected))
                    continue;

                _mainRenderer.createGPUGlyphData(nodeVisual._text, textColor, textAlignment, textScale,
                    nodeVisual._size, nodePosition, componentIndex,
                    gpuGraphDataForOverlay(componentRenderer->alpha()));
            }
        }

        for(auto& edge : componentRenderer->edges())
        {
            if(_hiddenEdges.get(edge->id()) || _hiddenNodes.get(edge->sourceId()) || _hiddenNodes.get(edge->targetId()))
                continue;

            const QVector3D& sourcePosition = scaledAndSmoothedNodePositions[edge->sourceId()];
            const QVector3D& targetPosition = scaledAndSmoothedNodePositions[edge->targetId()];

            auto& edgeVisual = _graphModel->edgeVisual(edge->id());
            auto& sourceNodeVisual = _graphModel->nodeVisual(edge->sourceId());
            auto& targetNodeVisual = _graphModel->nodeVisual(edge->targetId());

            auto nodeRadiusSumSq = sourceNodeVisual._size + targetNodeVisual._size;
            nodeRadiusSumSq *= nodeRadiusSumSq;
            const auto edgeLengthSq = (targetPosition - sourcePosition).lengthSquared();

            if(edgeLengthSq < nodeRadiusSumSq)
            {
                // The edge's nodes are intersecting. Their overlap defines a lens of a
                // certain radius. If this is greater than the edge radius, the edge is
                // entirely enclosed within the nodes and we can safely skip rendering
                // it altogether since it is entirely occluded.

                const auto sourceRadiusSq = sourceNodeVisual._size * sourceNodeVisual._size;
                const auto targetRadiusSq = targetNodeVisual._size * targetNodeVisual._size;
                const auto term = edgeLengthSq + sourceRadiusSq - targetRadiusSq;
                const auto intersectionLensRadiusSq = (edgeLengthSq * edgeLengthSq * sourceRadiusSq) -
                    ((edgeLengthSq * term * term) / 4.0f);
                const auto edgeRadiusSq = edgeVisual._size * edgeVisual._size;

                if(edgeRadiusSq < intersectionLensRadiusSq)
                    continue;
            }

            GPUGraphData::EdgeData edgeData;
            edgeData._sourcePosition[0] = sourcePosition.x();
            edgeData._sourcePosition[1] = sourcePosition.y();
            edgeData._sourcePosition[2] = sourcePosition.z();
            edgeData._targetPosition[0] = targetPosition.x();
            edgeData._targetPosition[1] = targetPosition.y();
            edgeData._targetPosition[2] = targetPosition.z();
            edgeData._sourceSize = _graphModel->nodeVisual(edge->sourceId())._size;
            edgeData._targetSize = _graphModel->nodeVisual(edge->targetId())._size;
            edgeData._edgeType = static_cast<int>(edgeVisualType);
            edgeData._component = componentIndex;
            edgeData._size = edgeVisual._size;
            edgeData._outerColor[0] = edgeVisual._outerColor.redF();
            edgeData._outerColor[1] = edgeVisual._outerColor.greenF();
            edgeData._outerColor[2] = edgeVisual._outerColor.blueF();
            edgeData._innerColor[0] = edgeVisual._innerColor.redF();
            edgeData._innerColor[1] = edgeVisual._innerColor.greenF();
            edgeData._innerColor[2] = edgeVisual._innerColor.blueF();

            edgeData._outlineColor[0] = 0.0f;
            edgeData._outlineColor[1] = 0.0f;
            edgeData._outlineColor[2] = 0.0f;

            auto* gpuGraphData = gpuGraphDataForAlpha(componentRenderer->alpha(),
                edgeVisual._state.test(VisualFlags::Unhighlighted) ? UnhighlightedAlpha : 1.0f);

            if(gpuGraphData != nullptr)
            {
                gpuGraphData->_edgeData.push_back(edgeData);

                if(showEdgeText == TextState::Off || edgeVisual._state.test(VisualFlags::Unhighlighted))
                    continue;

                if(showEdgeText == TextState::Selected && !edgeVisual._state.test(VisualFlags::Selected))
                    continue;

                QVector3D midPoint = (sourcePosition + targetPosition) * 0.5f;
                _mainRenderer.createGPUGlyphData(edgeVisual._text, textColor, textAlignment, textScale,
                                                 edgeVisual._size, midPoint, componentIndex,
                                                 gpuGraphDataForOverlay(componentRenderer->alpha()));
            }
        }

        componentIndex++;
    }

    uploadGPUGraphData();
}

GraphComponentRenderer* ScreenshotRenderer::componentRendererForId(ComponentId componentId) const
{
    if(componentId.isNull())
        return nullptr;

    GraphComponentRenderer* renderer = _componentRenderers.at(componentId);
    Q_ASSERT(renderer != nullptr);
    return renderer;
}

void ScreenshotRenderer::updateComponentGPUData()
{
    //FIXME this doesn't necessarily need to be entirely regenerated and rebuffered
    // every frame, so it makes sense to do partial updates as and when required.
    // OTOH, it's probably not ever going to be masses of data, so maybe we should
    // just suck it up; need to get a profiler on it and see how long we're spending
    // here transfering the buffer, when there are lots of components
    std::vector<GLfloat> componentData;

    for(ComponentId componentId : _graphModel->graph().componentIds())
    {
        componentRendererForId(componentId)->initialise(_graphModel, componentId,
                                                        _selectionManager, nullptr);
        componentRendererForId(componentId)->setViewportSize(_screenshotWidth, _screenshotHeight);
        componentRendererForId(componentId)->setDimensions(QRectF(0, 0, _screenshotWidth, _screenshotHeight));
        componentRendererForId(componentId)->setVisible(true);
    }

    for(auto& componentRendererRef : _componentRenderers)
    {
        GraphComponentRenderer* componentRenderer = componentRendererRef;
        if(componentRenderer == nullptr)
        {
            qWarning() << "null component renderer";
            continue;
        }

        if(!componentRenderer->visible())
            continue;

        // Model View
        for(int i = 0; i < 16; i++)
            componentData.push_back(componentRenderer->modelViewMatrix().data()[i]);

        // Projection
        if(_isScreenshot)
        {
            // Tile projection for high res screenshots
            float tileWidthRatio = static_cast<float>(width()) / _screenshotWidth;
            float tileHeightRatio = static_cast<float>(height()) / _screenshotHeight;

            float tileTranslationX = ((1.0f / tileWidthRatio) - 1.0f) - (2.0f * _currentTileX);
            float tileTranslationY = ((1.0f / tileHeightRatio) - 1.0f) - (2.0f * _currentTileY);

            QMatrix4x4 projMatrix = componentRenderer->screenshotTileProjectionMatrix(TILE_SIZE);
            QMatrix4x4 tileTranslation;

            tileTranslation.translate(tileTranslationX, tileTranslationY, 0.0f);
            projMatrix = tileTranslation * projMatrix;

            for(int i = 0; i < 16; i++)
                componentData.push_back(projMatrix.data()[i]);
        }
        else
        {
            // Normal projection
            for(int i = 0; i < 16; i++)
                componentData.push_back(componentRenderer->projectionMatrix().data()[i]);
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
