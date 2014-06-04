#include "contentpanewidget.h"

#include "../parsers/gmlfileparser.h"
#include "../graph/genericgraphmodel.h"
#include "../graph/simplecomponentmanager.h"
#include "../layout/layout.h"
#include "../layout/eadeslayout.h"
#include "../layout/linearcomponentlayout.h"
#include "../layout/circlepackingcomponentlayout.h"
#include "../layout/radialcirclecomponentlayout.h"
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
    _componentLayoutThread(nullptr),
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

    delete _componentLayoutThread;
    _componentLayoutThread = nullptr;

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
        _graphModel->graph().disableComponentMangagement();
        /*break;
    }*/

    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &ContentPaneWidget::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &ContentPaneWidget::onGraphChanged, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentAdded, this, &ContentPaneWidget::onComponentAdded, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentWillBeRemoved, this, &ContentPaneWidget::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &ContentPaneWidget::onComponentSplit, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentsWillMerge, this, &ContentPaneWidget::onComponentsWillMerge, Qt::DirectConnection);

    _graphFileParserThread = new GraphFileParserThread(filename, _graphModel->graph(), graphFileParser);
    connect(graphFileParser, &GraphFileParser::progress, this, &ContentPaneWidget::progress);
    connect(graphFileParser, &GraphFileParser::complete, this, &ContentPaneWidget::onCompletion);
    _graphFileParserThread->start();

    return true;
}

void ContentPaneWidget::onCompletion(int success)
{
    _graphModel->graph().enableComponentMangagement();

    _nodeLayoutThread = new NodeLayoutThread(new EadesLayoutFactory(_graphModel));
    _nodeLayoutThread->addAllComponents(_graphModel->graph());
    _nodeLayoutThread->start();

    /*componentLayoutThread = new LayoutThread(new RadialCircleComponentLayout(_graphModel->graph(),
                                                                       _graphModel->componentPositions(),
                                                                       _graphModel->nodePositions()), true);
    componentLayoutThread->start();

    // Do the component layout whenever the node layout changes
    connect(nodeLayoutThread, &LayoutThread::executed, componentLayoutThread, &LayoutThread::execute);*/

    _selectionManager = new SelectionManager(_graphModel->graph());
    GraphView* graphView = new GraphView(_graphModel, &_commandManager, _selectionManager);
    connect(_nodeLayoutThread, &LayoutThread::executed, graphView, &GraphView::layoutChanged);

    connect(graphView, &GraphView::userInteractionStarted, [=]()
    {
        pauseLayout(true);
    });

    connect(graphView, &GraphView::userInteractionFinished, [=]()
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

void ContentPaneWidget::onComponentSplit(const Graph*, ComponentId /*splitter*/, const QSet<ComponentId>& splitters)
{
    if(_nodeLayoutThread != nullptr)
    {
        for(ComponentId componentId : splitters)
            _nodeLayoutThread->addComponent(componentId);
    }
}

void ContentPaneWidget::onComponentsWillMerge(const Graph*, const QSet<ComponentId>& mergers, ComponentId merger)
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
    if(_componentLayoutThread != nullptr)
    {
        if(autoResume && !_componentLayoutThread->paused())
            _resumePreviouslyActiveLayout = true;

        _componentLayoutThread->pauseAndWait();
    }

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
    bool componentLayoutPaused = (_componentLayoutThread == nullptr || _componentLayoutThread->paused());

    return nodeLayoutPaused && componentLayoutPaused;
}

void ContentPaneWidget::resumeLayout(bool autoResume)
{
    if(autoResume && !_resumePreviouslyActiveLayout)
        return;

    _resumePreviouslyActiveLayout = false;

    if(_nodeLayoutThread != nullptr)
        _nodeLayoutThread->resume();

    if(_componentLayoutThread != nullptr)
        _componentLayoutThread->resume();
}

void ContentPaneWidget::selectAll()
{
    if(_selectionManager != nullptr)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Select All"),
            [this]() { return _selectionManager->selectAllNodes(); },
            [this, previousSelection]() { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void ContentPaneWidget::selectNone()
{
    if(_selectionManager != nullptr)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Select None"),
            [this]() { return _selectionManager->clearNodeSelection(); },
            [this, previousSelection]() { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

void ContentPaneWidget::invertSelection()
{
    if(_selectionManager != nullptr)
    {
        auto previousSelection = _selectionManager->selectedNodes();
        _commandManager.execute(tr("Invert Selection"),
            [this]() { _selectionManager->invertNodeSelection(); return true; },
            [this, previousSelection]() { _selectionManager->setSelectedNodes(previousSelection); });
    }
}

const QString ContentPaneWidget::nextUndoAction() const
{
    QString undoAction = tr("Undo");

    if(!_commandManager.undoableCommands().isEmpty())
        undoAction.append(tr(" ") + _commandManager.undoableCommands().first()->description());

    return undoAction;
}

const QString ContentPaneWidget::nextRedoAction() const
{
    QString redoAction = tr("Redo");

    if(!_commandManager.redoableCommands().isEmpty())
        redoAction.append(tr(" ") + _commandManager.redoableCommands().first()->description());

    return redoAction;
}

void ContentPaneWidget::deleteSelectedNodes()
{
    auto edges = _graphModel->graph().edgesForNodes(_selectionManager->selectedNodes());
    auto nodes = _selectionManager->selectedNodes();

    if(nodes.isEmpty())
        return;

    _commandManager.execute(nodes.size() > 1 ? tr("Delete Nodes") : tr("Delete Node"),
        [this, nodes]()
        {
            _selectionManager->clearNodeSelection();
            // Edge removal happens implicitly
            _graphModel->graph().removeNodes(nodes);
            return true;
        },
        [this, nodes, edges]()
        {
            Graph::ScopedTransaction lock(_graphModel->graph());
            _graphModel->graph().addNodes(nodes);
            _graphModel->graph().addEdges(edges);
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
