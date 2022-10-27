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

#include "graphquickitem.h"

#include "shared/utils/static_block.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include "rendering/graphrenderer.h"

#include "commands/commandmanager.h"

#include <QOpenGLContext>
#include <QQmlEngine>

GraphQuickItem::GraphQuickItem(QQuickItem* parent) :
    QQuickFramebufferObject(parent)
{
    // Prevent updates until we're properly initialised
    setFlag(Flag::ItemHasContents, false);

    setMirrorVertically(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

void GraphQuickItem::initialise(GraphModel* graphModel,
                                CommandManager* commandManager,
                                SelectionManager* selectionManager,
                                GPUComputeThread* gpuComputeThread)
{
    _graphModel = graphModel;
    _commandManager = commandManager;
    _selectionManager = selectionManager;
    _gpuComputeThread = gpuComputeThread;

    setFlag(Flag::ItemHasContents, true);

    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphQuickItem::graphChanged);
    connect(&_graphModel->graph(), &Graph::graphChanged, [this] { updateVisibleComponentIndex(); });

    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphQuickItem::metricsChanged);
    connect(this, &GraphQuickItem::focusedComponentIdChanged, this, &GraphQuickItem::metricsChanged);

    emit graphChanged();
    emit metricsChanged();

    // Force an initial update; this will usually occur anyway for other reasons,
    // but it can't hurt to do it unconditionally too
    update();
}

void GraphQuickItem::resetView()
{
    _viewResetPending = true;
    update();
}

bool GraphQuickItem::viewResetPending()
{
    bool b = _viewResetPending;
    _viewResetPending = false;
    return b;
}

Projection GraphQuickItem::projection() const
{
    return _projection;
}

void GraphQuickItem::setProjection(Projection projection)
{
    if(projection != _projection)
    {
        _projection = projection;
        update();
    }
}

Shading GraphQuickItem::shading2D() const
{
    return _shading2D;
}

void GraphQuickItem::setShading2D(Shading shading2D)
{
    if(shading2D != _shading2D)
    {
        _shading2D = shading2D;
        update();
    }
}

Shading GraphQuickItem::shading3D() const
{
    return _shading3D;
}

void GraphQuickItem::setShading3D(Shading shading3D)
{
    if(shading3D != _shading3D)
    {
        _shading3D = shading3D;
        update();
    }
}

void GraphQuickItem::setInteracting(bool interacting) const
{
    if(_interacting != interacting)
    {
        _interacting = interacting;
        emit interactingChanged();
    }
}

void GraphQuickItem::setTransitioning(bool transitioning) const
{
    if(_transitioning != transitioning)
    {
        _transitioning = transitioning;
        emit transitioningChanged();
    }
}

bool GraphQuickItem::viewIsReset() const
{
    return _viewIsReset;
}

bool GraphQuickItem::inOverviewMode() const
{
    return _inOverviewMode;
}

void GraphQuickItem::setViewIsReset(bool viewIsReset)
{
    if(_viewIsReset != viewIsReset)
    {
        _viewIsReset = viewIsReset;
        emit viewIsResetChanged();
    }
}

bool GraphQuickItem::canEnterOverviewMode() const
{
    return _canEnterOverviewMode;
}

void GraphQuickItem::setCanEnterOverviewMode(bool canEnterOverviewMode)
{
    if(_canEnterOverviewMode != canEnterOverviewMode)
    {
        _canEnterOverviewMode = canEnterOverviewMode;
        emit canEnterOverviewModeChanged();
    }
}

void GraphQuickItem::setInOverviewMode(bool inOverviewMode)
{
    if(_inOverviewMode != inOverviewMode)
    {
        _inOverviewMode = inOverviewMode;
        emit inOverviewModeChanged();
    }
}

void GraphQuickItem::setFocusedComponentId(ComponentId componentId)
{
    if(_focusedComponentId != componentId)
    {
        _focusedComponentId = componentId;
        updateVisibleComponentIndex();
        emit focusedComponentIdChanged();
    }
}

void GraphQuickItem::captureScreenshot(int width, int height, const QString& path, int dpi, bool fillSize)
{
    if(width <= 0 || height <= 0)
        return;

    emit screenshotRequested(width, height, path, dpi, fillSize);
    update();
}

void GraphQuickItem::requestPreview(int width, int height, bool fillSize)
{
    if(width <= 0 || height <= 0)
        return;

    emit previewRequested(width, height, fillSize);
    update();
}

void GraphQuickItem::switchToOverviewMode(bool)
{
    _overviewModeSwitchPending = true;
    update();
}

bool GraphQuickItem::overviewModeSwitchPending()
{
    bool b = _overviewModeSwitchPending;
    _overviewModeSwitchPending = false;
    return b;
}

void GraphQuickItem::moveFocusToNode(NodeId nodeId)
{
    _desiredFocusNodeIds = {nodeId};
    update();
}

void GraphQuickItem::moveFocusToNodes(const std::vector<NodeId>& nodeIds)
{
    _desiredFocusNodeIds = nodeIds;
    update();
}

std::vector<NodeId> GraphQuickItem::desiredFocusNodeIds()
{
    auto nodeIds = _desiredFocusNodeIds;
    _desiredFocusNodeIds.clear();
    return nodeIds;
}

void GraphQuickItem::moveFocusToComponent(ComponentId componentId)
{
    _desiredFocusComponentId = componentId;
    update();
}

ComponentId GraphQuickItem::desiredFocusComponentId()
{
    ComponentId componentId = _desiredFocusComponentId;
    _desiredFocusComponentId.setToNull();
    return componentId;
}

ComponentId GraphQuickItem::focusedComponentId() const
{
    return _focusedComponentId;
}

QQuickFramebufferObject::Renderer* GraphQuickItem::createRenderer() const
{
    // The compute thread must be initialised where there is a current OpenGL
    // context available, and this is as good a place as any for that
    _gpuComputeThread->initialise();

    auto* graphRenderer = new GraphRenderer(_graphModel, _commandManager, _selectionManager, _gpuComputeThread);
    connect(this, &GraphQuickItem::commandsStarted, graphRenderer, &GraphRenderer::onCommandsStarted, Qt::DirectConnection);
    connect(this, &GraphQuickItem::commandsFinished, graphRenderer, &GraphRenderer::onCommandsFinished, Qt::DirectConnection);
    connect(this, &GraphQuickItem::commandsFinished, this, &GraphQuickItem::updateRenderer, Qt::DirectConnection);
    connect(this, &GraphQuickItem::layoutChanged, graphRenderer, &GraphRenderer::onLayoutChanged);
    connect(this, &GraphQuickItem::screenshotRequested, graphRenderer, &GraphRenderer::onScreenshotRequested);
    connect(this, &GraphQuickItem::previewRequested, graphRenderer, &GraphRenderer::onPreviewRequested);

    connect(graphRenderer, &GraphRenderer::initialised, this, &GraphQuickItem::onRendererInitialised);
    connect(graphRenderer, &GraphRenderer::previewComplete, this, &GraphQuickItem::previewComplete);
    connect(graphRenderer, &GraphRenderer::screenshotComplete, this, &GraphQuickItem::onScreenshotComplete);
    connect(graphRenderer, &GraphRenderer::modeChanged, this, &GraphQuickItem::update);
    connect(graphRenderer, &GraphRenderer::userInteractionStarted, this, &GraphQuickItem::onUserInteractionStarted);
    connect(graphRenderer, &GraphRenderer::userInteractionFinished, this, &GraphQuickItem::onUserInteractionFinished);
    connect(graphRenderer, &GraphRenderer::transitionStarted, this, &GraphQuickItem::onTransitionStarted);
    connect(graphRenderer, &GraphRenderer::transitionFinished, this, &GraphQuickItem::onTransitionFinished);
    connect(graphRenderer, &GraphRenderer::taskAddedToExecutor, this, &GraphQuickItem::update);

    connect(graphRenderer, &GraphRenderer::synchronizeComplete, this, &GraphQuickItem::onSynchronizeComplete);

    connect(graphRenderer, &GraphRenderer::clicked, this, &GraphQuickItem::clicked);

    connect(graphRenderer, &GraphRenderer::fpsChanged, this, &GraphQuickItem::onFPSChanged);

    return graphRenderer;
}

bool GraphQuickItem::event(QEvent* e)
{
    switch(e->type())
    {
    case QEvent::Type::NativeGesture: enqueueEvent(dynamic_cast<QNativeGestureEvent*>(e)); return true;
    default: break;
    }

    return QQuickItem::event(e);
}

void GraphQuickItem::onLayoutChanged()
{
    update();
    emit layoutChanged();
}

void GraphQuickItem::onRendererInitialised()
{
    _initialised = true;
    emit initialisedChanged();
}

void GraphQuickItem::onSynchronizeComplete()
{
    bool changed = _updating;

    _updating = false;

    if(changed)
        emit updatingChanged();
}

void GraphQuickItem::onFPSChanged(float fps)
{
    _fps = fps;
    emit fpsChanged();
}

void GraphQuickItem::onUserInteractionStarted() const
{
    setInteracting(true);
}

void GraphQuickItem::onUserInteractionFinished()
{
    setInteracting(false);

    // Force a call to GraphRenderer::synchronize so that any
    // pending renderer state gets reflected in the QuickItem
    update();
}

void GraphQuickItem::onTransitionStarted() const
{
    setTransitioning(true);
}

void GraphQuickItem::onTransitionFinished()
{
    setTransitioning(false);

    // Force a call to GraphRenderer::synchronize so that any
    // pending renderer state gets reflected in the QuickItem
    update();
}

void GraphQuickItem::onScreenshotComplete(const QImage& screenshot, const QString& path)
{
    _commandManager->executeOnce([screenshot, path](Command&)
    {
        // Ensure local filesystem path
        screenshot.save(QUrl(path).toLocalFile());
        QDesktopServices::openUrl(QUrl(path).toLocalFile());
    }, {tr("Save Screenshot"), tr("Saving Screenshot"), tr("Screenshot Saved")});
}

void GraphQuickItem::mousePressEvent(QMouseEvent* e)
{
    // Any mouse press events cause us to get focus, not so much because we
    // need focus ourselves, but we want other controls to lose focus
    setFocus(true);

    enqueueEvent(e);
}

void GraphQuickItem::mouseReleaseEvent(QMouseEvent* e)      { enqueueEvent(e); }
void GraphQuickItem::mouseMoveEvent(QMouseEvent* e)         { enqueueEvent(e); }
void GraphQuickItem::mouseDoubleClickEvent(QMouseEvent* e)  { enqueueEvent(e); }
void GraphQuickItem::wheelEvent(QWheelEvent* e)             { enqueueEvent(e); }

const IGraphComponent* GraphQuickItem::focusedComponent() const
{
    return !_focusedComponentId.isNull() && _graphModel->graph().containsComponentId(_focusedComponentId) ?
        _graphModel->graph().componentById(_focusedComponentId) : nullptr;
}

int GraphQuickItem::numNodes() const
{
    if(_graphModel != nullptr)
        return focusedComponent() != nullptr ? focusedComponent()->numNodes() : _graphModel->graph().numNodes();

    return -1;
}

int GraphQuickItem::numVisibleNodes() const
{
    if(_graphModel != nullptr)
    {
        const auto& nodeIds = focusedComponent() != nullptr ? focusedComponent()->nodeIds() : _graphModel->graph().nodeIds();

        return static_cast<int>(std::count_if(nodeIds.begin(), nodeIds.end(),
        [this](NodeId nodeId)
        {
            return _graphModel->graph().typeOf(nodeId) != MultiElementType::Tail;
        }));
    }

    return -1;
}

int GraphQuickItem::numEdges() const
{
    if(_graphModel != nullptr)
        return focusedComponent() != nullptr ? focusedComponent()->numEdges() : _graphModel->graph().numEdges();

    return -1;
}

int GraphQuickItem::numVisibleEdges() const
{
    if(_graphModel != nullptr)
    {
        const auto& edgeIds = focusedComponent() != nullptr ? focusedComponent()->edgeIds() : _graphModel->graph().edgeIds();

        return static_cast<int>(std::count_if(edgeIds.begin(), edgeIds.end(),
        [this](EdgeId edgeId)
        {
            return _graphModel->graph().typeOf(edgeId) != MultiElementType::Tail;
        }));
    }

    return -1;
}

int GraphQuickItem::numComponents() const
{
    if(_graphModel != nullptr)
        return _graphModel->graph().numComponents();

    return -1;
}

void GraphQuickItem::updateVisibleComponentIndex()
{
    const auto& componentIds = _graphModel->graph().componentIds();
    _visibleComponentIndex = static_cast<int>(std::distance(componentIds.begin(),
        std::find(componentIds.begin(), componentIds.end(), _focusedComponentId)) + 1);

    emit visibleComponentIndexChanged();
}

void GraphQuickItem::updateRenderer()
{
    bool changed = !_updating;
    _updating = true;

    if(changed)
        emit updatingChanged();

    update();
}

static_block
{
    qmlRegisterType<GraphQuickItem>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "Graph");
}
