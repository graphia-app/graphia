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
    graphFileParserThread(nullptr),
    nodeLayoutThread(nullptr),
    componentLayoutThread(nullptr),
    resumeLayoutPostChange(false)
{
    this->setLayout(new QVBoxLayout());
}

ContentPaneWidget::~ContentPaneWidget()
{
    delete graphFileParserThread;
    graphFileParserThread = nullptr;

    delete nodeLayoutThread;
    nodeLayoutThread = nullptr;

    delete componentLayoutThread;
    componentLayoutThread = nullptr;

    delete _selectionManager;
    delete _graphModel;
}

bool ContentPaneWidget::initFromFile(const QString &filename)
{
    QFileInfo info(filename);

    if(!info.exists() || graphFileParserThread != nullptr)
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

    graphFileParserThread = new GraphFileParserThread(filename, _graphModel->graph(), graphFileParser);
    connect(graphFileParser, &GraphFileParser::progress, this, &ContentPaneWidget::onProgress);
    connect(graphFileParser, &GraphFileParser::complete, this, &ContentPaneWidget::onCompletion);
    graphFileParserThread->start();

    return true;
}

void ContentPaneWidget::onCompletion(int success)
{
    _graphModel->graph().enableComponentMangagement();

    nodeLayoutThread = new NodeLayoutThread(new EadesLayoutFactory(_graphModel));
    nodeLayoutThread->addAllComponents(_graphModel->graph());
    nodeLayoutThread->start();

    /*componentLayoutThread = new LayoutThread(new RadialCircleComponentLayout(_graphModel->graph(),
                                                                       _graphModel->componentPositions(),
                                                                       _graphModel->nodePositions()), true);
    componentLayoutThread->start();

    // Do the component layout whenever the node layout changes
    connect(nodeLayoutThread, &LayoutThread::executed, componentLayoutThread, &LayoutThread::execute);*/

    _selectionManager = new SelectionManager(_graphModel->graph());
    GraphView* graphView = new GraphView();
    graphView->setGraphModel(_graphModel);
    graphView->setSelectionManager(_selectionManager);
    connect(nodeLayoutThread, &LayoutThread::executed, graphView, &GraphView::layoutChanged);

    layout()->addWidget(graphView);

    if(_graphModel->contentWidget() != nullptr)
        layout()->addWidget(_graphModel->contentWidget());

    emit complete(success);
}

void ContentPaneWidget::onGraphWillChange(const Graph*)
{
    // Graph is about to change so suspend any active layout process
    if(componentLayoutThread != nullptr && !componentLayoutThread->paused())
    {
        componentLayoutThread->pauseAndWait();
        resumeLayoutPostChange = true;
    }

    if(nodeLayoutThread != nullptr && !nodeLayoutThread->paused())
    {
        nodeLayoutThread->pauseAndWait();
        resumeLayoutPostChange = true;
    }
}

void ContentPaneWidget::onGraphChanged(const Graph* graph)
{
    if(resumeLayoutPostChange)
    {
        if(nodeLayoutThread != nullptr)
            nodeLayoutThread->resume();

        if(componentLayoutThread != nullptr)
            componentLayoutThread->resume();
    }

    resumeLayoutPostChange = false;

    emit graphChanged(graph);
}

void ContentPaneWidget::onComponentAdded(const Graph*, ComponentId componentId)
{
    if(nodeLayoutThread != nullptr)
        nodeLayoutThread->addComponent(componentId);
}

void ContentPaneWidget::onComponentWillBeRemoved(const Graph*, ComponentId componentId)
{
    if(nodeLayoutThread != nullptr)
        nodeLayoutThread->removeComponent(componentId);
}

void ContentPaneWidget::onComponentSplit(const Graph*, ComponentId /*splitter*/, QSet<ComponentId> splitters)
{
    if(nodeLayoutThread != nullptr)
    {
        for(ComponentId componentId : splitters)
            nodeLayoutThread->addComponent(componentId);
    }
}

void ContentPaneWidget::onComponentsWillMerge(const Graph*, QSet<ComponentId> mergers, ComponentId merger)
{
    if(nodeLayoutThread != nullptr)
    {
        for(ComponentId componentId : mergers)
        {
            if(componentId != merger)
                nodeLayoutThread->removeComponent(componentId);
        }
    }
}

void ContentPaneWidget::pauseLayout()
{
    if(nodeLayoutThread != nullptr)
        nodeLayoutThread->pause();

    if(componentLayoutThread != nullptr)
        componentLayoutThread->pause();
}

bool ContentPaneWidget::layoutIsPaused()
{
    // Not typos: a non-existant thread counts as paused
    bool nodeLayoutPaused = (nodeLayoutThread == nullptr || nodeLayoutThread->paused());
    bool componentLayoutPaused = (componentLayoutThread == nullptr || componentLayoutThread->paused());

    return nodeLayoutPaused && componentLayoutPaused;
}

void ContentPaneWidget::resumeLayout()
{
    if(nodeLayoutThread != nullptr)
        nodeLayoutThread->resume();

    if(componentLayoutThread != nullptr)
        componentLayoutThread->resume();
}
