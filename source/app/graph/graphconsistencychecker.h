#ifndef GRAPHCONSISTENCYCHECKER_H
#define GRAPHCONSISTENCYCHECKER_H

#include <QObject>

class Graph;

class GraphConsistencyChecker : public QObject
{
    Q_OBJECT

public:
    explicit GraphConsistencyChecker(const Graph& graph);

    void toggle();
    void enable();
    void disable();
    bool enabled() const { return _enabled; }

private:
    const Graph* _graph;
    bool _enabled = false;

private slots:
    void onGraphChanged(const Graph* graph);
};

#endif // GRAPHCONSISTENCYCHECKER_H
