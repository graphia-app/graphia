#include "barneshuttree.h"

#include <stack>

BarnesHutTree::BarnesHutTree()
{
    setMaxNodesPerLeaf(1);
}

void BarnesHutTree::initialise(const NodeLayoutPositions& nodePositions, const std::vector<NodeId>& nodeIds)
{
    _mass = static_cast<int>(nodeIds.size());
    _centreOfMass = nodePositions.centreOfMass(nodeIds);

    for(auto& subVolume : _subVolumes)
        subVolume._sSq = subVolume._boundingBox.maxLength() * subVolume._boundingBox.maxLength();
}
