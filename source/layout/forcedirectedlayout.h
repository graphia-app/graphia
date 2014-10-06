#ifndef EADESLAYOUT_H
#define EADESLAYOUT_H

#include "layout.h"
#include "sequencelayout.h"
#include "centreinglayout.h"
#include "../graph/graphmodel.h"
#include "../utils/threadpool.h"

#include <QVector3D>

#include <vector>

class ForceDirectedLayout : public NodeLayout
{
    Q_OBJECT
private:
    std::vector<QVector3D> _prevDisplacements;
    std::vector<QVector3D> _displacements;
    static ThreadPool _threadPool;

public:
    ForceDirectedLayout(const ReadOnlyGraph& graph,
                        NodePositions& positions) :
        NodeLayout(graph, positions, Iterative::Yes),
        _prevDisplacements(graph.numNodes()),
        _displacements(graph.numNodes())
    {}

    void executeReal(uint64_t iteration);
};

class ForceDirectedLayoutFactory : public NodeLayoutFactory
{
public:
    ForceDirectedLayoutFactory(std::shared_ptr<GraphModel> graphModel) :
        NodeLayoutFactory(graphModel)
    {}

    std::shared_ptr<NodeLayout> create(ComponentId componentId) const
    {
        auto component = this->_graphModel->graph().componentById(componentId);
        return std::make_shared<ForceDirectedLayout>(*component, this->_graphModel->nodePositions());
    }
};

#endif // EADESLAYOUT_H
