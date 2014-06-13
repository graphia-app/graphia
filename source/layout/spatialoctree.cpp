#include "spatialoctree.h"

#include "../layout/layout.h"
#include "../rendering/graphcomponentscene.h"
#include "../maths/ray.h"

#include <unordered_set>
#include <vector>

static void subDivideBoundingBox(const BoundingBox3D& boundingBox, SpatialOctree::SubVolume subVolumes[8])
{
    const QVector3D c = boundingBox.centre();
    const float cx = c.x();
    const float cy = c.y();
    const float cz = c.z();
    const float xh = boundingBox.xLength() * 0.5f;
    const float yh = boundingBox.yLength() * 0.5f;
    const float zh = boundingBox.zLength() * 0.5f;

    subVolumes[0]._boundingBox = BoundingBox3D(QVector3D(cx - xh, cy - yh, cz - zh), QVector3D(cx,      cy,      cz     ));
    subVolumes[1]._boundingBox = BoundingBox3D(QVector3D(cx,      cy - yh, cz - zh), QVector3D(cx + xh, cy,      cz     ));
    subVolumes[2]._boundingBox = BoundingBox3D(QVector3D(cx - xh, cy,      cz - zh), QVector3D(cx,      cy + yh, cz     ));
    subVolumes[3]._boundingBox = BoundingBox3D(QVector3D(cx,      cy,      cz - zh), QVector3D(cx + xh, cy + yh, cz     ));

    subVolumes[4]._boundingBox = BoundingBox3D(QVector3D(cx - xh, cy - yh, cz     ), QVector3D(cx,      cy,      cz + zh));
    subVolumes[5]._boundingBox = BoundingBox3D(QVector3D(cx,      cy - yh, cz     ), QVector3D(cx + xh, cy,      cz + zh));
    subVolumes[6]._boundingBox = BoundingBox3D(QVector3D(cx - xh, cy,      cz     ), QVector3D(cx,      cy + yh, cz + zh));
    subVolumes[7]._boundingBox = BoundingBox3D(QVector3D(cx,      cy,      cz     ), QVector3D(cx + xh, cy + yh, cz + zh));
}

static bool distributeNodesOverVolume(SpatialOctree& spatialOctTree, const std::vector<NodeId> nodeIds,
                                      const NodePositions& nodePositions, std::unordered_set<SpatialOctree::SubVolume*>& subVolumes)
{
    bool distinctPositions = false;
    QVector3D lastPosition = nodePositions[nodeIds[0]];

    for(NodeId nodeId : nodeIds)
    {
        const QVector3D& nodePosition = nodePositions[nodeId];
        SpatialOctree::SubVolume& subVolume = spatialOctTree.subVolumeForPoint(nodePosition);

        subVolume._nodeIds.push_back(nodeId);
        subVolumes.insert(&subVolume);

        if(!distinctPositions)
        {
            if(nodePosition != lastPosition)
                distinctPositions = true;
            else
                lastPosition = nodePosition;
        }
    }

    return distinctPositions;
}

static void distributeNodesOverSubVolumes(SpatialOctree& spatialOctTree, const std::vector<NodeId> nodeIds,
                                          const NodePositions& nodePositions,
                                          std::function<bool(SpatialOctree::SubVolume*)> predicate)
{
    const int MAX_NODES_PER_LEAF = 4;

    std::unordered_set<SpatialOctree::SubVolume*> subVolumes;
    bool distinctPositions = distributeNodesOverVolume(spatialOctTree, nodeIds, nodePositions, subVolumes);

    for(SpatialOctree::SubVolume* subVolume : subVolumes)
    {
        if(!predicate(subVolume))
            continue;

        if(subVolume->_nodeIds.size() > MAX_NODES_PER_LEAF && distinctPositions)
        {
            // Subdivide
            subVolume->_subTree = new SpatialOctree(subVolume->_boundingBox, subVolume->_nodeIds, nodePositions);
            subVolume->_nodeIds.clear();
        }
    }
}

SpatialOctree::SpatialOctree(const BoundingBox3D& boundingBox, const std::vector<NodeId> nodeIds, const NodePositions& nodePositions, std::function<bool(SubVolume*)> predicate) :
    _centre(boundingBox.centre())
{
    for(int i = 0; i < 8; i++)
        _subVolumes[i]._subTree = nullptr;

    subDivideBoundingBox(boundingBox, _subVolumes);
    distributeNodesOverSubVolumes(*this, nodeIds, nodePositions, predicate);
}

SpatialOctree::SpatialOctree(const BoundingBox3D& boundingBox, const std::vector<NodeId> nodeIds, const NodePositions& nodePositions,
                               const QVector3D& origin, const QVector3D& direction) :
    _centre(boundingBox.centre())
{
    for(int i = 0; i < 8; i++)
        _subVolumes[i]._subTree = nullptr;

    subDivideBoundingBox(boundingBox, _subVolumes);
    Ray ray(origin, direction);
    distributeNodesOverSubVolumes(*this, nodeIds, nodePositions,
        [&ray](SpatialOctree::SubVolume* subVolume)
        {
            return subVolume->_boundingBox.intersects(ray);
        });
}

SpatialOctree::~SpatialOctree()
{
    for(int i = 0; i < 8; i++)
        delete _subVolumes[i]._subTree;
}

SpatialOctree::SubVolume& SpatialOctree::subVolumeForPoint(const QVector3D& point)
{
    int i = 0;
    QVector3D diff = point - _centre;

    if(diff.z() >= 0.0f)
        i += 4;

    if(diff.y() >= 0.0f)
        i += 2;

    if(diff.x() >= 0.0f)
        i += 1;

    SubVolume& subVolume = _subVolumes[i];

    if(subVolume._subTree != nullptr)
        return subVolume._subTree->subVolumeForPoint(point);

    return subVolume;
}

std::vector<const SpatialOctree::SubVolume*> SpatialOctree::leaves(std::function<bool (const SpatialOctree::SubVolume*, int)> predicate, int treeDepth) const
{
    std::vector<const SubVolume*> leafVolumes;

    for(int i = 0; i < 8; i++)
    {
        const SubVolume& subVolume = _subVolumes[i];

        if(!predicate(&subVolume, treeDepth))
            continue;

        if(subVolume._subTree != nullptr)
        {
            auto subLeafVolumes = subVolume._subTree->leaves(predicate, treeDepth + 1);
            leafVolumes.insert(leafVolumes.begin(), subLeafVolumes.begin(), subLeafVolumes.end());
        }
        else
            leafVolumes.push_back(&subVolume);
    }

    return leafVolumes;
}

void SpatialOctree::visitVolumes(std::function<void (const SpatialOctree::SubVolume*, int)> visitor, int treeDepth) const
{
    for(int i = 0; i < 8; i++)
    {
        const SubVolume& subVolume = _subVolumes[i];
        visitor(&subVolume, treeDepth);

        if(subVolume._subTree != nullptr)
            subVolume._subTree->visitVolumes(visitor, treeDepth + 1);
    }
}

void SpatialOctree::dumpToQDebug()
{
    visitVolumes([](const SubVolume* subVolume, int treeDepth)
    {
        QString indentString = "";
        for(int i = 0; i < treeDepth; i++)
            indentString += "  ";

        QString subVolumeString;
        subVolumeString.sprintf("0x%p", subVolume);

        qDebug() << (indentString + subVolumeString);
        for(auto nodeId : subVolume->_nodeIds )
            qDebug() << nodeId;
    });
}

void SpatialOctree::debugRenderOctree(GraphComponentScene* graphComponentScene, const QVector3D& offset)
{
    visitVolumes(
        [&graphComponentScene, &offset](const SpatialOctree::SubVolume* subVolume, int)
        {
            int r = std::abs(static_cast<int>(subVolume->_boundingBox.centre().x() * 10) % 255);
            int g = std::abs(static_cast<int>(subVolume->_boundingBox.centre().y() * 10) % 255);
            int b = std::abs(static_cast<int>(subVolume->_boundingBox.centre().z() * 10) % 255);
            int mean = std::max((r + g + b) / 3, 1);
            const int TARGET = 192;
            r = std::min((r * TARGET) / mean, 255);
            g = std::min((g * TARGET) / mean, 255);
            b = std::min((b * TARGET) / mean, 255);

            QColor lineColor(r, g, b);

            if(!subVolume->_nodeIds.empty())
                graphComponentScene->addDebugBoundingBox(subVolume->_boundingBox + offset, lineColor);
        });
}
