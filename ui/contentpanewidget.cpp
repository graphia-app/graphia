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
    loaderThread(nullptr)
{
    this->setLayout(new QVBoxLayout());
}

ContentPaneWidget::~ContentPaneWidget()
{
    if(loaderThread != nullptr)
    {
        if(!_initialised)
        {
            loaderThread->cancel();
            loaderThread->wait();
        }

        delete loaderThread;
        loaderThread = nullptr;
    }

    delete _graphModel;
}

bool ContentPaneWidget::initFromFile(const QString &filename)
{
    QFileInfo info(filename);

    if(!info.exists() || loaderThread != nullptr)
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

    loaderThread = new LoaderThread(filename, _graphModel->graph(), graphFileParser, this);
    loaderThread->start();

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
