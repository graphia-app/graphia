#include "contentpanewidget.h"

#include "../parsers/gmlfileparser.h"
#include "../graph/genericgraphmodel.h"
#include "../graph/simplecomponentmanager.h"
#include "../layout/layout.h"
#include "../layout/eadeslayout.h"
#include "graphview.h"

#include <QFileInfo>
#include <QVBoxLayout>

ContentPaneWidget::ContentPaneWidget(QWidget* parent) :
    QWidget(parent),
    _graphModel(nullptr),
    graphFileParserThread(nullptr),
    layoutThread(nullptr),
    resumeLayoutPostChange(false)
{
    this->setLayout(new QVBoxLayout());
}

ContentPaneWidget::~ContentPaneWidget()
{
    delete graphFileParserThread;
    graphFileParserThread = nullptr;

    delete layoutThread;
    layoutThread = nullptr;

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
    layoutThread = new LayoutThread(new EadesLayoutFactory(_graphModel));
    _graphModel->graph().setComponentManager(new SimpleComponentManager(_graphModel->graph()));

    GraphView* graphView = new GraphView();
    graphView->setGraphModel(_graphModel);

    layout()->addWidget(graphView);

    if(_graphModel->contentWidget() != nullptr)
        layout()->addWidget(_graphModel->contentWidget());

    emit complete(success);
}

void ContentPaneWidget::onGraphWillChange(const Graph*)
{
    if(layoutThread == nullptr)
        return;

    // Graph is about to change so suspend any active layout process
    if(!layoutThread->isPaused())
    {
        layoutThread->pauseAndWait();
        resumeLayoutPostChange = true;
    }
}

void ContentPaneWidget::onGraphChanged(const Graph*)
{
    if(layoutThread == nullptr)
        return;

    if(resumeLayoutPostChange)
        layoutThread->resume();

    resumeLayoutPostChange = false;
}

void ContentPaneWidget::onComponentAdded(const Graph*, ComponentId componentId)
{
    if(layoutThread == nullptr)
        return;

    layoutThread->add(componentId);
}

void ContentPaneWidget::onComponentWillBeRemoved(const Graph*, ComponentId componentId)
{
    if(layoutThread == nullptr)
        return;

    layoutThread->remove(componentId);
}

void ContentPaneWidget::onComponentSplit(const Graph*, ComponentId /*splitter*/, QSet<ComponentId> splitters)
{
    if(layoutThread == nullptr)
        return;

    for(ComponentId componentId : splitters)
        layoutThread->add(componentId);
}

void ContentPaneWidget::onComponentsWillMerge(const Graph*, QSet<ComponentId> mergers, ComponentId merger)
{
    if(layoutThread == nullptr)
        return;

    for(ComponentId componentId : mergers)
    {
        if(componentId != merger)
            layoutThread->remove(componentId);
    }
}

void ContentPaneWidget::pauseLayout()
{
    if(layoutThread == nullptr)
        return;

    layoutThread->pause();
}

bool ContentPaneWidget::layoutIsPaused()
{
    if(layoutThread == nullptr)
        return false;

    return layoutThread->isPaused();
}

void ContentPaneWidget::resumeLayout()
{
    if(layoutThread == nullptr)
        return;

    layoutThread->resume();
}
