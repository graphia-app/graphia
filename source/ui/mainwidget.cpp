#include "mainwidget.h"

#include "../parsers/gmlfileparser.h"
#include "../graph/genericgraphmodel.h"
#include "../graph/simplecomponentmanager.h"
#include "../layout/layout.h"
#include "../layout/eadeslayout.h"
#include "../layout/collision.h"
#include "../utils/make_unique.h"
#include "graphwidget.h"
#include "selectionmanager.h"

#include <QFileInfo>
#include <QVBoxLayout>

MainWidget::MainWidget(QWidget* parent) :
    QWidget(parent),
    _resumePreviouslyActiveLayout(false)
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

    //FIXME switch based on file content
    /*switch(fileTypeOf(filename))
    {
    case GmlFile:*/
        _graphModel = std::make_shared<GenericGraphModel>(info.fileName());
        graphFileParser = std::make_unique<GmlFileParser>(filename);
        /*break;
    }*/

    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &MainWidget::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &MainWidget::onGraphChanged, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &MainWidget::graphChanged, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentAdded, this, &MainWidget::onComponentAdded, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentWillBeRemoved, this, &MainWidget::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &MainWidget::onComponentSplit, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentsWillMerge, this, &MainWidget::onComponentsWillMerge, Qt::DirectConnection);

    _graphFileParserThread = std::make_unique<GraphFileParserThread>(filename, _graphModel->graph(), std::move(graphFileParser));
    connect(_graphFileParserThread.get(), &GraphFileParserThread::progress, this, &MainWidget::progress);
    connect(_graphFileParserThread.get(), &GraphFileParserThread::complete, this, &MainWidget::onCompletion);
    _graphFileParserThread->start();

    return true;
}

void MainWidget::onCompletion(int success)
{
    _nodeLayoutThread = std::make_unique<NodeLayoutThread>(std::make_unique<EadesLayoutFactory>(_graphModel));
    _nodeLayoutThread->addAllComponents(_graphModel->graph());
    _nodeLayoutThread->start();

    _selectionManager = std::make_shared<SelectionManager>(_graphModel->graph());
    _graphWidget = new GraphWidget(_graphModel, _commandManager, _selectionManager);

    connect(_graphWidget, &GraphWidget::userInteractionStarted,  [this] { pauseLayout(true); });
    connect(_graphWidget, &GraphWidget::userInteractionFinished, [this] { resumeLayout(true); });

    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, [this] { pauseLayout(true); });
    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, _graphWidget, &GraphWidget::onCommandWillExecuteAsynchronously);
    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, this, &MainWidget::commandWillExecuteAsynchronously);

    connect(&_commandManager, &CommandManager::commandProgress, this, &MainWidget::commandProgress); 

    connect(&_commandManager, &CommandManager::commandCompleted, [this] { resumeLayout(true); });
    connect(&_commandManager, &CommandManager::commandCompleted, _graphWidget, &GraphWidget::onCommandCompleted);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &MainWidget::commandCompleted);

    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &MainWidget::selectionChanged);

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

void MainWidget::onComponentAdded(const Graph*, ComponentId componentId)
{
    if(_nodeLayoutThread)
        _nodeLayoutThread->addComponent(componentId);
}

void MainWidget::onComponentWillBeRemoved(const Graph*, ComponentId componentId)
{
    if(_nodeLayoutThread)
        _nodeLayoutThread->removeComponent(componentId);
}

void MainWidget::onComponentSplit(const Graph*, ComponentId /*splitter*/, const ElementIdSet<ComponentId>& splitters)
{
    if(_nodeLayoutThread)
    {
        for(ComponentId componentId : splitters)
            _nodeLayoutThread->addComponent(componentId);
    }
}

void MainWidget::onComponentsWillMerge(const Graph*, const ElementIdSet<ComponentId>& mergers, ComponentId merger)
{
    if(_nodeLayoutThread)
    {
        for(ComponentId componentId : mergers)
        {
            if(componentId != merger)
                _nodeLayoutThread->removeComponent(componentId);
        }
    }
}

void MainWidget::pauseLayout(bool autoResume)
{
    if(_nodeLayoutThread)
    {
        if(autoResume && !_nodeLayoutThread->paused())
            _resumePreviouslyActiveLayout = true;

        _nodeLayoutThread->pauseAndWait();
    }
}

bool MainWidget::layoutIsPaused()
{
    // Not typos: a non-existant thread counts as paused
    bool nodeLayoutPaused = (_nodeLayoutThread == nullptr || _nodeLayoutThread->paused());

    return nodeLayoutPaused;
}

void MainWidget::resumeLayout(bool autoResume)
{
    if(autoResume && !_resumePreviouslyActiveLayout)
        return;

    _resumePreviouslyActiveLayout = false;

    if(_nodeLayoutThread)
        _nodeLayoutThread->resume();
}

void MainWidget::selectAll()
{
    if(_selectionManager)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Select All"),
            [this](ProgressFn) { return _selectionManager->selectAllNodes(); },
            [this, previousSelection](ProgressFn) { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void MainWidget::selectNone()
{
    if(_selectionManager)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Select None"),
            [this](ProgressFn) { return _selectionManager->clearNodeSelection(); },
            [this, previousSelection](ProgressFn) { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void MainWidget::invertSelection()
{
    if(_selectionManager)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Invert Selection"),
            [this](ProgressFn) { _selectionManager->invertNodeSelection(); return true; },
            [this, previousSelection](ProgressFn) { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

const QString MainWidget::nextUndoAction() const
{
    QString undoAction = tr("Undo");

    if(!_commandManager.undoableCommandDescriptions().empty())
        undoAction.append(tr(" ") + _commandManager.undoableCommandDescriptions().front());

    return undoAction;
}

const QString MainWidget::nextRedoAction() const
{
    QString redoAction = tr("Redo");

    if(!_commandManager.redoableCommandDescriptions().empty())
        redoAction.append(tr(" ") + _commandManager.redoableCommandDescriptions().front());

    return redoAction;
}

void MainWidget::deleteSelectedNodes()
{
    auto edges = _graphModel->graph().edgesForNodes(_selectionManager->selectedNodes());
    auto nodes = _selectionManager->selectedNodes();

    if(nodes.empty())
        return;

    _commandManager.execute(nodes.size() > 1 ? tr("Delete Nodes") : tr("Delete Node"),
        [this, nodes](ProgressFn)
        {
            _selectionManager->clearNodeSelection(false);
            _graphModel->graph().removeNodes(nodes);
            return true;
        },
        [this, nodes, edges](ProgressFn)
        {
            _graphModel->graph().performTransaction(
                [&nodes, &edges](Graph& graph)
                {
                    graph.addNodes(nodes);
                    graph.addEdges(edges);
                });

            _selectionManager->selectNodes(nodes);
        });
}

void MainWidget::undo()
{
    _commandManager.undo();
}

void MainWidget::redo()
{
    _commandManager.redo();
}
