#ifndef BARNESHUTTREE_H
#define BARNESHUTTREE_H

#include "octree.h"
#include "../utils/fixedsizestack.h"

#include <QVector3D>

#include <functional>

class BarnesHutTree;
struct BarnesHutSubVolume : SubVolume<BarnesHutTree> { float _sSq; };

class BarnesHutTree : public BaseOctree<BarnesHutTree, BarnesHutSubVolume>
{
private:
    float _theta = 0.8f;
    int _mass = 0;
    QVector3D _centreOfMass;

    void initialiseTreeNode() override;

public:
    BarnesHutTree();

    void setTheta(float theta) { _theta = theta; }

    template<typename Fn> QVector3D evaluateKernel(NodeId nodeId, Fn&& kernel) const
    {
        const QVector3D& nodePosition = _nodePositions->get(nodeId);
        QVector3D result;
        FixedSizeStack<const BarnesHutTree*> stack(_depth);

        stack.push(this);

        while(!stack.empty())
        {
            const BarnesHutTree* subTree = stack.pop();

            for(int i = 0; i < subTree->_numInternalNodes; i++)
            {
                auto subVolume = subTree->_internalNodes[i];

                const QVector3D& centreOfMass = subVolume->_subTree->_centreOfMass;
                const QVector3D difference = centreOfMass - nodePosition;
                const float distanceSq = difference.lengthSquared();
                const float sOverD = subVolume->_sSq / distanceSq;

                if(sOverD > _theta)
                    stack.push(subVolume->_subTree.get());
                else
                    result += kernel(subVolume->_subTree->_mass, difference, distanceSq);
            }

            for(int i = 0; i < subTree->_numNonEmptyLeaves; i++)
            {
                auto subVolume = subTree->_nonEmptyLeaves[i];

                NodeId otherNodeId = subVolume->_nodeIds.front();
                if(otherNodeId != nodeId)
                {
                    const QVector3D& otherNodePosition = _nodePositions->get(otherNodeId);
                    const QVector3D difference = otherNodePosition - nodePosition;
                    const float distanceSq = difference.lengthSquared();
                    result += kernel(1, difference, distanceSq);
                }
            }
        }

        return result;
    }
};

#endif // BARNESHUTTREE_H
