#include "contentpanewidget.h"

#include "../parsers/gmlfileparser.h"
#include "../graph/genericgraphmodel.h"
#include "../layout/layoutalgorithm.h"
#include "../layout/eadeslayout.h"
#include "graphview.h"

#include <QFileInfo>
#include <QVBoxLayout>

ContentPaneWidget::ContentPaneWidget(QWidget* parent) :
    QWidget(parent),
    _graphModel(nullptr),
    _initialised(false),
    graphFileParserThread(nullptr),
    layoutThread(nullptr),
    resumeLayoutPostChange(false)
{
    this->setLayout(new QVBoxLayout());
}

ContentPaneWidget::~ContentPaneWidget()
{
    if(graphFileParserThread != nullptr)
    {
        if(!_initialised)
        {
            graphFileParserThread->cancel();
            graphFileParserThread->wait();
        }

        delete graphFileParserThread;
        graphFileParserThread = nullptr;
    }

    if(layoutThread != nullptr)
    {
        layoutThread->stop();
        layoutThread->wait();
        delete layoutThread;
        layoutThread = nullptr;
    }

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

    graphFileParserThread = new GraphFileParserThread(filename, _graphModel->graph(), graphFileParser);
    connect(graphFileParser, &GraphFileParser::progress, this, &ContentPaneWidget::onProgress);
    connect(graphFileParser, &GraphFileParser::complete, this, &ContentPaneWidget::onCompletion);
    graphFileParserThread->start();

    return true;
}

void ContentPaneWidget::onCompletion(int success)
{
    _initialised = true;

    GraphView* graphView = new GraphView();
    graphView->setGraphModel(_graphModel);

    layout()->addWidget(graphView);

    if(_graphModel->contentWidget() != nullptr)
        layout()->addWidget(_graphModel->contentWidget());

    layoutThread = new LayoutThread(new EadesLayout(_graphModel->layout()));
    layoutThread->start();

    emit complete(success);
}

void ContentPaneWidget::onGraphWillChange(Graph &)
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

void ContentPaneWidget::onGraphChanged(Graph &)
{
    if(layoutThread == nullptr)
        return;

    if(resumeLayoutPostChange)
        layoutThread->resume();

    resumeLayoutPostChange = false;
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
