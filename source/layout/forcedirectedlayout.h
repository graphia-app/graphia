#ifndef EADESLAYOUT_H
#define EADESLAYOUT_H

#include "layout.h"
#include "sequencelayout.h"
#include "centreinglayout.h"
#include "../graph/graphmodel.h"
#include "../utils/threadpool.h"

#include <QVector3D>

#include <vector>

class ForceDirectedLayout : public Layout
{
    Q_OBJECT
private:
    std::vector<QVector3D> _prevDisplacements;
    std::vector<QVector3D> _displacements;
    static ThreadPool _threadPool;

public:
    ForceDirectedLayout(const ImmutableGraph& graph,
                        NodePositions& positions) :
        Layout(graph, positions, Iterative::Yes),
        _prevDisplacements(graph.numNodes()),
        _displacements(graph.numNodes())
    {}

    void executeReal(uint64_t iteration);
};

class ForceDirectedLayoutFactory : public LayoutFactory
{
public:
    ForceDirectedLayoutFactory(std::shared_ptr<GraphModel> graphModel) :
        LayoutFactory(graphModel)
    {}

    std::shared_ptr<Layout> create(ComponentId componentId) const
    {
        auto component = this->_graphModel->graph().componentById(componentId);
        return std::make_shared<ForceDirectedLayout>(*component, this->_graphModel->nodePositions());
    }
};

#endif // EADESLAYOUT_H
