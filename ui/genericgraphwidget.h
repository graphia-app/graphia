#ifndef GENERICGRAPHWIDGET_H
#define GENERICGRAPHWIDGET_H

#include "graphwidget.h"

#include "graph/graph.h"
#include "graph/grapharray.h"
#include "parsers/gmlfileparser.h"

#include <QString>
#include <QVector3D>
#include <QThread>

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

signals:

public slots:
    void genericProgress(int percentage) { emit progress(percentage); }
    void genericComplete(bool success) { _initialised = success; emit complete(success); }

public:
    const Graph& graph() { return _graph; }
    const NodeArray<QVector3D>& layout() { return _layout; }

    const QString& name();

    bool initialised() { return _initialised; }

    bool initFromFile(const QString& filename);
};

class LoaderThread : public QThread
{
    Q_OBJECT
private:
    QString filename;
    Graph* graph;
    GenericGraphWidget* graphWidget;

public:
    LoaderThread(const QString& filename, Graph& graph, GenericGraphWidget* graphWidget) :
        filename(filename), graph(&graph), graphWidget(graphWidget) {}

private:
    void run() Q_DECL_OVERRIDE
    {
        GmlFileParser gmlFileParser(filename);
        connect(&gmlFileParser, &GmlFileParser::progress,
                graphWidget, &GenericGraphWidget::genericProgress);
        connect(&gmlFileParser, &GmlFileParser::complete,
                graphWidget, &GenericGraphWidget::genericComplete);

        gmlFileParser.parse(*graph);
    }
};

#endif // GENERICGRAPHWIDGET_H
