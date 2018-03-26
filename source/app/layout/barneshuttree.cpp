#include "barneshuttree.h"

#include <stack>

BarnesHutTree::BarnesHutTree()
{
    setMaxNodesPerLeaf(1);
}

void BarnesHutTree::initialiseTreeNode(const std::vector<NodeId>& nodeIds)
{
    _mass = static_cast<int>(nodeIds.size());
    _centreOfMass = NodePositions::centreOfMass(*_nodePositions, nodeIds);

    for(auto& subVolume : _subVolumes)
        subVolume._sSq = subVolume._boundingBox.maxLength() * subVolume._boundingBox.maxLength();
}
