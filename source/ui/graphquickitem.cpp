#include "graphquickitem.h"

#include "../rendering/graphrenderer.h"

#include "../commands/commandmanager.h"

GraphQuickItem::GraphQuickItem(QQuickItem* parent) :
    QQuickFramebufferObject(parent),
    _viewResetPending(false),
    _overviewModeSwitchPending(false),
    _interacting(false),
    _viewIsReset(true),
    _canEnterOverviewMode(false)
{
    // Prevent updates until we're properly initialised
    setFlag(Flag::ItemHasContents, false);

    setAcceptedMouseButtons(Qt::AllButtons);
}

void GraphQuickItem::initialise(std::shared_ptr<GraphModel> graphModel,
                                CommandManager& commandManager,
                                std::shared_ptr<SelectionManager> selectionManager)
{
    _graphModel = graphModel;
    _commandManager = &commandManager;
    _selectionManager = selectionManager;

    setFlag(Flag::ItemHasContents, true);
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

void GraphQuickItem::setInteracting(bool interacting)
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

QQuickFramebufferObject::Renderer* GraphQuickItem::createRenderer() const
{
    auto graphRenderer = new GraphRenderer(_graphModel, *_commandManager, _selectionManager);

    connect(this, &GraphQuickItem::commandWillExecuteAsynchronously, graphRenderer, &GraphRenderer::onCommandWillExecuteAsynchronously);
    connect(this, &GraphQuickItem::commandCompleted, graphRenderer, &GraphRenderer::onCommandCompleted);
    connect(this, &GraphQuickItem::commandCompleted, this, &GraphQuickItem::update);

    connect(graphRenderer, &GraphRenderer::modeChanged, this, &GraphQuickItem::update);
    connect(graphRenderer, &GraphRenderer::userInteractionStarted, this, &GraphQuickItem::userInteractionStarted);
    connect(graphRenderer, &GraphRenderer::userInteractionFinished, this, &GraphQuickItem::userInteractionFinished);
    connect(graphRenderer, &GraphRenderer::userInteractionStarted, [this] { setInteracting(true); });
    connect(graphRenderer, &GraphRenderer::userInteractionFinished, [this] { setInteracting(false); });
    connect(graphRenderer, &GraphRenderer::taskAddedToExecutor, this, &GraphQuickItem::update);

    return graphRenderer;
}

// FIXME this is a hack to undo/cancel out, the Y flip that's applied to our target FBO
// when the QQ scene graph is rendered. See https://bugreports.qt.io/browse/QTBUG-41073
#include <QSGSimpleTextureNode>
QSGNode* GraphQuickItem::updatePaintNode(QSGNode* node, UpdatePaintNodeData* nodeData)
{
    if(!node)
    {
        node = QQuickFramebufferObject::updatePaintNode(node, nodeData);
        QSGSimpleTextureNode* n = static_cast<QSGSimpleTextureNode*>(node);
        if(n)
            n->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
        return node;
    }

    return QQuickFramebufferObject::updatePaintNode(node, nodeData);
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

void GraphQuickItem::onLayoutChanged()
{
    update();
}

void GraphQuickItem::mousePressEvent(QMouseEvent* e)        { enqueueEvent(e); }
void GraphQuickItem::mouseReleaseEvent(QMouseEvent* e)      { enqueueEvent(e); }
void GraphQuickItem::mouseMoveEvent(QMouseEvent* e)         { enqueueEvent(e); }
void GraphQuickItem::mouseDoubleClickEvent(QMouseEvent* e)  { enqueueEvent(e); }
void GraphQuickItem::wheelEvent(QWheelEvent* e)             { enqueueEvent(e); }
