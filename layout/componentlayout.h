#ifndef COMPONENTLAYOUT_H
#define COMPONENTLAYOUT_H

#include "layout.h"

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../maths/boundingbox.h"

#include <QVector2D>
#include <QObject>

#include <algorithm>
#include <limits>
#include <atomic>

class ComponentLayout : public QObject
{
    Q_OBJECT
private:
    virtual void execute() = 0;

protected:
    const Graph* _graph;
    ComponentArray<QVector2D>* positions;
    const NodeArray<QVector3D>* nodePositions;

public:
    ComponentLayout(const Graph& graph, ComponentArray<QVector2D>& positions,
                    const NodeArray<QVector3D>& nodePositions) :
        _graph(&graph),
        positions(&positions),
        nodePositions(&nodePositions)
    {}

    const ReadOnlyGraph& graph() { return *_graph; }

    BoundingBox2D boundingBox()
    {
        BoundingBox2D _boundingBox;

        for(ComponentId componentId : *_graph->componentIds())
        {
            const ReadOnlyGraph& component = *_graph->componentById(componentId);
            float componentRadius = Layout::boundingCircleRadiusInXY(component, *nodePositions);
            QVector2D componentPosition = (*positions)[componentId];
            BoundingBox2D componentBoundingBox(
                        QVector2D(componentPosition.x() - componentRadius, componentPosition.y() - componentRadius),
                        QVector2D(componentPosition.x() + componentRadius, componentPosition.y() + componentRadius));

            _boundingBox.expandToInclude(componentBoundingBox);
        }

        return _boundingBox;
    }
};

#endif // COMPONENTLAYOUT_H
