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
    _centreOfMass = NodePositions::centreOfMass(*_nodePositions, _nodeIds);

    for(auto& subVolume : _subVolumes)
        subVolume._sSq = subVolume._boundingBox.maxLength() * subVolume._boundingBox.maxLength();
}
