/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "graphdisplay.h"

#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"

#include "app/rendering/graphrenderer.h"

#include "app/commands/commandmanager.h"

#include <QQmlEngine>
#include <QtGlobal>

GraphDisplay::GraphDisplay(QQuickItem* parent) :
    QQuickFramebufferObject(parent)
{
    // Prevent updates until we're properly initialised
    setFlag(Flag::ItemHasContents, false);

    setMirrorVertically(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

void GraphDisplay::initialise(GraphModel* graphModel, CommandManager* commandManager, SelectionManager* selectionManager)
{
    _graphModel = graphModel;
    _commandManager = commandManager;
    _selectionManager = selectionManager;

    setFlag(Flag::ItemHasContents, true);

    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphDisplay::graphChanged);
    connect(&_graphModel->graph(), &Graph::graphChanged, [this] { updateVisibleComponentIndex(); });

    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphDisplay::metricsChanged);
    connect(this, &GraphDisplay::focusedComponentIdChanged, this, &GraphDisplay::metricsChanged);

    emit graphChanged();
    emit metricsChanged();

    // Force an initial update; this will usually occur anyway for other reasons,
    // but it can't hurt to do it unconditionally too
    update();
}

void GraphDisplay::resetView()
{
    _viewResetPending = true;
    update();
}

bool GraphDisplay::viewResetPending()
{
    const bool b = _viewResetPending;
    _viewResetPending = false;
    return b;
}

Projection GraphDisplay::projection() const
{
    return _projection;
}

void GraphDisplay::setProjection(Projection projection)
{
    if(projection != _projection)
    {
        _projection = projection;
        update();
    }
}

Shading GraphDisplay::shading2D() const
{
    return _shading2D;
}

void GraphDisplay::setShading2D(Shading shading2D)
{
    if(shading2D != _shading2D)
    {
        _shading2D = shading2D;
        update();
    }
}

Shading GraphDisplay::shading3D() const
{
    return _shading3D;
}

void GraphDisplay::setShading3D(Shading shading3D)
{
    if(shading3D != _shading3D)
    {
        _shading3D = shading3D;
        update();
    }
}

void GraphDisplay::setInteracting(bool interacting) const
{
    if(_interacting != interacting)
    {
        _interacting = interacting;
        emit interactingChanged();
    }
}

void GraphDisplay::setTransitioning(bool transitioning) const
{
    if(_transitioning != transitioning)
    {
        _transitioning = transitioning;
        emit transitioningChanged();
    }
}

bool GraphDisplay::viewIsReset() const
{
    return _viewIsReset;
}

bool GraphDisplay::inOverviewMode() const
{
    return _inOverviewMode;
}

void GraphDisplay::setViewIsReset(bool viewIsReset)
{
    if(_viewIsReset != viewIsReset)
    {
        _viewIsReset = viewIsReset;
        emit viewIsResetChanged();
    }
}

bool GraphDisplay::canEnterOverviewMode() const
{
    return _canEnterOverviewMode;
}

void GraphDisplay::setCanEnterOverviewMode(bool canEnterOverviewMode)
{
    if(_canEnterOverviewMode != canEnterOverviewMode)
    {
        _canEnterOverviewMode = canEnterOverviewMode;
        emit canEnterOverviewModeChanged();
    }
}

void GraphDisplay::setInOverviewMode(bool inOverviewMode)
{
    if(_inOverviewMode != inOverviewMode)
    {
        _inOverviewMode = inOverviewMode;
        emit inOverviewModeChanged();
    }
}

void GraphDisplay::setFocusedComponentId(ComponentId componentId)
{
    if(_focusedComponentId != componentId)
    {
        _focusedComponentId = componentId;
        updateVisibleComponentIndex();
        emit focusedComponentIdChanged();
    }
}

void GraphDisplay::captureScreenshot(int width, int height, const QString& path, int dpi, bool fillSize)
{
    if(width <= 0 || height <= 0)
        return;

    emit screenshotRequested(width, height, path, dpi, fillSize);
    update();
}

void GraphDisplay::requestPreview(int width, int height, bool fillSize)
{
    if(width <= 0 || height <= 0)
        return;

    emit previewRequested(width, height, fillSize);
    update();
}

void GraphDisplay::switchToOverviewMode(bool)
{
    _overviewModeSwitchPending = true;
    update();
}

bool GraphDisplay::overviewModeSwitchPending()
{
    const bool b = _overviewModeSwitchPending;
    _overviewModeSwitchPending = false;
    return b;
}

void GraphDisplay::moveFocusToNode(NodeId nodeId)
{
    _desiredFocusNodeIds = {nodeId};
    update();
}

void GraphDisplay::moveFocusToNodes(const std::vector<NodeId>& nodeIds)
{
    _desiredFocusNodeIds = nodeIds;
    update();
}

std::vector<NodeId> GraphDisplay::desiredFocusNodeIds()
{
    auto nodeIds = _desiredFocusNodeIds;
    _desiredFocusNodeIds.clear();
    return nodeIds;
}

void GraphDisplay::moveFocusToComponent(ComponentId componentId)
{
    _desiredFocusComponentId = componentId;
    update();
}

ComponentId GraphDisplay::desiredFocusComponentId()
{
    ComponentId componentId = _desiredFocusComponentId;
    _desiredFocusComponentId.setToNull();
    return componentId;
}

ComponentId GraphDisplay::focusedComponentId() const
{
    return _focusedComponentId;
}

QQuickFramebufferObject::Renderer* GraphDisplay::createRenderer() const
{
    auto* graphRenderer = new GraphRenderer(_graphModel, _commandManager, _selectionManager);
    connect(this, &GraphDisplay::commandsStarted, graphRenderer, &GraphRenderer::onCommandsStarted, Qt::DirectConnection);
    connect(this, &GraphDisplay::commandsFinished, graphRenderer, &GraphRenderer::onCommandsFinished, Qt::DirectConnection);
    connect(this, &GraphDisplay::commandsFinished, this, &GraphDisplay::updateRenderer, Qt::DirectConnection);
    connect(this, &GraphDisplay::layoutChanged, graphRenderer, &GraphRenderer::onLayoutChanged);
    connect(this, &GraphDisplay::screenshotRequested, graphRenderer, &GraphRenderer::onScreenshotRequested);
    connect(this, &GraphDisplay::previewRequested, graphRenderer, &GraphRenderer::onPreviewRequested);

    connect(graphRenderer, &GraphRenderer::initialised, this, &GraphDisplay::onRendererInitialised);
    connect(graphRenderer, &GraphRenderer::previewComplete, this, &GraphDisplay::previewComplete);
    connect(graphRenderer, &GraphRenderer::screenshotComplete, this, &GraphDisplay::onScreenshotComplete);
    connect(graphRenderer, &GraphRenderer::modeChanged, this, &GraphDisplay::update);
    connect(graphRenderer, &GraphRenderer::userInteractionStarted, this, &GraphDisplay::onUserInteractionStarted);
    connect(graphRenderer, &GraphRenderer::userInteractionFinished, this, &GraphDisplay::onUserInteractionFinished);
    connect(graphRenderer, &GraphRenderer::transitionStarted, this, &GraphDisplay::onTransitionStarted);
    connect(graphRenderer, &GraphRenderer::transitionFinished, this, &GraphDisplay::onTransitionFinished);
    connect(graphRenderer, &GraphRenderer::taskAddedToExecutor, this, &GraphDisplay::update);

    connect(graphRenderer, &GraphRenderer::synchronizeComplete, this, &GraphDisplay::onSynchronizeComplete);

    connect(graphRenderer, &GraphRenderer::clicked, this, &GraphDisplay::clicked);

    connect(graphRenderer, &GraphRenderer::fpsChanged, this, &GraphDisplay::onFPSChanged);

    return graphRenderer;
}

bool GraphDisplay::event(QEvent* e)
{
    switch(e->type())
    {
    case QEvent::Type::NativeGesture: enqueueEvent(dynamic_cast<QNativeGestureEvent*>(e)); return true;
    default: break;
    }

    return QQuickItem::event(e);
}

void GraphDisplay::onLayoutChanged()
{
    update();
    emit layoutChanged();
}

void GraphDisplay::onRendererInitialised()
{
    _initialised = true;
    emit initialisedChanged();
}

void GraphDisplay::onSynchronizeComplete()
{
    const bool changed = _updating;

    _updating = false;

    if(changed)
        emit updatingChanged();
}

void GraphDisplay::onFPSChanged(float fps)
{
    _fps = fps;
    emit fpsChanged();
}

void GraphDisplay::onUserInteractionStarted() const
{
    setInteracting(true);
}

void GraphDisplay::onUserInteractionFinished()
{
    setInteracting(false);

    // Force a call to GraphRenderer::synchronize so that any
    // pending renderer state gets reflected in the QuickItem
    update();
}

void GraphDisplay::onTransitionStarted() const
{
    setTransitioning(true);
}

void GraphDisplay::onTransitionFinished()
{
    setTransitioning(false);

    // Force a call to GraphRenderer::synchronize so that any
    // pending renderer state gets reflected in the QuickItem
    update();
}

void GraphDisplay::onScreenshotComplete(const QImage& screenshot, const QString& path)
{
    _commandManager->executeOnce([screenshot, path](Command&)
    {
        // Ensure local filesystem path
        screenshot.save(QUrl(path).toLocalFile());
#ifndef Q_OS_WASM
        QDesktopServices::openUrl(QUrl(path).toLocalFile());
#endif
    }, {tr("Save Screenshot"), tr("Saving Screenshot"), tr("Screenshot Saved")});
}

void GraphDisplay::mousePressEvent(QMouseEvent* e)
{
    // Any mouse press events cause us to get focus, not so much because we
    // need focus ourselves, but we want other controls to lose focus
    setFocus(true);

    enqueueEvent(e);
}

void GraphDisplay::mouseReleaseEvent(QMouseEvent* e)        { enqueueEvent(e); }
void GraphDisplay::mouseMoveEvent(QMouseEvent* e)           { enqueueEvent(e); }
void GraphDisplay::mouseDoubleClickEvent(QMouseEvent* e)    { enqueueEvent(e); }
void GraphDisplay::wheelEvent(QWheelEvent* e)               { enqueueEvent(e); }

const IGraphComponent* GraphDisplay::focusedComponent() const
{
    return !_focusedComponentId.isNull() && _graphModel->graph().containsComponentId(_focusedComponentId) ?
        _graphModel->graph().componentById(_focusedComponentId) : nullptr;
}

size_t GraphDisplay::numNodes() const
{
    if(_graphModel != nullptr)
        return focusedComponent() != nullptr ? focusedComponent()->numNodes() : _graphModel->graph().numNodes();

    return 0;
}

size_t GraphDisplay::numVisibleNodes() const
{
    if(_graphModel != nullptr)
    {
        const auto& nodeIds = focusedComponent() != nullptr ? focusedComponent()->nodeIds() : _graphModel->graph().nodeIds();

        return static_cast<size_t>(std::count_if(nodeIds.begin(), nodeIds.end(),
        [this](NodeId nodeId)
        {
            return _graphModel->graph().typeOf(nodeId) != MultiElementType::Tail;
        }));
    }

    return 0;
}

size_t GraphDisplay::numEdges() const
{
    if(_graphModel != nullptr)
        return focusedComponent() != nullptr ? focusedComponent()->numEdges() : _graphModel->graph().numEdges();

    return 0;
}

size_t GraphDisplay::numVisibleEdges() const
{
    if(_graphModel != nullptr)
    {
        const auto& edgeIds = focusedComponent() != nullptr ? focusedComponent()->edgeIds() : _graphModel->graph().edgeIds();

        return static_cast<size_t>(std::count_if(edgeIds.begin(), edgeIds.end(),
        [this](EdgeId edgeId)
        {
            return _graphModel->graph().typeOf(edgeId) != MultiElementType::Tail;
        }));
    }

    return 0;
}

size_t GraphDisplay::numComponents() const
{
    if(_graphModel != nullptr)
        return _graphModel->graph().numComponents();

    return 0;
}

void GraphDisplay::updateVisibleComponentIndex()
{
    const auto& componentIds = _graphModel->graph().componentIds();
    _visibleComponentIndex = static_cast<int>(std::distance(componentIds.begin(),
        std::find(componentIds.begin(), componentIds.end(), _focusedComponentId)) + 1);

    emit visibleComponentIndexChanged();
}

void GraphDisplay::updateRenderer()
{
    const bool changed = !_updating;
    _updating = true;

    if(changed)
        emit updatingChanged();

    update();
}
