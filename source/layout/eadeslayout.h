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
    EadesLayout(const ReadOnlyGraph& graph,
                NodePositions& positions) :
        NodeLayout(graph, positions, Iterative::Yes),
        _moves(graph.numNodes())
    {}

    void executeReal(uint64_t iteration);
};

class EadesLayoutFactory : public NodeLayoutFactory
{
public:
    EadesLayoutFactory(std::shared_ptr<GraphModel> graphModel) :
        NodeLayoutFactory(graphModel)
    {}

    std::shared_ptr<NodeLayout> create(ComponentId componentId) const
    {
        auto component = this->_graphModel->graph().componentById(componentId);
        return std::make_shared<EadesLayout>(*component, this->_graphModel->nodePositions());
    }
};

#endif // EADESLAYOUT_H
