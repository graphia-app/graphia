#include "mainwidget.h"

#include "../parsers/gmlfileparser.h"
#include "../graph/genericgraphmodel.h"
#include "../graph/simplecomponentmanager.h"
#include "../layout/layout.h"
#include "../layout/eadeslayout.h"
#include "../layout/collision.h"
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

bool MainWidget::initFromFile(const QString &filename)
{
    QFileInfo info(filename);

    if(!info.exists() || _graphFileParserThread != nullptr)
        return false;

    std::unique_ptr<GraphFileParser> graphFileParser;

    //FIXME switch based on file content
    /*switch(fileTypeOf(filename))
    {
    case GmlFile:*/
        _graphModel.reset(new GenericGraphModel(info.fileName()));
        graphFileParser.reset(new GmlFileParser(filename));
        /*break;
    }*/

    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &MainWidget::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &MainWidget::onGraphChanged, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentAdded, this, &MainWidget::onComponentAdded, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentWillBeRemoved, this, &MainWidget::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &MainWidget::onComponentSplit, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentsWillMerge, this, &MainWidget::onComponentsWillMerge, Qt::DirectConnection);

    _graphFileParserThread.reset(new GraphFileParserThread(filename, _graphModel->graph(), std::move(graphFileParser)));
    connect(_graphFileParserThread.get(), &GraphFileParserThread::progress, this, &MainWidget::progress);
    connect(_graphFileParserThread.get(), &GraphFileParserThread::complete, this, &MainWidget::onCompletion);
    _graphFileParserThread->start();

    return true;
}

void MainWidget::onCompletion(int success)
{
    _nodeLayoutThread.reset(new NodeLayoutThread(new EadesLayoutFactory(_graphModel)));
    _nodeLayoutThread->addAllComponents(_graphModel->graph());
    _nodeLayoutThread->start();

    _selectionManager.reset(new SelectionManager(_graphModel->graph()));
    GraphWidget* graphWidget = new GraphWidget(_graphModel, _commandManager, _selectionManager);

    connect(graphWidget, &GraphWidget::userInteractionStarted,
        [this]
        {
            pauseLayout(true);
        });

    connect(graphWidget, &GraphWidget::userInteractionFinished,
        [this]
        {
            resumeLayout(true);
        });

    connect(&_commandManager, &CommandManager::commandStackChanged, this, &MainWidget::commandStackChanged);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &MainWidget::selectionChanged);

    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(graphWidget);

    if(_graphModel->contentWidget() != nullptr)
        layout()->addWidget(_graphModel->contentWidget());

    emit complete(success);
}

void MainWidget::onGraphWillChange(const Graph&)
{
    // Graph is about to change so suspend any active layout process
    pauseLayout(true);
}

void MainWidget::onGraphChanged(const Graph& graph)
{
    resumeLayout(true);

    emit graphChanged(graph);
}

void MainWidget::onComponentAdded(const Graph&, ComponentId componentId)
{
    if(_nodeLayoutThread != nullptr)
        _nodeLayoutThread->addComponent(componentId);
}

void MainWidget::onComponentWillBeRemoved(const Graph&, ComponentId componentId)
{
    if(_nodeLayoutThread != nullptr)
        _nodeLayoutThread->removeComponent(componentId);
}

void MainWidget::onComponentSplit(const Graph&, ComponentId /*splitter*/, const ElementIdSet<ComponentId>& splitters)
{
    if(_nodeLayoutThread != nullptr)
    {
        for(ComponentId componentId : splitters)
            _nodeLayoutThread->addComponent(componentId);
    }
}

void MainWidget::onComponentsWillMerge(const Graph&, const ElementIdSet<ComponentId>& mergers, ComponentId merger)
{
    if(_nodeLayoutThread != nullptr)
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
    if(_nodeLayoutThread != nullptr)
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

    if(_nodeLayoutThread != nullptr)
        _nodeLayoutThread->resume();
}

void MainWidget::selectAll()
{
    if(_selectionManager != nullptr)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Select All"),
            [this] { return _selectionManager->selectAllNodes(); },
            [this, previousSelection] { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void MainWidget::selectNone()
{
    if(_selectionManager != nullptr)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Select None"),
            [this] { return _selectionManager->clearNodeSelection(); },
            [this, previousSelection] { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void MainWidget::invertSelection()
{
    if(_selectionManager != nullptr)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Invert Selection"),
            [this] { _selectionManager->invertNodeSelection(); return true; },
            [this, previousSelection] { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

const QString MainWidget::nextUndoAction() const
{
    QString undoAction = tr("Undo");

    if(!_commandManager.undoableCommands().empty())
        undoAction.append(tr(" ") + _commandManager.undoableCommands()[0]->description());

    return undoAction;
}

const QString MainWidget::nextRedoAction() const
{
    QString redoAction = tr("Redo");

    if(!_commandManager.redoableCommands().empty())
        redoAction.append(tr(" ") + _commandManager.redoableCommands()[0]->description());

    return redoAction;
}

void MainWidget::deleteSelectedNodes()
{
    auto edges = _graphModel->graph().edgesForNodes(_selectionManager->selectedNodes());
    auto nodes = _selectionManager->selectedNodes();

    if(nodes.empty())
        return;

    _commandManager.execute(nodes.size() > 1 ? tr("Delete Nodes") : tr("Delete Node"),
        [this, nodes]
        {
            _selectionManager->clearNodeSelection();
            // Edge removal happens implicitly
            _graphModel->graph().removeNodes(nodes);
            return true;
        },
        [this, nodes, edges]
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
    if(_commandManager.canUndo())
        _commandManager.undo();
}

void MainWidget::redo()
{
    if(_commandManager.canRedo())
        _commandManager.redo();
}
