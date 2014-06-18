#include "contentpanewidget.h"

#include "../parsers/gmlfileparser.h"
#include "../graph/genericgraphmodel.h"
#include "../graph/simplecomponentmanager.h"
#include "../layout/layout.h"
#include "../layout/eadeslayout.h"
#include "../layout/collision.h"
#include "graphview.h"
#include "selectionmanager.h"

#include <QFileInfo>
#include <QVBoxLayout>

ContentPaneWidget::ContentPaneWidget(QWidget* parent) :
    QWidget(parent),
    _graphModel(nullptr),
    _selectionManager(nullptr),
    _graphFileParserThread(nullptr),
    _nodeLayoutThread(nullptr),
    _resumePreviouslyActiveLayout(false)
{
    this->setLayout(new QVBoxLayout());
}

ContentPaneWidget::~ContentPaneWidget()
{
    delete _graphFileParserThread;
    _graphFileParserThread = nullptr;

    delete _nodeLayoutThread;
    _nodeLayoutThread = nullptr;

    delete _selectionManager;
    delete _graphModel;
}

bool ContentPaneWidget::initFromFile(const QString &filename)
{
    QFileInfo info(filename);

    if(!info.exists() || _graphFileParserThread != nullptr)
        return false;

    GraphFileParser* graphFileParser = nullptr;

    //FIXME switch based on file content
    /*switch(fileTypeOf(filename))
    {
    case GmlFile:*/
        _graphModel = new GenericGraphModel(info.fileName());
        graphFileParser = new GmlFileParser(filename);
        /*break;
    }*/

    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &ContentPaneWidget::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &ContentPaneWidget::onGraphChanged, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentAdded, this, &ContentPaneWidget::onComponentAdded, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentWillBeRemoved, this, &ContentPaneWidget::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &ContentPaneWidget::onComponentSplit, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentsWillMerge, this, &ContentPaneWidget::onComponentsWillMerge, Qt::DirectConnection);

    _graphFileParserThread = new GraphFileParserThread(filename, _graphModel->graph(), graphFileParser);
    connect(_graphFileParserThread, &GraphFileParserThread::progress, this, &ContentPaneWidget::progress);
    connect(_graphFileParserThread, &GraphFileParserThread::complete, this, &ContentPaneWidget::onCompletion);
    _graphFileParserThread->start();

    return true;
}

void ContentPaneWidget::onCompletion(int success)
{
    _nodeLayoutThread = new NodeLayoutThread(new EadesLayoutFactory(_graphModel));
    _nodeLayoutThread->addAllComponents(_graphModel->graph());
    _nodeLayoutThread->start();

    _selectionManager = new SelectionManager(_graphModel->graph());
    GraphView* graphView = new GraphView(_graphModel, &_commandManager, _selectionManager);

    connect(graphView, &GraphView::userInteractionStarted,
        [this]
        {
            pauseLayout(true);
        });

    connect(graphView, &GraphView::userInteractionFinished,
        [this]
        {
            resumeLayout(true);
        });

    connect(&_commandManager, &CommandManager::commandStackChanged, this, &ContentPaneWidget::commandStackChanged);
    connect(_selectionManager, &SelectionManager::selectionChanged, this, &ContentPaneWidget::selectionChanged);

    layout()->addWidget(graphView);

    if(_graphModel->contentWidget() != nullptr)
        layout()->addWidget(_graphModel->contentWidget());

    emit complete(success);
}

void ContentPaneWidget::onGraphWillChange(const Graph*)
{
    // Graph is about to change so suspend any active layout process
    pauseLayout(true);
}

void ContentPaneWidget::onGraphChanged(const Graph* graph)
{
    resumeLayout(true);

    emit graphChanged(graph);
}

void ContentPaneWidget::onComponentAdded(const Graph*, ComponentId componentId)
{
    if(_nodeLayoutThread != nullptr)
        _nodeLayoutThread->addComponent(componentId);
}

void ContentPaneWidget::onComponentWillBeRemoved(const Graph*, ComponentId componentId)
{
    if(_nodeLayoutThread != nullptr)
        _nodeLayoutThread->removeComponent(componentId);
}

void ContentPaneWidget::onComponentSplit(const Graph*, ComponentId /*splitter*/, const ElementIdSet<ComponentId>& splitters)
{
    if(_nodeLayoutThread != nullptr)
    {
        for(ComponentId componentId : splitters)
            _nodeLayoutThread->addComponent(componentId);
    }
}

void ContentPaneWidget::onComponentsWillMerge(const Graph*, const ElementIdSet<ComponentId>& mergers, ComponentId merger)
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

void ContentPaneWidget::pauseLayout(bool autoResume)
{
    if(_nodeLayoutThread != nullptr)
    {
        if(autoResume && !_nodeLayoutThread->paused())
            _resumePreviouslyActiveLayout = true;

        _nodeLayoutThread->pauseAndWait();
    }
}

bool ContentPaneWidget::layoutIsPaused()
{
    // Not typos: a non-existant thread counts as paused
    bool nodeLayoutPaused = (_nodeLayoutThread == nullptr || _nodeLayoutThread->paused());

    return nodeLayoutPaused;
}

void ContentPaneWidget::resumeLayout(bool autoResume)
{
    if(autoResume && !_resumePreviouslyActiveLayout)
        return;

    _resumePreviouslyActiveLayout = false;

    if(_nodeLayoutThread != nullptr)
        _nodeLayoutThread->resume();
}

void ContentPaneWidget::selectAll()
{
    if(_selectionManager != nullptr)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Select All"),
            [this] { return _selectionManager->selectAllNodes(); },
            [this, previousSelection] { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void ContentPaneWidget::selectNone()
{
    if(_selectionManager != nullptr)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Select None"),
            [this] { return _selectionManager->clearNodeSelection(); },
            [this, previousSelection] { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void ContentPaneWidget::invertSelection()
{
    if(_selectionManager != nullptr)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Invert Selection"),
            [this] { _selectionManager->invertNodeSelection(); return true; },
            [this, previousSelection] { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

const QString ContentPaneWidget::nextUndoAction() const
{
    QString undoAction = tr("Undo");

    if(!_commandManager.undoableCommands().empty())
        undoAction.append(tr(" ") + _commandManager.undoableCommands()[0]->description());

    return undoAction;
}

const QString ContentPaneWidget::nextRedoAction() const
{
    QString redoAction = tr("Redo");

    if(!_commandManager.redoableCommands().empty())
        redoAction.append(tr(" ") + _commandManager.redoableCommands()[0]->description());

    return redoAction;
}

void ContentPaneWidget::deleteSelectedNodes()
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
                [nodes, edges](Graph& graph)
                {
                    graph.addNodes(nodes);
                    graph.addEdges(edges);
                });

            _selectionManager->selectNodes(nodes);
        });
}

void ContentPaneWidget::undo()
{
    if(_commandManager.canUndo())
        _commandManager.undo();
}

void ContentPaneWidget::redo()
{
    if(_commandManager.canRedo())
        _commandManager.redo();
}
