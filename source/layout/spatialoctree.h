#ifndef SPATIALOCTREE_H
#define SPATIALOCTREE_H

#include "../graph/graph.h"
#include "../maths/boundingbox.h"
#include "../layout/layout.h"

#include <QVector3D>

#include <functional>
#include <vector>
#include <memory>

class GraphComponentScene;

class SpatialOctree
{
public:
    struct SubVolume
    {
        BoundingBox3D _boundingBox;
        std::vector<NodeId> _nodeIds;
        std::unique_ptr<SpatialOctree> _subTree;
    };

private:
    QVector3D _centre;
    SubVolume _subVolumes[8];

public:
    SpatialOctree(const BoundingBox3D& boundingBox, const std::vector<NodeId> nodeIds, const NodePositions& nodePositions,
                   std::function<bool(SpatialOctree::SubVolume&)> predicate = [](SubVolume&) { return true; });
    SpatialOctree(const BoundingBox3D& boundingBox, const std::vector<NodeId> nodeIds, const NodePositions& nodePositions,
                   const QVector3D& origin, const QVector3D& direction);

    SubVolume& subVolumeForPoint(const QVector3D& point);
    std::vector<const SubVolume*> leaves(std::function<bool(const SubVolume&, int)> predicate, int treeDepth = 0) const;
    std::vector<const SubVolume*> leaves() const { return leaves([](const SubVolume&, int){ return true; }); }
    void visitVolumes(std::function<void(const SubVolume&, int treeDepth)> visitor = [](const SubVolume&, int){}, int treeDepth = 0) const;
    void dumpToQDebug();
    void debugRenderOctree(GraphComponentScene* graphComponentScene, const QVector3D& offset = QVector3D());
};

#endif // SPATIALOCTREE_H
