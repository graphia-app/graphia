#include "genericgraphwidget.h"

#include <QFileInfo>

GenericGraphWidget::GenericGraphWidget(QWidget *parent) :
    GraphWidget(parent),
    _layout(_graph),
    _initialised(false),
    loaderThread(nullptr)
{
}

const QString& GenericGraphWidget::name()
{
    return _name;
}

void GenericGraphWidget::cancelInitialisation()
{
    if(!initialised())
    {
        loaderThread->cancel();
        loaderThread->wait();
    }
}

bool GenericGraphWidget::initFromFile(const QString &filename)
{
    QFileInfo info(filename);

    if(!info.exists() || loaderThread != nullptr)
        return false;

    _name = info.fileName();

    loaderThread = new LoaderThread(filename, _graph, this);
    connect(loaderThread, &LoaderThread::finished, loaderThread, &QObject::deleteLater);
    loaderThread->start();

    return true;
}
