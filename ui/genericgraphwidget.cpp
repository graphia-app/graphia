#include "genericgraphwidget.h"

#include <QFileInfo>

GenericGraphWidget::GenericGraphWidget(QWidget *parent) :
    GraphWidget(parent),
    _layout(_graph),
    _initialised(false)
{
}

const QString& GenericGraphWidget::name()
{
    return _name;
}

bool GenericGraphWidget::initFromFile(const QString &filename)
{
    QFileInfo info(filename);

    if(!info.exists())
        return false;

    _name = info.fileName();

    LoaderThread *loaderThread = new LoaderThread(filename, _graph, this);
    connect(loaderThread, &LoaderThread::finished, loaderThread, &QObject::deleteLater);
    loaderThread->start();

    return true;
}
