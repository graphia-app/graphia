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
                        const LayoutSettings* settings) :
        Layout(graph, positions, settings, Iterative::Yes, 0.4f, 4),
        _prevDisplacements(graph.numNodes()),
        _displacements(graph.numNodes())
    {}

    void executeReal(bool firstIteration);
};

class ForceDirectedLayoutFactory : public LayoutFactory
{
public:
    explicit ForceDirectedLayoutFactory(std::shared_ptr<GraphModel> graphModel) :
        LayoutFactory(graphModel)
    {
        _layoutSettings.registerSetting("RepulsiveForce",  QObject::tr("Repulsive Force"),  1.0f, 100.0f, 1.0f);
        _layoutSettings.registerSetting("AttractiveForce", QObject::tr("Attractive Force"), 1.0f, 100.0f, 1.0f);
    }

    std::shared_ptr<Layout> create(ComponentId componentId, NodePositions& nodePositions) const
    {
        auto component = this->_graphModel->graph().componentById(componentId);
        return std::make_shared<ForceDirectedLayout>(*component, nodePositions, &_layoutSettings);
    }
};

#endif // FORCEDIRECTEDLAYOUT_H
