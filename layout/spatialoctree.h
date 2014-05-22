#ifndef SPATIALOCTREE_H
#define SPATIALOCTREE_H

#include "../graph/graph.h"
#include "../maths/boundingbox.h"
#include "../layout/layout.h"

#include <QVector3D>
#include <QVector>

#include <functional>

class GraphScene;

class SpatialOctTree
{
public:
    struct SubVolume
    {
        BoundingBox3D boundingBox;
        QVector<NodeId> nodeIds;
        SpatialOctTree* subTree;
    };

private:
    QVector3D centre;
    SubVolume subVolumes[8];

public:
    SpatialOctTree(const BoundingBox3D& boundingBox, const QVector<NodeId> nodeIds, const NodePositions& nodePositions,
                   std::function<bool(SpatialOctTree::SubVolume*)> predicate = [](SubVolume*) { return true; });
    SpatialOctTree(const BoundingBox3D& boundingBox, const QVector<NodeId> nodeIds, const NodePositions& nodePositions,
                   const QVector3D& origin, const QVector3D& direction);

    virtual ~SpatialOctTree();

    SubVolume& subVolumeForPoint(const QVector3D& point);
    QList<const SubVolume*> leaves(std::function<bool(const SubVolume*, int)> predicate, int treeDepth = 0) const;
    QList<const SubVolume*> leaves() const { return leaves([](const SubVolume*, int){ return true; }); }
    void visitVolumes(std::function<void(const SubVolume*, int treeDepth)> visitor = [](const SubVolume*, int){}, int treeDepth = 0) const;
    void dumpToQDebug();
    void debugRenderOctTree(GraphScene* graphScene, const QVector3D& offset = QVector3D());
};

#endif // SPATIALOCTREE_H
