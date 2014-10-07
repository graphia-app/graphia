#include "barneshuttree.h"

#include <stack>

BarnesHutTree::BarnesHutTree() :
    BaseOctree(),
    _theta(0.8f)
{
    setMaxNodesPerLeaf(1);
}

void BarnesHutTree::initialiseTreeNode()
{
    _mass = static_cast<int>(_nodeIds.size());
    float massReciprocal = 1.0f / _mass;
    _centreOfMass = QVector3D();

    for(auto nodeId : _nodeIds)
        _centreOfMass += (_nodePositions->get(nodeId) * massReciprocal);

    for(auto& subVolume : _subVolumes)
        subVolume._sSq = subVolume._boundingBox.maxLength() * subVolume._boundingBox.maxLength();
}
