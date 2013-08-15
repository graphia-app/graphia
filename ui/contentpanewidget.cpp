#include "contentpanewidget.h"

#include "../parsers/gmlfileparser.h"
#include "../graph/genericgraphmodel.h"
#include "graphview.h"

#include <QFileInfo>
#include <QVBoxLayout>

ContentPaneWidget::ContentPaneWidget(QWidget* parent) :
    QWidget(parent),
    _graphModel(nullptr),
    _initialised(false),
    graphFileParserThread(nullptr)
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

    graphFileParserThread = new GraphFileParserThread(filename, _graphModel->graph(), graphFileParser);

    connect(graphFileParser, &GraphFileParser::progress,
            this, &ContentPaneWidget::onProgress);
    connect(graphFileParser, &GraphFileParser::complete,
            this, &ContentPaneWidget::onCompletion);

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

    emit complete(success);
}
