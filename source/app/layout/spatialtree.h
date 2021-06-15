/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef SPATIALTREE_H
#define SPATIALTREE_H

#include "shared/graph/igraphcomponent.h"
#include "maths/boundingbox.h"
#include "nodepositions.h"
#include "shared/utils/scopetimer.h"
#include "shared/utils/threadpool.h"

#include <QVector3D>
#include <QColor>

#include <functional>
#include <vector>
#include <memory>
#include <stack>
#include <array>
#include <cstdlib>

class AbstractSpatialTree
{
public:
    virtual ~AbstractSpatialTree() = default;
    virtual void build(const IGraphComponent& graph, const NodeLayoutPositions& nodePositions) = 0;
};

template<size_t NumDimensions>
using BoundingBox = typename std::conditional_t<NumDimensions == 3,
    BoundingBox3D, typename std::conditional_t<NumDimensions == 2,
    BoundingBox2D, void>>;

template<typename TreeType, size_t NumDimensions>
struct SubVolume
{
    BoundingBox<NumDimensions> _boundingBox;
    std::vector<NodeId> _nodeIds;
    std::unique_ptr<TreeType> _subTree;
    bool _leaf = true;
    bool _empty = true;

    // Floating point precision means that it becomes impossible to
    // divide a bounding box beyond a certain minimum size. We don't
    // want to allow that to happen, obviously, so this is a means
    // to detect whether it would and avoid subdivision if so
    bool divisible() const
    {
        const auto cx = _boundingBox.centre().x();
        const auto xh = _boundingBox.xLength() * 0.5f;

        if(cx + xh == cx || cx - xh == cx)
            return false;

        const auto cy = _boundingBox.centre().y();
        const auto yh = _boundingBox.yLength() * 0.5f;

        if(cy + yh == cy || cy - yh == cy)
            return false;

        if constexpr(NumDimensions == 3)
        {
            const auto cz = _boundingBox.centre().z();
            const auto zh = _boundingBox.zLength() * 0.5f;

            if(cz + zh == cz || cz - zh == cz)
                return false; // NOLINT
        }

        return true;
    }
};

// Use the CRT pattern so we can create instances of subclasses by default constructor
template<typename TreeType, size_t NumDimensions, typename SubVolumeType = SubVolume<TreeType, NumDimensions>>
class SpatialTree : virtual public AbstractSpatialTree
{
private:
    static constexpr size_t NumSubVolumes()
    {
        size_t v = 1;

        for(auto i = 0ul; i < NumDimensions; i++)
            v *= 2;

        return v;
    }

    BoundingBox<NumDimensions> _boundingBox;

protected:
    size_t _depthFirstTraversalStackSizeRequirement = 0; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    std::array<SubVolumeType, NumSubVolumes()> _subVolumes = {}; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

    std::array<const SubVolumeType*, NumSubVolumes()> _nonEmptyLeaves = {}; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    int _numNonEmptyLeaves = 0; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

    std::array<const SubVolumeType*, NumSubVolumes()> _internalNodes = {}; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    int _numInternalNodes = 0; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

private:
    unsigned int _maxNodesPerLeaf = 1;

    // Temporary data structure used in the creation of the tree
    struct NewTree
    {
        SpatialTree* _tree;
        std::vector<NodeId> _nodeIds;

        NewTree(SpatialTree* tree, const std::vector<NodeId>& nodeIds) noexcept : // NOLINT
            _tree(tree), _nodeIds(nodeIds)
        {}

        NewTree(SpatialTree* tree, std::vector<NodeId>&& nodeIds) noexcept :
            _tree(tree), _nodeIds(std::move(nodeIds))
        {}

        NewTree(const NewTree& other) = default;
        NewTree& operator=(const NewTree& other) = default;
        NewTree(NewTree&& other) = default; // NOLINT
        NewTree& operator=(NewTree&& other) = default; // NOLINT
    };

    void initialiseSubVolumes()
    {
        const auto cx = _boundingBox.centre().x();
        const auto cy = _boundingBox.centre().y();
        const auto xh = _boundingBox.xLength() * 0.5f;
        const auto yh = _boundingBox.yLength() * 0.5f;

        if constexpr(NumDimensions == 3)
        {
            const auto cz = _boundingBox.centre().z();
            const auto zh = _boundingBox.zLength() * 0.5f;

            // clang-format off
            _subVolumes[0]._boundingBox = {{cx - xh, cy - yh, cz - zh}, {cx,      cy,      cz     }};
            _subVolumes[1]._boundingBox = {{cx,      cy - yh, cz - zh}, {cx + xh, cy,      cz     }};
            _subVolumes[2]._boundingBox = {{cx - xh, cy,      cz - zh}, {cx,      cy + yh, cz     }};
            _subVolumes[3]._boundingBox = {{cx,      cy,      cz - zh}, {cx + xh, cy + yh, cz     }};

            _subVolumes[4]._boundingBox = {{cx - xh, cy - yh, cz     }, {cx,      cy,      cz + zh}};
            _subVolumes[5]._boundingBox = {{cx,      cy - yh, cz     }, {cx + xh, cy,      cz + zh}};
            _subVolumes[6]._boundingBox = {{cx - xh, cy,      cz     }, {cx,      cy + yh, cz + zh}};
            _subVolumes[7]._boundingBox = {{cx,      cy,      cz     }, {cx + xh, cy + yh, cz + zh}};
            // clang-format on
        }
        else if constexpr(NumDimensions == 2)
        {
            // clang-format off
            _subVolumes[0]._boundingBox = {{cx - xh, cy - yh}, {cx,      cy,    }};
            _subVolumes[1]._boundingBox = {{cx,      cy - yh}, {cx + xh, cy,    }};
            _subVolumes[2]._boundingBox = {{cx - xh, cy,    }, {cx,      cy + yh}};
            _subVolumes[3]._boundingBox = {{cx,      cy,    }, {cx + xh, cy + yh}};
            // clang-format on
        }

        for(auto& subVolume : _subVolumes)
            Q_ASSERT(subVolume._boundingBox.valid());
    }

    void distributeNodesOverSubVolumes(const NodeLayoutPositions& nodePositions, const std::vector<NodeId>& nodeIds)
    {
        initialiseSubVolumes();

        bool distinctPositions = false;
        QVector3D lastPosition = nodePositions.get(nodeIds[0]);

        // Distribute NodeIds over SubVolumes
        for(NodeId nodeId : nodeIds)
        {
            const QVector3D& nodePosition = nodePositions.get(nodeId);
            SubVolumeType& subVolume = subVolumeForPoint(nodePosition);

            subVolume._nodeIds.push_back(nodeId);

            if(!distinctPositions)
            {
                if(nodePosition != lastPosition)
                    distinctPositions = true;
                else
                    lastPosition = nodePosition;
            }
        }

        // Decide if the SubVolumes need further sub-division
        for(auto& subVolume : _subVolumes)
        {
            if(subVolume._nodeIds.empty())
                continue;

            if(subVolume._nodeIds.size() > _maxNodesPerLeaf &&
               subVolume.divisible() && distinctPositions)
            {
                // Subdivide
                subVolume._subTree = std::make_unique<TreeType>();
                subVolume._subTree->_boundingBox = subVolume._boundingBox;

                subVolume._leaf = false;
                _internalNodes.at(_numInternalNodes++) = &subVolume;
            }
            else
            {
                subVolume._empty = false;
                _nonEmptyLeaves.at(_numNonEmptyLeaves++) = &subVolume;
            }
        }
    }

    // The second parameter and superset of _subVolumes[x]._nodeIds are the
    // same, at the point when this is called
    virtual void initialise(const NodeLayoutPositions&, const std::vector<NodeId>&) {}

    void build(const std::vector<NodeId>& nodeIds, const NodeLayoutPositions& nodePositions)
    {
        SCOPE_TIMER_MULTISAMPLES(50)

        std::vector<NewTree> newTrees;
        newTrees.emplace_back(this, nodeIds);

        while(!newTrees.empty())
        {
            auto results = parallel_for(newTrees.begin(), newTrees.end(),
            [&nodePositions](typename std::vector<NewTree>::iterator it)
            {
                auto* subTree = it->_tree;
                const auto& nodeIdsToDistribute = it->_nodeIds;

                subTree->distributeNodesOverSubVolumes(nodePositions, nodeIdsToDistribute);

                std::vector<NewTree> newChildTrees;
                for(int i = 0; i < subTree->_numInternalNodes; i++)
                {
                    const auto* subVolume = subTree->_internalNodes.at(i);
                    newChildTrees.emplace_back(subVolume->_subTree.get(),
                        std::move(subVolume->_nodeIds));
                }

                subTree->initialise(nodePositions, nodeIdsToDistribute);

                return newChildTrees;
            });

            // subTrees has now been processsed, but may have resulted in more subTrees
            newTrees.clear();
            newTrees.insert(newTrees.end(), std::make_move_iterator(results.begin()),
                std::make_move_iterator(results.end()));
        }

        std::stack<const SpatialTree*> stack;
        stack.push(this);
        _depthFirstTraversalStackSizeRequirement = 1;
        while(!stack.empty())
        {
            const SpatialTree* subTree = stack.top();
            stack.pop();

            for(int i = 0; i < subTree->_numInternalNodes; i++)
            {
                const auto* subVolume = subTree->_internalNodes.at(i);
                stack.push(subVolume->_subTree.get());
                _depthFirstTraversalStackSizeRequirement =
                    std::max(_depthFirstTraversalStackSizeRequirement, stack.size());
            }
        }
    }

    SubVolumeType& subVolumeForPoint(const QVector3D& point)
    {
        size_t i = 0;
        QVector3D diff = point - _boundingBox.centre();

        if constexpr(NumDimensions == 3)
        {
            if(diff.z() >= 0.0f)
                i += 4;
        }

        if(diff.y() >= 0.0f)
            i += 2;

        if(diff.x() >= 0.0f)
            i += 1;

        Q_ASSERT(i < NumSubVolumes());
        SubVolumeType& subVolume = _subVolumes.at(i);

        if(!subVolume._leaf)
            return subVolume._subTree->subVolumeForPoint(point);

        return subVolume;
    }

    const SubVolumeType& subVolumeForPoint(const QVector3D& point) const
    {
        return subVolumeForPoint(point);
    }

protected:
    void setMaxNodesPerLeaf(unsigned int maxNodesPerLeaf) { _maxNodesPerLeaf = maxNodesPerLeaf; }

public:
    void build(const IGraphComponent& graph, const NodeLayoutPositions& nodePositions) override
    {
        if constexpr(NumDimensions == 2)
        {
            auto boundingBox3D = nodePositions.boundingBox(graph.nodeIds());
            _boundingBox = {boundingBox3D.min().toVector2D(), boundingBox3D.max().toVector2D()};
        }
        else if constexpr(NumDimensions == 3)
            _boundingBox = nodePositions.boundingBox(graph.nodeIds());

        Q_ASSERT(_boundingBox.valid());
        build(graph.nodeIds(), nodePositions);
    }
};

#endif // SPATIALTREE_H
