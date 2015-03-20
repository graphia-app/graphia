#include "mainwidget.h"

#include "../loading/fileidentifier.h"
#include "../loading/gmlfiletype.h"
#include "../loading/gmlfileparser.h"
#include "../graph/genericgraphmodel.h"
#include "../graph/simplecomponentmanager.h"
#include "../layout/layout.h"
#include "../layout/forcedirectedlayout.h"
#include "../layout/collision.h"
#include "../utils/make_unique.h"
#include "../commands/deleteselectednodescommand.h"
#include "graphwidget.h"
#include "selectionmanager.h"

#include <QFileInfo>
#include <QVBoxLayout>

MainWidget::MainWidget(QWidget* parent) :
    QWidget(parent),
    _loadComplete(false),
    _autoResume(0)
{
    this->setLayout(new QVBoxLayout());
}

MainWidget::~MainWidget()
{
    // Defined so we can use smart pointers to incomplete types in the header
}

bool MainWidget::initFromFile(const QString& filename)
{
    QFileInfo info(filename);

    if(!info.exists() || _graphFileParserThread)
        return false;

    std::unique_ptr<GraphFileParser> graphFileParser;

    FileIdentifier fi;
    fi.registerFileType(std::make_shared<GmlFileType>());

    auto fileTypeName = fi.identify(filename);

    if(fileTypeName.compare("GML") == 0)
    {
        _graphModel = std::make_shared<GenericGraphModel>(info.fileName());
        graphFileParser = std::make_unique<GmlFileParser>(filename);
    }

    //FIXME what we should really be doing:
    // query which plugins can load fileTypeName
    // allow the user to choose which plugin to use if there is more than 1
    // _graphModel = plugin->graphModelForFilename(filename);
    // graphFileParser = plugin->parserForFilename(filename);

    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &MainWidget::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &MainWidget::onGraphChanged, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &MainWidget::graphChanged, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentAdded, this, &MainWidget::onComponentAdded, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentWillBeRemoved, this, &MainWidget::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &MainWidget::onComponentSplit, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentsWillMerge, this, &MainWidget::onComponentsWillMerge, Qt::DirectConnection);

    _graphFileParserThread = std::make_unique<GraphFileParserThread>(filename, _graphModel->graph(), std::move(graphFileParser));
    connect(_graphFileParserThread.get(), &GraphFileParserThread::progress, this, &MainWidget::progress);
    connect(_graphFileParserThread.get(), &GraphFileParserThread::complete, this, &MainWidget::onLoadCompletion);
    _graphFileParserThread->start();

    return true;
}

void MainWidget::onLoadCompletion(bool success)
{
    _loadComplete = true;

    _nodeLayoutThread = std::make_unique<NodeLayoutThread>(std::make_unique<ForceDirectedLayoutFactory>(_graphModel));
    _nodeLayoutThread->addAllComponents(_graphModel->graph());
    _nodeLayoutThread->start();

    _selectionManager = std::make_shared<SelectionManager>(_graphModel->graph());
    _graphWidget = new GraphWidget(_graphModel, _commandManager, _selectionManager);
    _graphWidget->initialise();

    connect(_graphWidget, &GraphWidget::userInteractionStarted, [this] { pauseLayout(true); });
    connect(_graphWidget, &GraphWidget::userInteractionStarted, this, &MainWidget::userInteractionStarted);
    connect(_graphWidget, &GraphWidget::userInteractionFinished, [this] { resumeLayout(true); });
    connect(_graphWidget, &GraphWidget::userInteractionFinished, this, &MainWidget::userInteractionFinished);

    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, [this] { pauseLayout(true); });
    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, _graphWidget, &GraphWidget::onCommandWillExecuteAsynchronously);
    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, this, &MainWidget::commandWillExecuteAsynchronously);

    connect(&_commandManager, &CommandManager::commandProgress, this, &MainWidget::commandProgress); 

    connect(&_commandManager, &CommandManager::commandCompleted, [this] { resumeLayout(true); });
    connect(&_commandManager, &CommandManager::commandCompleted, _graphWidget, &GraphWidget::onCommandCompleted);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &MainWidget::commandCompleted);

    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &MainWidget::selectionChanged);

    connect(_nodeLayoutThread.get(), &NodeLayoutThread::executed, _graphWidget, &GraphWidget::layoutChanged);

    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(_graphWidget);

    if(_graphModel->contentWidget() != nullptr)
        layout()->addWidget(_graphModel->contentWidget());

    emit complete(success);
}

void MainWidget::onGraphWillChange(const Graph*)
{
    // Graph is about to change so suspend any active layout process
    pauseLayout(true);
}

void MainWidget::onGraphChanged(const Graph*)
{
    resumeLayout(true);
}

void MainWidget::onComponentAdded(const Graph*, ComponentId componentId, bool)
{
    if(_nodeLayoutThread)
        _nodeLayoutThread->addComponent(componentId);
}

void MainWidget::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool)
{
    if(_nodeLayoutThread)
        _nodeLayoutThread->removeComponent(componentId);
}

void MainWidget::onComponentSplit(const Graph*, const ComponentSplitSet& componentSplitSet)
{
    if(_nodeLayoutThread)
    {
        for(ComponentId componentId : componentSplitSet.splitters())
            _nodeLayoutThread->addComponent(componentId);
    }
}

void MainWidget::onComponentsWillMerge(const Graph*, const ComponentMergeSet& componentMergeSet)
{
    if(_nodeLayoutThread)
    {
        for(ComponentId componentId : componentMergeSet.mergers())
        {
            if(componentId != componentMergeSet.newComponentId())
                _nodeLayoutThread->removeComponent(componentId);
        }
    }
}

void MainWidget::pauseLayout(bool autoResume)
{
    std::unique_lock<std::mutex> lock(_autoResumeMutex);

    if(_nodeLayoutThread)
    {
        if(!autoResume)
            _nodeLayoutThread->pauseAndWait();
        else
        {
            if(_autoResume > 0 || !_nodeLayoutThread->paused())
                _autoResume++;

            if(_autoResume == 1)
                _nodeLayoutThread->pauseAndWait();
        }
    }
}

bool MainWidget::layoutIsPaused()
{
    std::unique_lock<std::mutex> lock(_autoResumeMutex);

    // Not a typo: a non-existant thread counts as paused
    bool nodeLayoutPaused = (_nodeLayoutThread == nullptr || _nodeLayoutThread->paused()) &&
            _autoResume == 0;

    return nodeLayoutPaused;
}

void MainWidget::resumeLayout(bool autoResume)
{
    std::unique_lock<std::mutex> lock(_autoResumeMutex);

    if(_nodeLayoutThread)
    {
        if(!autoResume)
            _nodeLayoutThread->resume();
        else
        {
            if(_autoResume == 1)
                _nodeLayoutThread->resume();

            if(_autoResume > 0)
                _autoResume--;
        }
    }
}

void MainWidget::selectAll()
{
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

void MainWidget::selectNone()
{
    if(_selectionManager)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.executeSynchronous(tr("Select None"),
            [this](Command&) { return _selectionManager->clearNodeSelection(); },
            [this, previousSelection](Command&) { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void MainWidget::invertSelection()
{
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

const QString MainWidget::nextUndoAction() const
{
    QString undoAction = _commandManager.nextUndoAction();

    if(undoAction.isEmpty())
        undoAction = tr("Undo");

    return undoAction;
}

const QString MainWidget::nextRedoAction() const
{
    QString redoAction = _commandManager.nextRedoAction();

    if(redoAction.isEmpty())
        redoAction = tr("Redo");

    return redoAction;
}

bool MainWidget::busy() const
{
    return _commandManager.busy() || !_loadComplete;
}

bool MainWidget::interacting() const
{
    return _graphWidget->interacting();
}

void MainWidget::resetView()
{
    _graphWidget->resetView();
}

bool MainWidget::viewIsReset() const
{
    return _graphWidget->viewIsReset();
}

void MainWidget::switchToOverviewMode()
{
    _graphWidget->switchToOverviewMode();
}

GraphWidget::Mode MainWidget::mode() const
{
    return _graphWidget->mode();
}

void MainWidget::deleteSelectedNodes()
{
    if(_selectionManager->selectedNodes().empty())
        return;

    _commandManager.execute(std::make_shared<DeleteSelectedNodesCommand>(_graphModel, _selectionManager));
}

void MainWidget::undo()
{
    _commandManager.undo();
}

void MainWidget::redo()
{
    _commandManager.redo();
}
