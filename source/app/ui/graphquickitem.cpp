#include "graphquickitem.h"

#include "graph/graphmodel.h"

#include "rendering/graphrenderer.h"

#include "commands/commandmanager.h"
#include <QOffscreenSurface>
#include <QOpenGLContext>


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
    emit graphChanged();

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

bool GraphQuickItem::interacting() const
{
    return _interacting;
}

void GraphQuickItem::setInteracting(bool interacting) const
{
    if(_interacting != interacting)
    {
        _interacting = interacting;
        emit interactingChanged();
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

void GraphQuickItem::captureScreenshot(int width, int height, QString path, int dpi, bool fillSize)
{
    emit screenshotRequested(width, height, path, dpi, fillSize);
    update();
}

void GraphQuickItem::requestPreview(int width, int height, bool fillSize)
{
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
    _desiredFocusNodeId = nodeId;
    update();
}

NodeId GraphQuickItem::desiredFocusNodeId()
{
    NodeId nodeId = _desiredFocusNodeId;
    _desiredFocusNodeId.setToNull();
    return nodeId;
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

    auto graphRenderer = new GraphRenderer(_graphModel, _commandManager, _selectionManager, _gpuComputeThread);
    connect(this, &GraphQuickItem::commandWillExecute, graphRenderer, &GraphRenderer::onCommandWillExecute, Qt::DirectConnection);
    connect(this, &GraphQuickItem::commandCompleted, graphRenderer, &GraphRenderer::onCommandCompleted, Qt::DirectConnection);
    connect(this, &GraphQuickItem::commandCompleted, this, &GraphQuickItem::update);
    connect(this, &GraphQuickItem::layoutChanged, graphRenderer, &GraphRenderer::onLayoutChanged);
    connect(this, &GraphQuickItem::screenshotRequested, graphRenderer, &GraphRenderer::onScreenshotRequested);
    connect(this, &GraphQuickItem::previewRequested, graphRenderer, &GraphRenderer::onPreviewRequested);

    connect(graphRenderer, &GraphRenderer::previewComplete, this, &GraphQuickItem::previewComplete);
    connect(graphRenderer, &GraphRenderer::screenshotComplete, this, &GraphQuickItem::onScreenshotComplete);
    connect(graphRenderer, &GraphRenderer::modeChanged, this, &GraphQuickItem::update);
    connect(graphRenderer, &GraphRenderer::userInteractionStarted, this, &GraphQuickItem::onUserInteractionStarted);
    connect(graphRenderer, &GraphRenderer::userInteractionFinished, this, &GraphQuickItem::onUserInteractionFinished);
    connect(graphRenderer, &GraphRenderer::taskAddedToExecutor, this, &GraphQuickItem::update);

    connect(graphRenderer, &GraphRenderer::fpsChanged, this, &GraphQuickItem::onFPSChanged);

    return graphRenderer;
}

bool GraphQuickItem::eventsPending() { return !_eventQueue.empty(); }

std::unique_ptr<QEvent> GraphQuickItem::nextEvent()
{
    if(eventsPending())
    {
        auto e = std::move(_eventQueue.front());
        _eventQueue.pop();
        return e;
    }

    return {};
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

void GraphQuickItem::onFPSChanged(float fps)
{
    _fps = fps;
    emit fpsChanged();
}

void GraphQuickItem::onUserInteractionStarted()
{
    setInteracting(true);
}

void GraphQuickItem::onUserInteractionFinished()
{
    setInteracting(false);
}

void GraphQuickItem::onScreenshotComplete(QImage screenshot, QString path)
{
    _commandManager->executeOnce({tr("Save Screenshot"),
                                 tr("Saving Screenshot")},
                                 [screenshot, path](Command&)
    {
        // Ensure local filesystem path
        screenshot.save(QUrl(path).toLocalFile());
        QDesktopServices::openUrl(QUrl(path).toLocalFile());
    });
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

int GraphQuickItem::numNodes() const
{
    if(_graphModel != nullptr)
        return _graphModel->graph().numNodes();

    return -1;
}

int GraphQuickItem::numVisibleNodes() const
{
    if(_graphModel != nullptr)
    {
        return std::count_if(_graphModel->graph().nodeIds().begin(), _graphModel->graph().nodeIds().end(),
        [this](NodeId nodeId)
        {
            return _graphModel->graph().typeOf(nodeId) != MultiElementType::Tail;
        });
    }

    return -1;
}

int GraphQuickItem::numEdges() const
{
    if(_graphModel != nullptr)
        return _graphModel->graph().numEdges();

    return -1;
}

int GraphQuickItem::numVisibleEdges() const
{
    if(_graphModel != nullptr)
    {
        return std::count_if(_graphModel->graph().edgeIds().begin(), _graphModel->graph().edgeIds().end(),
        [this](EdgeId edgeId)
        {
            return _graphModel->graph().typeOf(edgeId) != MultiElementType::Tail;
        });
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
    _visibleComponentIndex = std::distance(componentIds.begin(),
        std::find(componentIds.begin(), componentIds.end(), _focusedComponentId)) + 1;

    emit visibleComponentIndexChanged();
}
