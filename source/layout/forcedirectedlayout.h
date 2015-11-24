#ifndef FORCEDIRECTEDLAYOUT_H
#define FORCEDIRECTEDLAYOUT_H

#include "layout.h"
#include "../graph/graphmodel.h"

#include <QVector3D>

#include <vector>

class ForceDirectedLayout : public Layout
{
    Q_OBJECT
private:
    std::vector<QVector3D> _prevDisplacements;
    std::vector<QVector3D> _displacements;

public:
    ForceDirectedLayout(const Graph& graph,
                        NodePositions& positions,
                        std::shared_ptr<LayoutSettings> settings) :
        Layout(graph, positions, settings, Iterative::Yes, 0.4f, 4),
        _prevDisplacements(graph.numNodes()),
        _displacements(graph.numNodes())
    {

    }

    void executeReal(bool firstIteration);
};

class ForceDirectedLayoutFactory : public LayoutFactory
{
public:
    ForceDirectedLayoutFactory(std::shared_ptr<GraphModel> graphModel) :
        LayoutFactory(graphModel)
    {
        _layoutSettings = std::make_shared<LayoutSettings>();
        _layoutSettings->registerParam("Spring Length",1,200,10);
        _layoutSettings->registerParam("Repulsive Force",0.01f,100,1);
        _layoutSettings->registerParam("Attractive Force",0.01f,100,1);
    }

    std::shared_ptr<Layout> create(ComponentId componentId, NodePositions& nodePositions) const
    {
        auto component = this->_graphModel->graph().componentById(componentId);
        return std::make_shared<ForceDirectedLayout>(*component, nodePositions, _layoutSettings);
    }
};

#endif // FORCEDIRECTEDLAYOUT_H
