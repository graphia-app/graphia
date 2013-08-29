#ifndef EADESLAYOUT_H
#define EADESLAYOUT_H

#include "layout.h"
#include "../graph/graphmodel.h"

#include <QVector3D>

class EadesLayout : public Layout
{
    Q_OBJECT
private:
    bool firstIteration;
    QVector<QVector3D> moves;

public:
    EadesLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        Layout(graph, positions, Layout::Unbounded),
        firstIteration(true),
        moves(graph.numNodes())
    {}

    void executeReal();
};

class EadesLayoutFactory : public LayoutFactory
{
public:
    EadesLayoutFactory(GraphModel* _graphModel) :
        LayoutFactory(_graphModel)
    {}

    Layout* create(ComponentId componentId) const
    {
        const ReadOnlyGraph* graph = this->_graphModel->graph().componentById(componentId);
        return new EadesLayout(*graph, this->_graphModel->layout());
    }
};

#endif // EADESLAYOUT_H
