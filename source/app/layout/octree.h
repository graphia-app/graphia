#ifndef OCTREE_H
#define OCTREE_H

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

template<typename TreeType> struct SubVolume
{
    BoundingBox3D _boundingBox;
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

        const auto cz = _boundingBox.centre().z();
        const auto zh = _boundingBox.zLength() * 0.5f;

        if(cz + zh == cz || cz - zh == cz)
            return false; // NOLINT

        return true;
    }
};

// Use the CRT pattern so we can create instances of subclasses by default constructor
template<typename TreeType, typename SubVolumeType = SubVolume<TreeType>> class BaseOctree
{
protected:
    BoundingBox3D _boundingBox; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    size_t _depthFirstTraversalStackSizeRequirement = 0; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    std::array<SubVolumeType, 8> _subVolumes = {}; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

    std::array<const SubVolumeType*, 8> _nonEmptyLeaves = {}; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    int _numNonEmptyLeaves = 0; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

    std::array<const SubVolumeType*, 8> _internalNodes = {}; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    int _numInternalNodes = 0; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

private:
    unsigned int _maxNodesPerLeaf = 1;

    // Temporary data structure used in the creation of the tree
    struct NewTree
    {
        BaseOctree* _tree;
        std::vector<NodeId> _nodeIds;

        NewTree(BaseOctree* tree, const std::vector<NodeId>& nodeIds) noexcept : // NOLINT
            _tree(tree), _nodeIds(nodeIds)
        {}

        NewTree(BaseOctree* tree, std::vector<NodeId>&& nodeIds) noexcept :
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
        const auto cz = _boundingBox.centre().z();
        const auto xh = _boundingBox.xLength() * 0.5f;
        const auto yh = _boundingBox.yLength() * 0.5f;
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
                Q_ASSERT(subVolume._boundingBox.volume() > 0.0f);
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

public:
    virtual ~BaseOctree() = default;

    void setMaxNodesPerLeaf(unsigned int maxNodesPerLeaf) { _maxNodesPerLeaf = maxNodesPerLeaf; }

    void build(const std::vector<NodeId>& nodeIds, const NodeLayoutPositions& nodePositions)
    {
        SCOPE_TIMER_MULTISAMPLES(50)

        std::vector<NewTree> newTrees;
        newTrees.emplace_back(this, nodeIds);

        while(!newTrees.empty())
        {
            auto results = concurrent_for(newTrees.begin(), newTrees.end(),
            [&nodePositions](typename std::vector<NewTree>::iterator it)
            {
                auto* subTree = it->_tree;
                const auto& nodeIdsToDistribute = it->_nodeIds;

                subTree->distributeNodesOverSubVolumes(nodePositions, nodeIdsToDistribute);

                std::vector<NewTree> newChildTrees;
                for(int i = 0; i < subTree->_numInternalNodes; i++)
                {
                    auto subVolume = subTree->_internalNodes.at(i);
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

        std::stack<const BaseOctree*> stack;
        stack.push(this);
        _depthFirstTraversalStackSizeRequirement = 1;
        while(!stack.empty())
        {
            const BaseOctree* subTree = stack.top();
            stack.pop();

            for(int i = 0; i < subTree->_numInternalNodes; i++)
            {
                auto subVolume = subTree->_internalNodes.at(i);
                stack.push(subVolume->_subTree.get());
                _depthFirstTraversalStackSizeRequirement =
                    std::max(_depthFirstTraversalStackSizeRequirement, stack.size());
            }
        }
    }

    void build(const IGraphComponent& graph, const NodeLayoutPositions& nodePositions)
    {
        _boundingBox = nodePositions.boundingBox(graph.nodeIds());
        Q_ASSERT(_boundingBox.valid());
        build(graph.nodeIds(), nodePositions);
    }

    SubVolumeType& subVolumeForPoint(const QVector3D& point)
    {
        int i = 0;
        QVector3D diff = point - _boundingBox.centre();

        if(diff.z() >= 0.0f)
            i += 4;

        if(diff.y() >= 0.0f)
            i += 2;

        if(diff.x() >= 0.0f)
            i += 1;

        SubVolumeType& subVolume = _subVolumes.at(i);

        if(!subVolume._leaf)
            return subVolume._subTree->subVolumeForPoint(point);

        return subVolume;
    }

    const SubVolumeType& subVolumeForPoint(const QVector3D& point) const
    {
        return subVolumeForPoint(point);
    }

    void visitVolumes(std::function<bool(const SubVolumeType&, int treeDepth)> visitor = [](const SubVolumeType&, int){}) const
    {
        std::stack<std::tuple<const BaseOctree*, int>> stack;
        stack.emplace(this, 0);

        while(!stack.empty())
        {
            auto[subTree, treeDepth] = stack.top();
            stack.pop();

            for(const auto& subVolume : subTree->_subVolumes)
            {
                if(!visitor(subVolume, treeDepth))
                    continue;

                if(!subVolume._leaf)
                    stack.emplace(subVolume._subTree.get(), treeDepth + 1);
            }
        }
    }

    std::vector<const SubVolumeType*> leaves(std::function<bool(const SubVolumeType&, int)> predicate =
            [](const SubVolumeType&, int) { return true; }) const
    {
        std::vector<const SubVolumeType*> leafVolumes;

        visitVolumes(
            [&leafVolumes, predicate](const SubVolumeType& subVolume, int treeDepth)
            {
                if(!predicate(subVolume, treeDepth))
                    return false;

                if(subVolume._leaf)
                    leafVolumes.push_back(&subVolume);

                return true;
            });

        return leafVolumes;
    }

    void dumpToQDebug() const
    {
        QString dump;
        int totalNodes = 0;

        visitVolumes(
            [&dump, &totalNodes](const SubVolumeType& subVolume, int treeDepth)
            {
                QString indentString = QLatin1String("");
                for(int i = 0; i < treeDepth; i++)
                    indentString += QLatin1String("  ");

                QString subVolumeString = QStringLiteral("0x%1").arg(&subVolume);

                dump.append(indentString).append(subVolumeString);
                if(subVolume._nodeIds.empty())
                    dump.append(" empty");
                dump.append("\n");

                for(auto nodeId : subVolume._nodeIds )
                {
                    dump.append(indentString).append("[\n");
                    dump.append(indentString).append("  ").append(nodeId).append("\n");
                    dump.append(indentString).append("]\n");
                }

                totalNodes += subVolume._nodeIds.size();

                return true;
            });

        qDebug() << dump << "Total nodes:" << totalNodes;
    }
};

class Octree : public BaseOctree<Octree> {};

#endif // OCTREE_H
