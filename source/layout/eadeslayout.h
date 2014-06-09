#ifndef EADESLAYOUT_H
#define EADESLAYOUT_H

#include "layout.h"
#include "sequencelayout.h"
#include "centreinglayout.h"
#include "../graph/graphmodel.h"

#include <QVector3D>

#include <vector>

class EadesLayout : public NodeLayout
{
    Q_OBJECT
private:
    std::vector<QVector3D> _moves;

public:
    EadesLayout(const ReadOnlyGraph& graph, NodePositions& positions) :
        NodeLayout(graph, positions, Iterative::Yes),
        _moves(graph.numNodes())
    {}

    void executeReal(uint64_t iteration);
};

class EadesLayoutFactory : public NodeLayoutFactory
{
public:
    EadesLayoutFactory(GraphModel* graphModel) :
        NodeLayoutFactory(graphModel)
    {}

    NodeLayout* create(ComponentId componentId) const
    {
        const ReadOnlyGraph* graph = this->_graphModel->graph().componentById(componentId);
        return new EadesLayout(*graph, this->_graphModel->nodePositions());
    }
};

#endif // EADESLAYOUT_H
