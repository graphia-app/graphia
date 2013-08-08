#ifndef GENERICGRAPHWIDGET_H
#define GENERICGRAPHWIDGET_H

#include "graphwidget.h"

#include "graph/graph.h"
#include "graph/grapharray.h"
#include "parsers/gmlfileparser.h"

#include <QString>
#include <QVector3D>
#include <QThread>

class LoaderThread;

class GenericGraphWidget : public GraphWidget
{
    Q_OBJECT
public:
    explicit GenericGraphWidget(QWidget *parent = 0);

private:
    QString _name;

    Graph _graph;
    NodeArray<QVector3D> _layout;

    bool _initialised;
    LoaderThread *loaderThread;

signals:

public slots:
    void genericProgress(int percentage) { emit progress(percentage); }
    void genericComplete(bool success) { _initialised = success; emit complete(success); }

public:
    const Graph& graph() { return _graph; }
    const NodeArray<QVector3D>& layout() { return _layout; }

    const QString& name();

    bool initialised() { return _initialised; }
    void cancelInitialisation();

    bool initFromFile(const QString& filename);
};

class LoaderThread : public QThread
{
    Q_OBJECT
private:
    QString filename;
    Graph* graph;
    GenericGraphWidget* graphWidget;
    GraphFileParser* graphFileParser;

public:
    LoaderThread(const QString& filename, Graph& graph, GenericGraphWidget* graphWidget) :
        filename(filename),
        graph(&graph),
        graphWidget(graphWidget),
        graphFileParser(nullptr) {}

    void cancel()
    {
        if(this->isRunning())
            graphFileParser->cancel();
    }

private:
    void run() Q_DECL_OVERRIDE
    {
        graphFileParser = new GmlFileParser(filename); //FIXME choose parser based on file type
        connect(graphFileParser, &GraphFileParser::progress,
                graphWidget, &GenericGraphWidget::genericProgress);
        connect(graphFileParser, &GraphFileParser::complete,
                graphWidget, &GenericGraphWidget::genericComplete);

        graphFileParser->parse(*graph);
        delete graphFileParser;
    }
};

#endif // GENERICGRAPHWIDGET_H
