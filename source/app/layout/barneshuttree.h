/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BARNESHUTTREE_H
#define BARNESHUTTREE_H

#include "spatialtree.h"
#include "shared/utils/fixedsizestack.h"

#include <QVector3D>

#include <functional>

class AbstractBarnesHutTree : virtual public AbstractSpatialTree
{
public:
    virtual QVector3D evaluateKernel(const NodeLayoutPositions& nodePositions, NodeId nodeId,
        const std::function<QVector3D(int, const QVector3D&, float)>& kernel) const = 0;
};

template<size_t NumDimensions>
class BarnesHutTree;

template<size_t NumDimensions>
struct BarnesHutSubVolume : SubVolume<BarnesHutTree<NumDimensions>, NumDimensions> { float _sSq = 0.0f; };

template<size_t NumDimensions>
class BarnesHutTree :
    public SpatialTree<BarnesHutTree<NumDimensions>, NumDimensions, BarnesHutSubVolume<NumDimensions>>,
    public AbstractBarnesHutTree
{
private:
    static constexpr float E = 0.0001f;
    static constexpr float E2 = E * E;

    // Cycle through different epsilon vectors so that there is enough
    // variation that the forces don't get stuck in 2 or fewer dimensions
    mutable size_t _di = 0;
    QVector3D differenceEpsilon() const
    {
        if constexpr(NumDimensions == 3)
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
        else if constexpr(NumDimensions == 2)
        {
            static std::array<QVector3D, 4> vs =
            {{
                {   E, 0.0f, 0.0f},
                {0.0f,    E, 0.0f},
                {  -E, 0.0f, 0.0f},
                {0.0f,   -E, 0.0f},
            }};

            _di = (_di + 1) % vs.size();
            return vs.at(_di);
        }
    }

    float _theta = 0.8f;
    int _mass = 0;
    QVector3D _centreOfMass;

    void initialise(const NodeLayoutPositions& nodePositions, const std::vector<NodeId>& nodeIds) override
    {
        _mass = static_cast<int>(nodeIds.size());
        _centreOfMass = nodePositions.centreOfMass(nodeIds);

        for(auto& subVolume : this->_subVolumes)
            subVolume._sSq = subVolume._boundingBox.maxLength() * subVolume._boundingBox.maxLength();
    }

public:
    BarnesHutTree()
    {
        this->setMaxNodesPerLeaf(1);
    }

    void setTheta(float theta) { _theta = theta; }

    QVector3D evaluateKernel(const NodeLayoutPositions& nodePositions, NodeId nodeId,
        const std::function<QVector3D(int, const QVector3D&, float)>& kernel) const override
    {
        const QVector3D& nodePosition = nodePositions.get(nodeId);
        QVector3D result;
        FixedSizeStack<const BarnesHutTree*> stack(this->_depthFirstTraversalStackSizeRequirement);

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
                    distanceSq = E2;
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
                        distanceSq = E2;
                    }

                    result += kernel(1, difference, distanceSq);
                }
            }
        }

        return result;
    }
};

using BarnesHutTree2D = BarnesHutTree<2>;
using BarnesHutTree3D = BarnesHutTree<3>;

#endif // BARNESHUTTREE_H
