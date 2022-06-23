/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "graphrenderer.h"

#include "glyphmap.h"
#include "graphcomponentscene.h"
#include "graphoverviewscene.h"
#include "compute/sdfcomputejob.h"

#include "app/preferences.h"

#include "shared/utils/doasyncthen.h"

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
#include <QCoreApplication>
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickOpenGLUtils>
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
    _graphOverviewScene(new GraphOverviewScene(commandManager, this)),
    _graphComponentScene(new GraphComponentScene(this)),
    _graphOverviewInteractor(new GraphOverviewInteractor(_graphModel, _graphOverviewScene, commandManager, _selectionManager, this)),
    _graphComponentInteractor(new GraphComponentInteractor(_graphModel, _graphComponentScene, commandManager, _selectionManager, this)),
    _hiddenNodes(_graphModel->graph()),
    _hiddenEdges(_graphModel->graph()),
    _layoutChanged(true),
    _performanceCounter(std::chrono::seconds(1))
{
    ShaderTools::loadShaderProgram(_debugLinesShader, QStringLiteral(":/shaders/debuglines.vert"), QStringLiteral(":/shaders/debuglines.frag"));

    _glyphMap = std::make_unique<GlyphMap>(u::pref(QStringLiteral("visuals/textFont")).toString());

    const auto* graph = &_graphModel->graph();

    connect(graph, &Graph::nodeAdded, this, &GraphRenderer::onNodeAdded, Qt::DirectConnection);
    connect(graph, &Graph::edgeAdded, this, &GraphRenderer::onEdgeAdded, Qt::DirectConnection);
    connect(graph, &Graph::nodeAddedToComponent, this, &GraphRenderer::onNodeAddedToComponent, Qt::DirectConnection);
    connect(graph, &Graph::edgeAddedToComponent, this, &GraphRenderer::onEdgeAddedToComponent, Qt::DirectConnection);

    connect(graph, &Graph::graphWillChange, this, &GraphRenderer::onGraphWillChange, Qt::DirectConnection);
    connect(graph, &Graph::graphChanged, this, &GraphRenderer::onGraphChanged, Qt::DirectConnection);
    connect(graph, &Graph::componentAdded, this, &GraphRenderer::onComponentAdded, Qt::DirectConnection);

    connect(&_transition, &Transition::started, this, &GraphRenderer::rendererStartedTransition, Qt::DirectConnection);
    connect(&_transition, &Transition::finished, this, &GraphRenderer::rendererFinishedTransition, Qt::DirectConnection);

    connect(graph, &Graph::componentWillBeRemoved, this, &GraphRenderer::onComponentWillBeRemoved, Qt::DirectConnection);

    _screenshotRenderer = std::make_unique<ScreenshotRenderer>();
    connect(_screenshotRenderer.get(), &ScreenshotRenderer::screenshotComplete, this, &GraphRenderer::screenshotComplete);
    connect(_screenshotRenderer.get(), &ScreenshotRenderer::previewComplete, this, &GraphRenderer::previewComplete);

    connect(&_preferencesWatcher, &PreferencesWatcher::preferenceChanged,
        this, &GraphRenderer::onPreferenceChanged, Qt::DirectConnection);

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
        }, QStringLiteral("GraphRenderer::visualsChanged"));

        enableSceneUpdate();
    });

    _performanceCounter.setReportFn([this](float ticksPerSecond)
    {
        emit fpsChanged(ticksPerSecond);
    });

    u::doAsync([this, graph]
    {
        std::unique_lock<std::mutex> lock(_initialisationMutex);

        initialiseFromGraph(graph, *this);
        initialiseFromGraph(graph, *_graphOverviewScene);
        initialiseFromGraph(graph, *_graphComponentScene);

        // If the graph is a single component or empty, use component mode by default
        if(graph->numComponents() <= 1)
            switchToComponentMode(false);
        else
            switchToOverviewMode(false);

        updateText(true);

    }).then([this]
    {
        emit initialised();

        // The initial state is disabled, so this has no complement disableSceneUpdate call
        enableSceneUpdate();
        update();
    });
}

GraphRenderer::~GraphRenderer()
{
    std::unique_lock<std::mutex> lock(_initialisationMutex);

    // Force event processing in case the above .then(...) is still to run (due to a slow init)
    QCoreApplication::processEvents();

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

        glyphData._color[0] = static_cast<float>(textColor.redF());
        glyphData._color[1] = static_cast<float>(textColor.greenF());
        glyphData._color[2] = static_cast<float>(textColor.blueF());

        gpuGraphData->_glyphData.push_back(glyphData);
    }
}

void GraphRenderer::updateGPUDataIfRequired()
{
    if(!_gpuDataRequiresUpdate)
        return;

    _gpuDataRequiresUpdate = false;

    std::unique_lock<NodePositions> nodePositionsLock(_graphModel->nodePositions());
    std::unique_lock<std::recursive_mutex> glyphMapLock(_glyphMap->mutex());

    int componentIndex = 0;

    auto& nodePositions = _graphModel->nodePositions();

    resetGPUGraphData();

    NodeArray<QVector3D> scaledAndSmoothedNodePositions(_graphModel->graph());

    float textScale = u::pref(QStringLiteral("visuals/textSize")).toFloat();
    auto textAlignment = NORMALISE_QML_ENUM(TextAlignment, u::pref(QStringLiteral("visuals/textAlignment")).toInt());
    auto textColor = Document::contrastingColorForBackground();
    auto showNodeText = NORMALISE_QML_ENUM(TextState, u::pref(QStringLiteral("visuals/showNodeText")).toInt());
    auto showEdgeText = NORMALISE_QML_ENUM(TextState, u::pref(QStringLiteral("visuals/showEdgeText")).toInt());
    auto edgeVisualType = NORMALISE_QML_ENUM(EdgeVisualType, u::pref(QStringLiteral("visuals/edgeVisualType")).toInt());

    // Ignore the setting if the graph is undirected
    if(!_graphModel->directed())
        edgeVisualType = EdgeVisualType::Cylinder;

    for(const auto& componentRendererRef : _componentRenderers)
    {
        GraphComponentRenderer* componentRenderer = componentRendererRef;
        if(!componentRenderer->visible())
            continue;

        const float UnhighlightedAlpha = 0.22f;

        for(auto nodeId : componentRenderer->nodeIds())
        {
            if(_hiddenNodes.get(nodeId))
                continue;

            const QVector3D nodePosition = nodePositions.get(nodeId);
            scaledAndSmoothedNodePositions[nodeId] = nodePosition;

            const auto& nodeVisual = _graphModel->nodeVisual(nodeId);

            // Create and add NodeData
            GPUGraphData::NodeData nodeData;
            nodeData._position[0] = nodePosition.x();
            nodeData._position[1] = nodePosition.y();
            nodeData._position[2] = nodePosition.z();
            nodeData._component = componentIndex;
            nodeData._size = nodeVisual._size;
            nodeData._outerColor[0] = static_cast<float>(nodeVisual._outerColor.redF());
            nodeData._outerColor[1] = static_cast<float>(nodeVisual._outerColor.greenF());
            nodeData._outerColor[2] = static_cast<float>(nodeVisual._outerColor.blueF());
            nodeData._innerColor[0] = static_cast<float>(nodeVisual._innerColor.redF());
            nodeData._innerColor[1] = static_cast<float>(nodeVisual._innerColor.greenF());
            nodeData._innerColor[2] = static_cast<float>(nodeVisual._innerColor.blueF());
            nodeData._selected = nodeVisual._state.test(VisualFlags::Selected) ? 1.0f : 0.0f;

            auto* gpuGraphData = gpuGraphDataForAlpha(componentRenderer->alpha(),
                nodeVisual._state.test(VisualFlags::Unhighlighted) ? UnhighlightedAlpha : 1.0f);

            if(gpuGraphData != nullptr)
            {
                gpuGraphData->_nodeData.push_back(nodeData);

                if(nodeData._selected != 0.0f)
                    gpuGraphData->_elementsSelected = true;

                if(showNodeText == TextState::Off || nodeVisual._state.test(VisualFlags::Unhighlighted))
                    continue;

                if(showNodeText == TextState::Selected && !nodeVisual._state.test(VisualFlags::Selected))
                    continue;

                if(showNodeText == TextState::Focused && componentRenderer->focusNodeId() != nodeId)
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

            const auto& edgeVisual = _graphModel->edgeVisual(edge->id());
            const auto& sourceNodeVisual = _graphModel->nodeVisual(edge->sourceId());
            const auto& targetNodeVisual = _graphModel->nodeVisual(edge->targetId());

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

                const auto n = edgeLengthSq - sourceRadiusSq + targetRadiusSq;
                const auto d = 4.0f * edgeLengthSq;
                const auto intersectionLensRadiusSq = targetRadiusSq - ((n * n) / d);

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
            edgeData._outerColor[0] = static_cast<float>(edgeVisual._outerColor.redF());
            edgeData._outerColor[1] = static_cast<float>(edgeVisual._outerColor.greenF());
            edgeData._outerColor[2] = static_cast<float>(edgeVisual._outerColor.blueF());
            edgeData._innerColor[0] = static_cast<float>(edgeVisual._innerColor.redF());
            edgeData._innerColor[1] = static_cast<float>(edgeVisual._innerColor.greenF());
            edgeData._innerColor[2] = static_cast<float>(edgeVisual._innerColor.blueF());
            edgeData._selected = 0.0f;

            auto* gpuGraphData = gpuGraphDataForAlpha(componentRenderer->alpha(),
                edgeVisual._state.test(VisualFlags::Unhighlighted) ? UnhighlightedAlpha : 1.0f);

            if(gpuGraphData != nullptr)
            {
                gpuGraphData->_edgeData.push_back(edgeData);

                if(edgeData._selected != 0.0f)
                    gpuGraphData->_elementsSelected = true;

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
    _screenshotRenderer->requestPreview(*this, width, height, fillSize);
}

void GraphRenderer::onScreenshotRequested(int width, int height, const QString& path, int dpi, bool fillSize)
{
    _screenshotRenderer->requestScreenshot(*this, width, height, path, dpi, fillSize);
}

void GraphRenderer::updateComponentGPUData()
{
    //FIXME this doesn't necessarily need to be entirely regenerated and rebuffered
    // every frame, so it makes sense to do partial updates as and when required.
    // OTOH, it's probably not ever going to be masses of data, so maybe we should
    // just suck it up; need to get a profiler on it and see how long we're spending
    // here transfering the buffer, when there are lots of components
    resetGPUComponentData();

    for(const auto& componentRendererRef : _componentRenderers)
    {
        GraphComponentRenderer* componentRenderer = componentRendererRef;
        if(componentRenderer == nullptr)
        {
            qWarning() << "null component renderer";
            continue;
        }

        if(!componentRenderer->visible())
            continue;

        appendGPUComponentData(componentRenderer->modelViewMatrix(),
            componentRenderer->projectionMatrix(),
            componentRenderer->camera()->distance(),
            componentRenderer->lightScale());
    }

    uploadGPUComponentData();
}

void GraphRenderer::setScene(Scene* scene)
{
    if(_scene != nullptr)
    {
        _scene->setVisible(false);
        _scene->onHide();
    }

    _scene = scene;

    _scene->setViewportSize(width(), height());

    _scene->setProjection(_projection);
    _scene->setVisible(true);
    _scene->onShow();
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

Projection GraphRenderer::projection() const
{
    return _projection;
}

void GraphRenderer::setProjection(Projection projection)
{
    if(projection != _projection)
    {
        _projection = projection;

        if(_scene != nullptr)
            _scene->setProjection(projection);
    }
}

void GraphRenderer::resetTime()
{
    _time.start();
    _lastTime = 0.0f;
}

float GraphRenderer::secondsElapsed()
{
    float time = static_cast<float>(_time.elapsed()) / 1000.0f;
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
        if(!_graphComponentScene->focusedOnNodeAtRadius(nodeId, radius))
            rendererStartedTransition();

        executeOnRendererThread([this, nodeId, radius]
        {
            _graphComponentScene->moveFocusToNode(nodeId, radius);
        }, QStringLiteral("GraphRenderer::moveFocusToNode"));
    }
    else if(mode() == Mode::Overview)
    {
        // To focus on a node, we need to be in component mode
        auto componentId = _graphModel->graph().componentIdOfNode(nodeId);
        switchToComponentMode(true, componentId, nodeId, radius);
    }
}

void GraphRenderer::moveFocusToComponent(ComponentId componentId)
{
    if(mode() == Mode::Component)
    {
        if(componentId != _graphComponentScene->componentId())
            rendererStartedTransition();

        _graphComponentScene->setComponentId(componentId, true);
    }
}

void GraphRenderer::rendererStartedTransition()
{
    if(!_transitionPotentiallyInProgress)
    {
        _transitionPotentiallyInProgress = true;

        emit transitionStarted();
        resetTime();
    }
}

void GraphRenderer::rendererFinishedTransition()
{
    if(transitionActive())
        return;

    // The transition may have been initiated by changes that
    // require the GPU data to be updated, so ensure this is done
    updateGPUData(When::Later);

    _transitionPotentiallyInProgress = false;
    emit transitionFinished();
}

void GraphRenderer::sceneFinishedTransition()
{
    clearHiddenElements();
    updateGPUData(When::Later);
}

void GraphRenderer::executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description) // NOLINT readability-make-member-function-const
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
            auto* renderer = componentRendererForId(componentId);
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
    // Refuse to switch to overview mode if there is nothing to display
    if(_graphModel->graph().numComponents() <= 1)
        return;

    doTransition = doTransition && mode() != GraphRenderer::Mode::Overview &&
        _graphComponentScene->componentRenderer() != nullptr;

    if(doTransition)
        rendererStartedTransition();

    executeOnRendererThread([this, doTransition]
    {
        // So that we can return to the current view parameters later
        _graphComponentScene->saveViewData();

        if(doTransition)
        {
            if(!_graphComponentScene->viewIsReset())
            {
                _graphComponentScene->startTransition().then(
                [this]
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

void GraphRenderer::switchToComponentMode(bool doTransition, ComponentId componentId, NodeId nodeId, float radius)
{
    doTransition = doTransition && mode() != GraphRenderer::Mode::Component;

    if(doTransition)
        rendererStartedTransition();

    executeOnRendererThread([this, componentId, nodeId, radius, doTransition]
    {
        _graphComponentScene->setComponentId(componentId);

        if(doTransition)
        {
            _graphOverviewScene->startTransitionToComponentMode(_graphComponentScene->componentId()).then(
            [this, componentId, nodeId, radius]
            {
                if(!nodeId.isNull())
                {
                    Q_ASSERT(_graphModel->graph().componentIdOfNode(nodeId) == componentId);
                    componentRendererForId(componentId)->moveSavedFocusToNode(nodeId, radius);
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
    const auto* component = _graphModel->graph().componentById(componentId);
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
    if(key == QStringLiteral("visuals/textFont"))
    {
        _glyphMap->setFontName(value.toString());
        updateText();
    }
    else if(key == QStringLiteral("visuals/backgroundColor"))
        update();
}

void GraphRenderer::onCommandsStarted()
{
    disableSceneUpdate();
}

void GraphRenderer::onCommandsFinished()
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

void GraphRenderer::updateText(bool wait)
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
        job->executeWhenComplete([this]
        {
            executeOnRendererThread([this]
            {
                _sdfTexture.swap();
                _textLayoutResults = _glyphMap->results();

                updateGPUData(When::Later);
                update(); // QQuickFramebufferObject::Renderer::update
            }, QStringLiteral("GraphRenderer::updateText"));
        });

        _gpuComputeThread->enqueue(job);

        if(wait)
            _gpuComputeThread->wait();
    }
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

void GraphRenderer::processEventQueue()
{
    while(!_eventQueue.empty())
    {
        auto e = std::move(_eventQueue.front());
        _eventQueue.pop();

        auto* mouseEvent = dynamic_cast<QMouseEvent*>(e.get());
        auto* wheelEvent = dynamic_cast<QWheelEvent*>(e.get());
        auto* nativeEvent = dynamic_cast<QNativeGestureEvent*>(e.get());

        switch(e->type())
        {
        case QEvent::Type::MouseButtonPress:
            _interactor->mousePressEvent(mouseEvent->pos() * _devicePixelRatio,
                mouseEvent->modifiers(), mouseEvent->button());

            break;

        case QEvent::Type::MouseButtonRelease:
            _interactor->mouseReleaseEvent(mouseEvent->pos() * _devicePixelRatio,
                mouseEvent->modifiers(), mouseEvent->button());

            break;

        case QEvent::Type::MouseMove:
            _interactor->mouseMoveEvent(mouseEvent->pos() * _devicePixelRatio,
                mouseEvent->modifiers(), mouseEvent->button());

            break;

        case QEvent::Type::MouseButtonDblClick:
            _interactor->mouseDoubleClickEvent(mouseEvent->pos() * _devicePixelRatio,
                mouseEvent->modifiers(), mouseEvent->button());

            break;

        case QEvent::Type::Wheel:
            switch(wheelEvent->deviceType())
            {
            case QInputDevice::DeviceType::Mouse:
                _interactor->wheelEvent(wheelEvent->position().toPoint() * _devicePixelRatio,
                    wheelEvent->angleDelta().y());
                break;

            case QInputDevice::DeviceType::TouchScreen:
            case QInputDevice::DeviceType::TouchPad:
                _interactor->panGestureEvent(wheelEvent->position().toPoint() * _devicePixelRatio,
                    -wheelEvent->pixelDelta() * _devicePixelRatio);
                break;

            default: break;
            }

            break;

        case QEvent::Type::NativeGesture:
            if(nativeEvent->gestureType() == Qt::ZoomNativeGesture)
            {
                _interactor->zoomGestureEvent(
                    nativeEvent->position().toPoint() * _devicePixelRatio,
                    static_cast<float>(nativeEvent->value()));
            }

            break;

        default: break;
        }
    }
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
        processEventQueue();

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

    // Tail nodes aren't visible, so they can't be focused
    nodeIds.erase(std::remove_if(nodeIds.begin(), nodeIds.end(),
    [this](auto nodeId)
    {
        return _graphModel->graph().typeOf(nodeId) == MultiElementType::Tail;
    }), nodeIds.end());

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
        if(mode() == Mode::Component && u::pref(QStringLiteral("misc/stayInComponentMode")).toBool())
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
            return Mode::Overview;
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
        auto nodePosition = _graphModel->nodePositions().get(nodeId);
        points.at(i++) = nodePosition;
    }

    BoundingSphere boundingSphere(points);
    QVector3D centre = boundingSphere.centre();
    float minDistance = std::numeric_limits<float>::max();
    NodeId closestToCentreNodeId;
    for(auto nodeId : nodeIds)
    {
        auto nodePosition = _graphModel->nodePositions().get(nodeId);
        float distance = (centre - nodePosition).length();

        if(distance < minDistance)
        {
            minDistance = distance;
            closestToCentreNodeId = nodeId;
        }
    }

    radius = GraphComponentRenderer::maxNodeDistanceFromPoint(*_graphModel,
        _graphModel->nodePositions().get(closestToCentreNodeId), nodeIds);
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

    QQuickOpenGLUtils::resetOpenGLState();

    glViewport(0, 0, width(), height());

    updateScene();
    renderGraph();

    render2D(_selectionRect);

    // Check the normal FBO
    if(!framebufferObject()->bind())
        qWarning() << "QQuickFrameBufferobject::Renderer FBO not bound";

    renderToFramebuffer();

    QQuickOpenGLUtils::resetOpenGLState();

    _performanceCounter.tick();
}

void GraphRenderer::synchronize(QQuickFramebufferObject* item)
{
    _devicePixelRatio = item->window()->devicePixelRatio();

    auto* graphQuickItem = qobject_cast<GraphQuickItem*>(item);

    if(graphQuickItem->viewResetPending())
        resetView();

    if(graphQuickItem->overviewModeSwitchPending())
        switchToOverviewMode();

    setProjection(graphQuickItem->projection());
    setShading(graphQuickItem->projection() == Projection::TwoDee ?
        graphQuickItem->shading2D() : graphQuickItem->shading3D());

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
        auto& newEvents = graphQuickItem->events();
        while(!newEvents.empty())
        {
            _eventQueue.push(std::move(newEvents.front()));
            newEvents.pop();
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

void GraphRenderer::ifSceneUpdateEnabled(const std::function<void()>& f) const
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
