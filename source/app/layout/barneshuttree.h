#ifndef BARNESHUTTREE_H
#define BARNESHUTTREE_H

#include "octree.h"
#include "shared/utils/fixedsizestack.h"

#include <QVector3D>

#include <functional>

class BarnesHutTree;
struct BarnesHutSubVolume : SubVolume<BarnesHutTree> { float _sSq; };

class BarnesHutTree : public BaseOctree<BarnesHutTree, BarnesHutSubVolume>
{
private:
    static Q_DECL_CONSTEXPR QVector3D differenceEpsilon() { return QVector3D(0.0f, 0.0f, 0.0001f); }
    static Q_DECL_CONSTEXPR float distanceSqEpsilon()
    {
        auto e = differenceEpsilon();
        return e.x() + e.x() + e.y() + e.y() + e.z() + e.z();
    }

    float _theta = 0.8f;
    int _mass = 0;
    QVector3D _centreOfMass;

    void initialiseTreeNode(const std::vector<NodeId> &nodeIds) override;

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
                    const QVector3D& otherNodePosition = _nodePositions->get(otherNodeId);
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
