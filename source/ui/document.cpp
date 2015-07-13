#include "document.h"

#include "../application.h"

#include "../loading/graphfileparser.h"
#include "../graph/graphmodel.h"

#include "../layout/layout.h"
#include "../layout/forcedirectedlayout.h"
#include "../layout/collision.h"

#include "../commands/deleteselectednodescommand.h"

#include "selectionmanager.h"
#include "graphquickitem.h"

#include "../utils/cpp1x_hacks.h"

Document::Document(QObject* parent) :
    QObject(parent),
    _loadProgress(0),
    _loadComplete(false),
    _autoResume(0)
{
}

bool Document::commandInProgress() const
{
    return !_loadComplete || _commandManager.busy();
}

bool Document::idle() const
{
    return !commandInProgress() && !_graphQuickItem->interacting();
}

bool Document::canDelete() const
{
    if(_selectionManager != nullptr)
        return idle() && !_selectionManager->selectedNodes().empty() && _graphModel->editable();

    return false;
}

int Document::commandProgress() const
{
    if(!_loadComplete)
        return _loadProgress;

    return _commandManager.commandProgress();
}

QString Document::commandVerb() const
{
    if(!_loadComplete)
        return QString(tr("Loading %1").arg(_title));

    return _commandManager.commandVerb();
}

void Document::pauseLayout(bool autoResume)
{
    std::unique_lock<std::recursive_mutex> lock(_autoResumeMutex);

    if(_layoutThread)
    {
        if(!autoResume)
            _layoutThread->pauseAndWait();
        else
        {
            if(_autoResume > 0 || !_layoutThread->paused())
            {
                _autoResume++;
                emit layoutIsPausedChanged();
            }

            if(_autoResume == 1)
                _layoutThread->pauseAndWait();
        }
    }

}

bool Document::layoutIsPaused()
{
    std::unique_lock<std::recursive_mutex> lock(_autoResumeMutex);

    // Not a typo: a non-existant thread counts as paused
    bool nodeLayoutPaused = (_layoutThread == nullptr || _layoutThread->paused()) &&
            _autoResume == 0;

    return nodeLayoutPaused;
}

void Document::resumeLayout(bool autoResume)
{
    std::unique_lock<std::recursive_mutex> lock(_autoResumeMutex);

    if(_layoutThread)
    {
        if(!autoResume)
            _layoutThread->resume();
        else
        {
            if(_autoResume == 1)
                _layoutThread->resume();

            if(_autoResume > 0)
            {
                _autoResume--;
                emit layoutIsPausedChanged();
            }
        }
    }
}

bool Document::canUndo() const
{
    return idle() && _commandManager.canUndo();
}

QString Document::nextUndoAction() const
{
    return _commandManager.nextUndoAction();
}

bool Document::canRedo() const
{
    return idle() && _commandManager.canRedo();
}

QString Document::nextRedoAction() const
{
    return _commandManager.nextRedoAction();
}

bool Document::canResetView() const
{
    return idle() && !_graphQuickItem->viewIsReset();
}

bool Document::canEnterOverviewMode() const
{
    return idle() && _graphQuickItem->canEnterOverviewMode();
}

bool Document::debugPauserEnabled() const
{
    return _graphModel != nullptr && _graphModel->graph().debugPauser.enabled();
}

bool Document::debugPaused() const
{
    return _graphModel != nullptr && _graphModel->graph().debugPauser.paused();
}

QString Document::debugResumeAction() const
{
    return _graphModel != nullptr ? _graphModel->graph().debugPauser.resumeAction() : tr("&Resume");
}

void Document::setTitle(const QString& title)
{
    if(title != _title)
    {
        _title = title;
        emit titleChanged();
    }
}

void Document::setStatus(const QString& status)
{
    if(status != _status)
    {
        _status = status;
        emit statusChanged();
    }
}

bool Document::openFile(const QUrl& fileUrl, const QString& fileType)
{
    std::unique_ptr<GraphFileParser> graphFileParser;
    std::shared_ptr<GraphModel> graphModel;

    if(!_application->parserAndModelForFile(fileUrl, fileType, graphFileParser, graphModel))
        return false;

    setTitle(fileUrl.fileName());
    emit commandInProgressChanged();
    emit idleChanged();
    emit commandVerbChanged(); // Show Loading message

    _graphModel = graphModel;

    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &Document::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &Document::onGraphChanged, Qt::DirectConnection);

    connect(&_graphModel->graph().debugPauser, &DebugPauser::enabledChanged, this, &Document::debugPauserEnabledChanged);
    connect(&_graphModel->graph().debugPauser, &DebugPauser::pausedChanged, this, &Document::debugPausedChanged);
    connect(&_graphModel->graph().debugPauser, &DebugPauser::resumeActionChanged, this, &Document::debugResumeActionChanged);

    _graphFileParserThread = std::make_unique<GraphFileParserThread>(_graphModel->graph(), std::move(graphFileParser));
    connect(_graphFileParserThread.get(), &GraphFileParserThread::progress, this, &Document::onLoadProgress);
    connect(_graphFileParserThread.get(), &GraphFileParserThread::complete, this, &Document::onLoadComplete);
    _graphFileParserThread->start();

    return true;
}

void Document::onLoadProgress(int percentage)
{
    _loadProgress = percentage;
    emit commandProgressChanged();
}

void Document::onLoadComplete(bool /*success FIXME hmm*/)
{
    _loadComplete = true;
    emit commandInProgressChanged();
    emit idleChanged();
    emit commandVerbChanged(); // Stop showing loading message

    _layoutThread = std::make_unique<LayoutThread>(_graphModel->graph(), std::make_unique<ForceDirectedLayoutFactory>(_graphModel));
    _layoutThread->addAllComponents();
    connect(_layoutThread.get(), &LayoutThread::pausedChanged, this, &Document::layoutIsPausedChanged);
    _layoutThread->start();

    _selectionManager = std::make_shared<SelectionManager>(_graphModel->graph());
    _graphQuickItem->initialise(_graphModel, _commandManager, _selectionManager);

    connect(_graphQuickItem, &GraphQuickItem::userInteractionStarted, [this] { pauseLayout(true); });
    connect(_graphQuickItem, &GraphQuickItem::userInteractionFinished, [this] { resumeLayout(true); });
    connect(_graphQuickItem, &GraphQuickItem::interactingChanged, this, &Document::idleChanged);

    connect(_graphQuickItem, &GraphQuickItem::viewIsResetChanged, this, &Document::canResetViewChanged);

    connect(_graphQuickItem, &GraphQuickItem::canEnterOverviewModeChanged, this, &Document::canEnterOverviewModeChanged);

    connect(&_commandManager, &CommandManager::busyChanged, this, &Document::idleChanged);

    connect(this, &Document::idleChanged, this, &Document::canDeleteChanged);
    connect(this, &Document::idleChanged, this, &Document::canUndoChanged);
    connect(this, &Document::idleChanged, this, &Document::canRedoChanged);
    connect(this, &Document::idleChanged, this, &Document::canEnterOverviewModeChanged);
    connect(this, &Document::idleChanged, this, &Document::canResetViewChanged);

    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, [this] { pauseLayout(true); });
    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, _graphQuickItem, &GraphQuickItem::commandWillExecuteAsynchronously);
    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, this, &Document::commandInProgressChanged);

    connect(&_commandManager, &CommandManager::commandProgressChanged, this, &Document::commandProgressChanged);
    connect(&_commandManager, &CommandManager::commandVerbChanged, this, &Document::commandVerbChanged);

    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::canUndoChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::nextUndoActionChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::canRedoChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::nextRedoActionChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, [this](const Command*, const QString& pastParticiple)
    {
        setStatus(pastParticiple);
        resumeLayout(true);
    });

    connect(&_commandManager, &CommandManager::commandCompleted, _graphQuickItem, &GraphQuickItem::commandCompleted);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::commandInProgressChanged);

    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::canDeleteChanged);

    connect(_layoutThread.get(), &LayoutThread::executed, _graphQuickItem, &GraphQuickItem::onLayoutChanged);

    /*FIXME layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(_graphWidget);

    if(_graphModel->contentWidget() != nullptr)
        layout()->addWidget(_graphModel->contentWidget());*/

    setStatus(QString(tr("Loaded %1 (%2 nodes, %3 edges, %4 components)")).arg(
                _graphModel->name()).arg(
                _graphModel->graph().numNodes()).arg(
                _graphModel->graph().numEdges()).arg(
                _graphModel->graph().numComponents()));
}


void Document::toggleLayout()
{
    if(!idle())
        return;

    if(layoutIsPaused())
        resumeLayout();
    else
        pauseLayout();
}

void Document::selectAll()
{
    if(!idle())
        return;

    if(_selectionManager)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.executeSynchronous(tr("Select All"),
            [this](Command& command)
            {
                bool nodesSelected = _selectionManager->selectAllNodes();
                command.setPastParticiple(_selectionManager->numNodesSelectedAsString());
                return nodesSelected;
            },
            [this, previousSelection](Command&) { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void Document::selectNone()
{
    if(!idle())
        return;

    if(_selectionManager)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.executeSynchronous(tr("Select None"),
            [this](Command&) { return _selectionManager->clearNodeSelection(); },
            [this, previousSelection](Command&) { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void Document::invertSelection()
{
    if(!idle())
        return;

    if(_selectionManager)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Invert Selection"), tr("Inverting Selection"),
            [this](Command& command)
            {
                _selectionManager->invertNodeSelection();
                command.setPastParticiple(_selectionManager->numNodesSelectedAsString());
                return true;
            },
            [this, previousSelection](Command&) { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void Document::undo()
{
    if(!idle())
        return;

    _commandManager.undo();
}

void Document::redo()
{
    if(!idle())
        return;

    _commandManager.redo();
}

void Document::deleteSelectedNodes()
{
    if(!idle())
        return;

    if(_selectionManager->selectedNodes().empty())
        return;

    _commandManager.execute(std::make_shared<DeleteSelectedNodesCommand>(_graphModel, _selectionManager));
}

void Document::resetView()
{
    if(!idle())
        return;

    _graphQuickItem->resetView();
}

void Document::switchToOverviewMode(bool doTransition)
{
    if(!idle())
        return;

    _graphQuickItem->switchToOverviewMode(doTransition);
}

void Document::toggleDebugPauser()
{
    _graphModel->graph().debugPauser.toggleEnabled();
}

void Document::debugResume()
{
    _graphModel->graph().debugPauser.resume();
}

void Document::onGraphWillChange(const Graph*)
{
    // Graph is about to change so suspend any active layout process
    pauseLayout(true);
}

void Document::onGraphChanged(const Graph*)
{
    resumeLayout(true);
}
