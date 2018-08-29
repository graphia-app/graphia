#ifndef BARNESHUTTREE_H
#define BARNESHUTTREE_H

#include "octree.h"
#include "shared/utils/fixedsizestack.h"

#include <QVector3D>

#include <functional>

class BarnesHutTree;
struct BarnesHutSubVolume : SubVolume<BarnesHutTree> { float _sSq = 0.0f; };

class BarnesHutTree : public BaseOctree<BarnesHutTree, BarnesHutSubVolume>
{
private:
    static constexpr float E = 0.0001f;

    // Cycle through different epsilon vectors so that there is enough
    // variation that the forces don't get stuck in 2 or fewer dimensions
    mutable size_t _di = 0;
    QVector3D differenceEpsilon() const
    {
        static std::array<QVector3D, 6> vs =
        {{
            {   E, 0.0f, 0.0f},
            {0.0f,    E, 0.0f},
            {0.0f, 0.0f,    E},
            {  -E, 0.0f, 0.0f},
            {0.0f,   -E, 0.0f},
            {0.0f, 0.0f,   -E},
        }};

        _di = (_di + 1) % vs.size();
        return vs.at(_di);
    }

    float constexpr distanceSqEpsilon() const
    {
        return E * E;
    }

    float _theta = 0.8f;
    int _mass = 0;
    QVector3D _centreOfMass;

    void initialise(const NodePositions& nodePositions, const std::vector<NodeId>& nodeIds) override;

public:
    BarnesHutTree();

    void setTheta(float theta) { _theta = theta; }

    template<typename Fn>
    QVector3D evaluateKernel(const NodePositions& nodePositions, NodeId nodeId, const Fn& kernel) const
    {
        const QVector3D& nodePosition = nodePositions.get(nodeId);
        QVector3D result;
        FixedSizeStack<const BarnesHutTree*> stack(_depthFirstTraversalStackSizeRequirement);

        stack.push(this);

        while(!stack.empty())
        {
            const BarnesHutTree* subTree = stack.pop();

            for(int i = 0; i < subTree->_numInternalNodes; i++)
            {
                auto subVolume = subTree->_internalNodes.at(i);

                const QVector3D& centreOfMass = subVolume->_subTree->_centreOfMass;
                QVector3D difference = (centreOfMass - nodePosition);
                float distanceSq = difference.lengthSquared();

                if(distanceSq == 0.0f)
                {
                    difference = differenceEpsilon();
                    distanceSq = distanceSqEpsilon();
                }

                const float sOverD = subVolume->_sSq / distanceSq;

                if(sOverD > _theta)
                    stack.push(subVolume->_subTree.get());
                else
                    result += kernel(subVolume->_subTree->_mass, difference, distanceSq);
            }

            for(int i = 0; i < subTree->_numNonEmptyLeaves; i++)
            {
                auto subVolume = subTree->_nonEmptyLeaves.at(i);

                NodeId otherNodeId = subVolume->_nodeIds.front();
                if(otherNodeId != nodeId)
                {
                    const QVector3D& otherNodePosition = nodePositions.get(otherNodeId);
                    QVector3D difference = otherNodePosition - nodePosition;
                    float distanceSq = difference.lengthSquared();

                    if(distanceSq == 0.0f)
                    {
                        difference = differenceEpsilon();
                        distanceSq = distanceSqEpsilon();
                    }

                    result += kernel(1, difference, distanceSq);
                }
            }
        }

        return result;
    }
};

#endif // BARNESHUTTREE_H
