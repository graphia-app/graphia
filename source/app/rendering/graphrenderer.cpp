#include "graphrenderer.h"

#include "glyphmap.h"
#include "graphcomponentscene.h"
#include "graphoverviewscene.h"
#include "compute/sdfcomputejob.h"
#include "shared/utils/preferences.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include "ui/graphcomponentinteractor.h"
#include "ui/graphoverviewinteractor.h"
#include "ui/document.h"
#include "ui/graphquickitem.h"
#include "ui/selectionmanager.h"
#include "ui/visualisations/elementvisual.h"

#include "shadertools.h"
#include "screenshotrenderer.h"

#include <QObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLDebugLogger>
#include <QColor>
#include <QQuickWindow>
#include <QEvent>
#include <QNativeGestureEvent>
#include <QTextLayout>
#include <QBuffer>

#include <utility>

template<typename Target>
void initialiseFromGraph(const Graph* graph, Target& target)
{
    for(auto componentId : graph->componentIds())
        target.onComponentAdded(graph, componentId, false);

    target.onGraphChanged(graph, true);
}

GraphRenderer::GraphRenderer(GraphModel* graphModel,
                             CommandManager* commandManager,
                             SelectionManager* selectionManager,
                             GPUComputeThread* gpuComputeThread) :
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _gpuComputeThread(gpuComputeThread),
    _componentRenderers(_graphModel->graph()),
    _hiddenNodes(_graphModel->graph()),
    _hiddenEdges(_graphModel->graph()),
    _layoutChanged(true),
    _performanceCounter(std::chrono::seconds(1))
{
    ShaderTools::loadShaderProgram(_debugLinesShader, QStringLiteral(":/shaders/debuglines.vert"), QStringLiteral(":/shaders/debuglines.frag"));

    _glyphMap = std::make_unique<GlyphMap>(u::pref("visuals/textFont").toString());

    auto graph = &_graphModel->graph();

    connect(graph, &Graph::nodeAdded, this, &GraphRenderer::onNodeAdded, Qt::DirectConnection);
    connect(graph, &Graph::edgeAdded, this, &GraphRenderer::onEdgeAdded, Qt::DirectConnection);
    connect(graph, &Graph::nodeAddedToComponent, this, &GraphRenderer::onNodeAddedToComponent, Qt::DirectConnection);
    connect(graph, &Graph::edgeAddedToComponent, this, &GraphRenderer::onEdgeAddedToComponent, Qt::DirectConnection);

    connect(graph, &Graph::graphWillChange, this, &GraphRenderer::onGraphWillChange, Qt::DirectConnection);
    connect(graph, &Graph::graphChanged, this, &GraphRenderer::onGraphChanged, Qt::DirectConnection);
    connect(graph, &Graph::componentAdded, this, &GraphRenderer::onComponentAdded, Qt::DirectConnection);

    connect(&_transition, &Transition::started, this, &GraphRenderer::rendererStartedTransition, Qt::DirectConnection);
    connect(&_transition, &Transition::finished, this, &GraphRenderer::rendererFinishedTransition, Qt::DirectConnection);

    _graphOverviewScene = new GraphOverviewScene(commandManager, this);
    _graphComponentScene = new GraphComponentScene(this);

    connect(graph, &Graph::componentWillBeRemoved, this, &GraphRenderer::onComponentWillBeRemoved, Qt::DirectConnection);

    _graphOverviewInteractor = new GraphOverviewInteractor(_graphModel, _graphOverviewScene, commandManager, _selectionManager, this);
    _graphComponentInteractor = new GraphComponentInteractor(_graphModel, _graphComponentScene, commandManager, _selectionManager, this);

    initialiseFromGraph(graph, *this);
    initialiseFromGraph(graph, *_graphOverviewScene);
    initialiseFromGraph(graph, *_graphComponentScene);

    _screenshotRenderer = std::make_unique<ScreenshotRenderer>();
    connect(_screenshotRenderer.get(), &ScreenshotRenderer::screenshotComplete, this, &GraphRenderer::screenshotComplete);
    connect(_screenshotRenderer.get(), &ScreenshotRenderer::previewComplete, this, &GraphRenderer::previewComplete);

    // If the graph is a single component or empty, use component mode by default
    if(graph->numComponents() <= 1)
        switchToComponentMode(false);
    else
        switchToOverviewMode(false);

    connect(S(Preferences), &Preferences::preferenceChanged, this, &GraphRenderer::onPreferenceChanged, Qt::DirectConnection);
    connect(_graphModel, &GraphModel::visualsWillChange, [this]
    {
        disableSceneUpdate();
    });

    connect(_graphModel, &GraphModel::visualsChanged, [this]
    {
        updateText();

        executeOnRendererThread([this]
        {
            updateGPUData(When::Later);
            update(); // QQuickFramebufferObject::Renderer::update
        }, QStringLiteral("GraphModel::visualsChanged"));

        enableSceneUpdate();
    });

    _performanceCounter.setReportFn([this](float ticksPerSecond)
    {
        emit fpsChanged(ticksPerSecond);
    });

    updateText([this]
    {
        emit initialised();
        enableSceneUpdate();
    });
}

GraphRenderer::~GraphRenderer()
{
    _FBOcomplete = false;
}

void GraphRenderer::createGPUGlyphData(const QString& text, const QColor& textColor, const TextAlignment& textAlignment,
                                    float textScale, float elementSize, const QVector3D& elementPosition,
                                    int componentIndex, GPUGraphData* gpuGraphData)
{
    Q_ASSERT(gpuGraphData != nullptr);

    auto& textLayout = _textLayoutResults._layouts[text];

    auto verticalCentre = -textLayout._xHeight * textScale * 0.5f;
    auto top = elementSize;
    auto bottom = (-elementSize) - (textLayout._xHeight * textScale);

    auto horizontalCentre = -textLayout._width * textScale * 0.5f;
    auto right = elementSize;
    auto left = (-elementSize) - (textLayout._width * textScale);

    for(const auto& glyph : textLayout._glyphs)
    {
        GPUGraphData::GlyphData glyphData;

        auto textureGlyph = _textLayoutResults._glyphs[glyph._index];

        glyphData._component = componentIndex;

        std::array<float, 2> baseOffset{{0.0f, 0.0f}};
        switch(textAlignment)
        {
        default:
        case TextAlignment::Right:  baseOffset = {{right,            verticalCentre}}; break;
        case TextAlignment::Left:   baseOffset = {{left,             verticalCentre}}; break;
        case TextAlignment::Centre: baseOffset = {{horizontalCentre, verticalCentre}}; break;
        case TextAlignment::Top:    baseOffset = {{horizontalCentre, top           }}; break;
        case TextAlignment::Bottom: baseOffset = {{horizontalCentre, bottom        }}; break;
        }

        glyphData._glyphOffset[0] = baseOffset[0] + (static_cast<float>(glyph._advance) * textScale);
        glyphData._glyphOffset[1] = baseOffset[1] - ((textureGlyph._height + textureGlyph._ascent) * textScale);
        glyphData._glyphSize[0] = textureGlyph._width;
        glyphData._glyphSize[1] = textureGlyph._height;

        glyphData._textureCoord[0] = textureGlyph._u;
        glyphData._textureCoord[1] = textureGlyph._v;
        glyphData._textureLayer = textureGlyph._layer;

        glyphData._basePosition[0] = elementPosition.x();
        glyphData._basePosition[1] = elementPosition.y();
        glyphData._basePosition[2] = elementPosition.z();

        glyphData._color[0] = textColor.redF();
        glyphData._color[1] = textColor.greenF();
        glyphData._color[2] = textColor.blueF();

        gpuGraphData->_glyphData.push_back(glyphData);
    }
}

void GraphRenderer::updateGPUDataIfRequired()
{
    if(!_gpuDataRequiresUpdate)
        return;

    _gpuDataRequiresUpdate = false;

    std::unique_lock<std::recursive_mutex> nodePositionsLock(_graphModel->nodePositions().mutex());
    std::unique_lock<std::recursive_mutex> glyphMapLock(_glyphMap->mutex());

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

                createGPUGlyphData(nodeVisual._text, textColor, textAlignment, textScale,
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
                createGPUGlyphData(edgeVisual._text, textColor, textAlignment, textScale,
                    edgeVisual._size, midPoint, componentIndex,
                    gpuGraphDataForOverlay(componentRenderer->alpha()));
            }
        }

        componentIndex++;
    }

    uploadGPUGraphData();
}

void GraphRenderer::updateGPUData(GraphRenderer::When when)
{
    _gpuDataRequiresUpdate = true;

    if(when == When::Now)
        updateGPUDataIfRequired();
}

void GraphRenderer::onPreviewRequested(int width, int height, bool fillSize)
{
    _screenshotRenderer->cloneState(*this);
    _screenshotRenderer->requestPreview(width, height, fillSize);
}

void GraphRenderer::onScreenshotRequested(int width, int height, const QString& path, int dpi, bool fillSize)
{
    _screenshotRenderer->cloneState(*this);
    _screenshotRenderer->requestScreenshot(width, height, path, dpi, fillSize);
}

void GraphRenderer::updateComponentGPUData()
{
    //FIXME this doesn't necessarily need to be entirely regenerated and rebuffered
    // every frame, so it makes sense to do partial updates as and when required.
    // OTOH, it's probably not ever going to be masses of data, so maybe we should
    // just suck it up; need to get a profiler on it and see how long we're spending
    // here transfering the buffer, when there are lots of components
    std::vector<GLfloat> componentData;

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
        for(int i = 0; i < 16; i++)
            componentData.push_back(componentRenderer->projectionMatrix().data()[i]);
    }

    glBindBuffer(GL_TEXTURE_BUFFER, componentDataTBO());
    glBufferData(GL_TEXTURE_BUFFER, componentData.size() * sizeof(GLfloat), componentData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void GraphRenderer::setScene(Scene* scene)
{
    if(_scene != nullptr)
    {
        _scene->setVisible(false);
        _scene->onHide();
    }

    _scene = scene;

    _scene->setVisible(true);
    _scene->onShow();

    _scene->setViewportSize(width(), height());
}

GraphRenderer::Mode GraphRenderer::mode() const
{
    return _mode;
}

void GraphRenderer::setMode(Mode mode)
{
    if(mode != _mode)
    {
        _mode = mode;
        emit modeChanged();
    }
}

void GraphRenderer::resetTime()
{
    _time.start();
    _lastTime = 0.0f;
}

float GraphRenderer::secondsElapsed()
{
    float time = _time.elapsed() / 1000.0f;
    float dTime = time - _lastTime;
    _lastTime = time;

    return dTime;
}

bool GraphRenderer::transitionActive() const
{
    return _transition.active() || _scene->transitionActive();
}

void GraphRenderer::moveFocusToNode(NodeId nodeId, float radius)
{
    if(mode() == Mode::Component)
    {
        executeOnRendererThread([this, nodeId, radius]
        {
            _graphComponentScene->moveFocusToNode(nodeId, radius);
        }, QStringLiteral("GraphRenderer::moveFocusToNode"));
    }
    else if(mode() == Mode::Overview)
    {
        // To focus on a node, we need to be in component mode
        auto componentId = _graphModel->graph().componentIdOfNode(nodeId);
        auto* newComponentRenderer = componentRendererForId(componentId);
        newComponentRenderer->moveFocusToNode(nodeId, radius);
        newComponentRenderer->saveViewData();
        newComponentRenderer->resetView();

        switchToComponentMode(true, componentId);
    }
}

void GraphRenderer::moveFocusToComponent(ComponentId componentId)
{
    if(mode() == Mode::Component)
        _graphComponentScene->setComponentId(componentId, true);
}

void GraphRenderer::rendererStartedTransition()
{
    if(!_transitionPotentiallyInProgress)
    {
        _transitionPotentiallyInProgress = true;

        emit userInteractionStarted();
        resetTime();
    }
}

void GraphRenderer::rendererFinishedTransition()
{
    if(transitionActive())
        return;

    _transitionPotentiallyInProgress = false;
    emit userInteractionFinished();
}

void GraphRenderer::sceneFinishedTransition()
{
    clearHiddenElements();
    updateGPUData(When::Later);
}

void GraphRenderer::executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description)
{
    _preUpdateExecutor.enqueue(std::move(task), description);
    emit taskAddedToExecutor();
}

// Stop the renderer thread executing any tasks until...
void GraphRenderer::pauseRendererThreadExecution()
{
    _preUpdateExecutor.pause();
}

// ...this is called
void GraphRenderer::resumeRendererThreadExecution()
{
    _preUpdateExecutor.resume();
}

bool GraphRenderer::visible() const
{
    return _graphOverviewScene->visible() || _graphComponentScene->visible();
}

void GraphRenderer::onNodeAdded(const Graph*, NodeId nodeId)
{
    _hiddenNodes.set(nodeId, true);
}

void GraphRenderer::onEdgeAdded(const Graph*, EdgeId edgeId)
{
    _hiddenEdges.set(edgeId, true);
}

void GraphRenderer::onNodeAddedToComponent(const Graph*, NodeId nodeId, ComponentId)
{
    _hiddenNodes.set(nodeId, true);
}

void GraphRenderer::onEdgeAddedToComponent(const Graph*, EdgeId edgeId, ComponentId)
{
    _hiddenEdges.set(edgeId, true);
}

void GraphRenderer::finishTransitionToOverviewMode(bool doTransition)
{
    setMode(GraphRenderer::Mode::Overview);
    setScene(_graphOverviewScene);
    setInteractor(_graphOverviewInteractor);

    if(doTransition)
    {
        // When we first change to overview mode we want all
        // the renderers to be in their reset state
        for(auto componentId : _graphModel->graph().componentIds())
        {
            auto renderer = componentRendererForId(componentId);
            renderer->resetView();
        }

        _graphOverviewScene->resetView(false);
        _graphOverviewScene->startTransitionFromComponentMode(_graphComponentScene->componentId());
    }

    updateGPUData(When::Later);
}

void GraphRenderer::finishTransitionToOverviewModeOnRendererThread(bool doTransition)
{
    setMode(GraphRenderer::Mode::Overview);
    executeOnRendererThread([this, doTransition]
    {
        finishTransitionToOverviewMode(doTransition);
    }, QStringLiteral("GraphRenderer::finishTransitionToOverviewMode"));
}

void GraphRenderer::finishTransitionToComponentMode(bool doTransition)
{
    setMode(GraphRenderer::Mode::Component);
    setScene(_graphComponentScene);
    setInteractor(_graphComponentInteractor);

    if(doTransition)
    {
        // Go back to where we were before
        _graphComponentScene->startTransition();
        _graphComponentScene->restoreViewData();
    }

    updateGPUData(When::Later);
}

void GraphRenderer::finishTransitionToComponentModeOnRendererThread(bool doTransition)
{
    setMode(GraphRenderer::Mode::Component);
    executeOnRendererThread([this, doTransition]
    {
        finishTransitionToComponentMode(doTransition);
    }, QStringLiteral("GraphRenderer::finishTransitionToComponentMode"));
}

void GraphRenderer::switchToOverviewMode(bool doTransition)
{
    executeOnRendererThread([this, doTransition]
    {
        // Refuse to switch to overview mode if there is nothing to display
        if(_graphModel->graph().numComponents() <= 1)
            return;

        // So that we can return to the current view parameters later
        _graphComponentScene->saveViewData();

        if(mode() != GraphRenderer::Mode::Overview && doTransition &&
           _graphComponentScene->componentRenderer() != nullptr)
        {
            if(!_graphComponentScene->viewIsReset())
            {
                _graphComponentScene->startTransition([this]
                {
                    sceneFinishedTransition();
                    _transition.willBeImmediatelyReused();
                    finishTransitionToOverviewModeOnRendererThread(true);
                });

                _graphComponentScene->resetView(false);
            }
            else
                finishTransitionToOverviewModeOnRendererThread(true);
        }
        else
            finishTransitionToOverviewMode(false);

    }, QStringLiteral("GraphRenderer::switchToOverviewMode"));
}

void GraphRenderer::switchToComponentMode(bool doTransition, ComponentId componentId, NodeId nodeId)
{
    executeOnRendererThread([this, componentId, nodeId, doTransition]
    {
        _graphComponentScene->setComponentId(componentId);

        if(mode() != GraphRenderer::Mode::Component && doTransition)
        {
            _graphOverviewScene->startTransitionToComponentMode(_graphComponentScene->componentId(),
            [this, componentId, nodeId]
            {
                if(!nodeId.isNull())
                {
                    Q_ASSERT(_graphModel->graph().componentIdOfNode(nodeId) == componentId);
                    componentRendererForId(componentId)->moveSavedFocusToNode(nodeId);
                }

                if(!_graphComponentScene->savedViewIsReset())
                {
                    _transition.willBeImmediatelyReused();
                    finishTransitionToComponentModeOnRendererThread(true);
                }
                else
                    finishTransitionToComponentModeOnRendererThread(false);
            });
        }
        else
            finishTransitionToComponentMode(false);

    }, QStringLiteral("GraphRenderer::switchToComponentMode"));
}

void GraphRenderer::onGraphWillChange(const Graph* graph)
{
    pauseRendererThreadExecution();

    // Hide any graph elements that are merged; they aren't displayed normally,
    // but during scene transitions they may become unmerged, and we don't want
    // to show them until the scene transition is over
    for(NodeId nodeId : graph->nodeIds())
    {
        if(graph->typeOf(nodeId) == MultiElementType::Tail)
            _hiddenNodes.set(nodeId, true);
    }

    for(EdgeId edgeId : graph->edgeIds())
    {
        if(graph->typeOf(edgeId) == MultiElementType::Tail)
            _hiddenEdges.set(edgeId, true);
    }
}

void GraphRenderer::onGraphChanged(const Graph* graph, bool changed)
{
    if(!changed)
        return;

    _numComponents = graph->numComponents();

    // We may not, in fact, subsequently actually start a transition here, but we
    // speculatively pretend we do so that if a command is currently in progress
    // there is some overlap between it and the renderer transition. This ensures
    // that Document::busy() returns true for the duration of the transaction.
    if(visible())
        rendererStartedTransition();

    executeOnRendererThread([this]
    {
        for(ComponentId componentId : _graphModel->graph().componentIds())
        {
            componentRendererForId(componentId)->initialise(_graphModel, componentId,
                                                            _selectionManager, this);
        }
        updateGPUData(When::Later);
    }, QStringLiteral("GraphRenderer::onGraphChanged update"));
}

void GraphRenderer::onComponentAdded(const Graph*, ComponentId componentId, bool)
{
    Q_ASSERT(!componentId.isNull());

    // If the component is entirely new, we shouldn't be hiding any of it
    auto* component = _graphModel->graph().componentById(componentId);
    for(NodeId nodeId : component->nodeIds())
        _hiddenNodes.set(nodeId, false);
    for(EdgeId edgeId : component->edgeIds())
        _hiddenEdges.set(edgeId, false);

    executeOnRendererThread([this, componentId]
    {
        componentRendererForId(componentId)->initialise(_graphModel, componentId,
                                                        _selectionManager, this);
    }, QStringLiteral("GraphRenderer::onComponentAdded"));
}

void GraphRenderer::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool)
{
    executeOnRendererThread([this, componentId]
    {
        componentRendererForId(componentId)->cleanup();
    }, QStringLiteral("GraphRenderer::onComponentWillBeRemoved (cleanup) component %1").arg(static_cast<int>(componentId)));
}

void GraphRenderer::onPreferenceChanged(const QString& key, const QVariant& value)
{
    if(key == QLatin1String("visuals/textFont"))
    {
        _glyphMap->setFontName(value.toString());
        updateText();
    }
}

void GraphRenderer::onCommandWillExecute()
{
    disableSceneUpdate();
}

void GraphRenderer::onCommandCompleted()
{
    enableSceneUpdate();
    update();
}

void GraphRenderer::onLayoutChanged()
{
    _layoutChanged = true;
}

void GraphRenderer::onComponentAlphaChanged(ComponentId)
{
    updateGPUData(When::Later);
}

void GraphRenderer::onComponentCleanup(ComponentId)
{
    updateGPUData(When::Later);
}

void GraphRenderer::onVisibilityChanged()
{
    updateGPUData(When::Later);
}

GLuint GraphRenderer::sdfTexture() const
{
    return _sdfTexture.front();
}

void GraphRenderer::updateText(std::function<void()> onCompleteFn) // NOLINT
{
    std::unique_lock<std::recursive_mutex> glyphMapLock(_glyphMap->mutex());

    for(auto nodeId : _graphModel->graph().nodeIds())
        _glyphMap->addText(_graphModel->nodeVisual(nodeId)._text);

    for(auto edgeId : _graphModel->graph().edgeIds())
        _glyphMap->addText(_graphModel->edgeVisual(edgeId)._text);

    if(_glyphMap->updateRequired())
    {
        glyphMapLock.unlock();

        auto job = std::make_unique<SDFComputeJob>(&_sdfTexture, _glyphMap.get());
        job->executeWhenComplete([this, onCompleteFn]
        {
            executeOnRendererThread([this]
            {
                _sdfTexture.swap();
                _textLayoutResults = _glyphMap->results();

                updateGPUData(When::Later);
                update(); // QQuickFramebufferObject::Renderer::update
            }, QStringLiteral("GraphRenderer::updateText"));

            onCompleteFn();
        });

        _gpuComputeThread->enqueue(job);
    }
    else
        onCompleteFn();
}

void GraphRenderer::resetView()
{
    if(_scene != nullptr)
        _scene->resetView();
}

bool GraphRenderer::viewIsReset() const
{
    if(_scene != nullptr)
        return _scene->viewIsReset();

    return true;
}

void GraphRenderer::updateScene()
{
    ifSceneUpdateEnabled([this]
    {
        _preUpdateExecutor.execute();
    });

    if(_scene == nullptr)
        return;

    if(_resized)
    {
        _scene->setViewportSize(width(), height());
        _resized = false;
    }

    ifSceneUpdateEnabled([this]
    {
        // _synchronousLayoutChanged can only ever be (atomically) true in this scope
        _synchronousLayoutChanged = _layoutChanged.exchange(false);

        // If there is a transition active then we'll need another
        // frame once we're finished with this one
        if(transitionActive())
            update(); // QQuickFramebufferObject::Renderer::update

        float dTime = secondsElapsed();
        _transition.update(dTime);
        _scene->update(dTime);

        if(layoutChanged())
            updateGPUData(When::Later);

        updateGPUDataIfRequired();
        updateComponentGPUData();

        _synchronousLayoutChanged = false;
    });

    if(!_FBOcomplete)
    {
        qWarning() << "Attempting to render without a complete FBO";
        return;
    }
}

GraphRenderer::Mode GraphRenderer::bestFocusParameters(GraphQuickItem* graphQuickItem,
    NodeId& focusNodeId, float& radius) const
{
    radius = 0.0f;
    focusNodeId = {};

    auto nodeIds = graphQuickItem->desiredFocusNodeIds();

    if(nodeIds.empty())
        return mode();

    if(nodeIds.size() == 1)
    {
        radius = GraphComponentRenderer::COMFORTABLE_ZOOM_RADIUS;
        focusNodeId = nodeIds.front();
        return Mode::Component;
    }

    ComponentIdSet componentIds;
    for(auto nodeId : nodeIds)
        componentIds.insert(_graphModel->graph().componentIdOfNode(nodeId));

    if(componentIds.size() > 1)
    {
        // We want to focus on multiple nodes, but they span multiple components
        if(mode() == Mode::Component)
        {
            // Prune the nodeIds we consider for focus down to only those in the focused component
            auto focusedComponentId = _graphComponentScene->componentId();

            nodeIds.erase(std::remove_if(nodeIds.begin(), nodeIds.end(),
            [this, focusedComponentId](auto nodeId)
            {
                return _graphModel->graph().componentIdOfNode(nodeId) != focusedComponentId;
            }), nodeIds.end());
        }
        else
            return mode();
    }

    // None of the nodes are in the currently focused component
    if(nodeIds.empty())
        return Mode::Overview;

    // If the request is for more than 1 node, then find their barycentre and
    // pick the closest node to where ever this happens to be
    std::vector<QVector3D> points(nodeIds.size());
    size_t i = 0;
    for(auto nodeId : nodeIds)
    {
        auto nodePosition = _graphModel->nodePositions().getScaledAndSmoothed(nodeId);
        points.at(i++) = nodePosition;
    }

    BoundingSphere boundingSphere(points);
    QVector3D centre = boundingSphere.centre();
    float minDistance = std::numeric_limits<float>::max();
    NodeId closestToCentreNodeId;
    for(auto nodeId : nodeIds)
    {
        auto nodePosition = _graphModel->nodePositions().getScaledAndSmoothed(nodeId);
        float distance = (centre - nodePosition).length();

        if(distance < minDistance)
        {
            minDistance = distance;
            closestToCentreNodeId = nodeId;
        }
    }

    radius = GraphComponentRenderer::maxNodeDistanceFromPoint(*_graphModel,
        _graphModel->nodePositions().getScaledAndSmoothed(closestToCentreNodeId), nodeIds);
    focusNodeId = closestToCentreNodeId;

    return mode();
}

QOpenGLFramebufferObject* GraphRenderer::createFramebufferObject(const QSize& size)
{
    // Piggy back our FBO resize on to Qt's
    _FBOcomplete = resize(size.width(), size.height());
    _resized = true;

    return new QOpenGLFramebufferObject(size);
}

void GraphRenderer::render()
{
    if(!_FBOcomplete)
    {
        qWarning() << "Attempting to render incomplete FBO";
        return;
    }

    glViewport(0, 0, width(), height());

    updateScene();
    renderGraph();

    render2D(_selectionRect);

    // Check the normal FBO
    if(!framebufferObject()->bind())
        qWarning() << "QQuickFrameBufferobject::Renderer FBO not bound";

    renderToFramebuffer();

    std::unique_lock<std::mutex> lock(_resetOpenGLStateMutex);
    resetOpenGLState();

    _performanceCounter.tick();
}

void GraphRenderer::synchronize(QQuickFramebufferObject* item)
{
    if(!resetOpenGLState)
    {
        resetOpenGLState = [item]
        {
            if(item->window() != nullptr)
                item->window()->resetOpenGLState();
        };

        connect(item, &QObject::destroyed, [this]
        {
            std::unique_lock<std::mutex> lock(_resetOpenGLStateMutex);
            resetOpenGLState = []{};
        });
    }

    auto graphQuickItem = qobject_cast<GraphQuickItem*>(item);

    if(graphQuickItem->viewResetPending())
        resetView();

    if(graphQuickItem->overviewModeSwitchPending())
        switchToOverviewMode();

    float radius = GraphComponentRenderer::COMFORTABLE_ZOOM_RADIUS;
    NodeId focusNodeId;
    auto targetMode = bestFocusParameters(graphQuickItem, focusNodeId, radius);
    ComponentId focusComponentId = graphQuickItem->desiredFocusComponentId();

    if(mode() == Mode::Component && targetMode == Mode::Overview)
        switchToOverviewMode();
    else if(!focusNodeId.isNull())
        moveFocusToNode(focusNodeId, radius);
    else if(!focusComponentId.isNull())
    {
        if(mode() == Mode::Overview)
            switchToComponentMode(true, focusComponentId);
        else
            moveFocusToComponent(focusComponentId);
    }

    ifSceneUpdateEnabled([this, &graphQuickItem]
    {
        if(_scene != nullptr)
        {
            //FIXME try delivering these events by queued connection instead
            while(graphQuickItem->eventsPending())
            {
                auto e = graphQuickItem->nextEvent();
                auto mouseEvent = dynamic_cast<QMouseEvent*>(e.get());
                auto wheelEvent = dynamic_cast<QWheelEvent*>(e.get());
                auto nativeGestureEvent = dynamic_cast<QNativeGestureEvent*>(e.get());

                switch(e->type())
                {
                case QEvent::Type::MouseButtonPress:    _interactor->mousePressEvent(mouseEvent);               break;
                case QEvent::Type::MouseButtonRelease:  _interactor->mouseReleaseEvent(mouseEvent);             break;
                case QEvent::Type::MouseMove:           _interactor->mouseMoveEvent(mouseEvent);                break;
                case QEvent::Type::MouseButtonDblClick: _interactor->mouseDoubleClickEvent(mouseEvent);         break;
                case QEvent::Type::Wheel:               _interactor->wheelEvent(wheelEvent);                    break;
                case QEvent::Type::NativeGesture:       _interactor->nativeGestureEvent(nativeGestureEvent);    break;
                default: break;
                }
            }
        }
    });

    // Tell the QuickItem what we're doing
    graphQuickItem->setViewIsReset(viewIsReset());
    graphQuickItem->setCanEnterOverviewMode(mode() != Mode::Overview && _numComponents > 1);
    graphQuickItem->setInOverviewMode(mode() == Mode::Overview);

    ComponentId focusedComponentId = mode() != Mode::Overview ? _graphComponentScene->componentId() : ComponentId();
    graphQuickItem->setFocusedComponentId(focusedComponentId);

    emit synchronizeComplete();
}

GraphComponentRenderer* GraphRenderer::componentRendererForId(ComponentId componentId) const
{
    if(componentId.isNull())
        return nullptr;

    GraphComponentRenderer* renderer = _componentRenderers.at(componentId);
    Q_ASSERT(renderer != nullptr);
    return renderer;
}

void GraphRenderer::enableSceneUpdate()
{
    std::unique_lock<std::recursive_mutex> lock(_sceneUpdateMutex);
    Q_ASSERT(_sceneUpdateDisabled > 0);
    _sceneUpdateDisabled--;
    resetTime();
}

void GraphRenderer::disableSceneUpdate()
{
    std::unique_lock<std::recursive_mutex> lock(_sceneUpdateMutex);
    _sceneUpdateDisabled++;
}

void GraphRenderer::ifSceneUpdateEnabled(const std::function<void()>& f)
{
    std::unique_lock<std::recursive_mutex> lock(_sceneUpdateMutex);
    if(_sceneUpdateDisabled == 0)
        f();
}

void GraphRenderer::clearHiddenElements()
{
    _hiddenNodes.resetElements();
    _hiddenEdges.resetElements();
}
