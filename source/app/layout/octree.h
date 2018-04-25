#ifndef OCTREE_H
#define OCTREE_H

#include "shared/graph/igraphcomponent.h"
#include "maths/boundingbox.h"
#include "nodepositions.h"

#include <QVector3D>
#include <QColor>

#include <functional>
#include <vector>
#include <memory>
#include <deque>
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

    bool _passesPredicate = true;

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
    const NodePositions* _nodePositions = nullptr;
    BoundingBox3D _boundingBox;
    QVector3D _centre;
    size_t _depth = 0;
    std::array<SubVolumeType, 8> _subVolumes = {};

    std::array<const SubVolumeType*, 8> _nonEmptyLeaves = {};
    int _numNonEmptyLeaves = 0;

    std::array<const SubVolumeType*, 8> _internalNodes = {};
    int _numInternalNodes = 0;

private:
    unsigned int _maxNodesPerLeaf = 1;
    std::function<bool(const BoundingBox3D&)> _predicate;

    struct SubTree
    {
        BaseOctree* _tree;
        std::vector<NodeId> _nodeIds;

        SubTree(BaseOctree* tree, const std::vector<NodeId>& nodeIds) noexcept :
            _tree(tree), _nodeIds(nodeIds)
        {}

        SubTree(BaseOctree* tree, std::vector<NodeId>&& nodeIds) noexcept :
            _tree(tree), _nodeIds(std::move(nodeIds))
        {}

        SubTree(const SubTree& other) = default;
        SubTree& operator=(const SubTree& other) = default;
        SubTree(SubTree&& other) = default;
        SubTree& operator=(SubTree&& other) = default;
    };

    std::deque<SubTree> distributeNodesOverSubVolumes(const std::vector<NodeId>& nodeIds)
    {
        bool distinctPositions = false;
        QVector3D lastPosition = _nodePositions->get(nodeIds[0]);

        // Distribute NodeIds over SubVolumes
        for(NodeId nodeId : nodeIds)
        {
            const QVector3D& nodePosition = _nodePositions->get(nodeId);
            SubVolumeType& subVolume = subVolumeForPoint(nodePosition);

            if(!subVolume._passesPredicate)
                continue;

            subVolume._nodeIds.push_back(nodeId);

            if(!distinctPositions)
            {
                if(nodePosition != lastPosition)
                    distinctPositions = true;
                else
                    lastPosition = nodePosition;
            }
        }

        std::deque<SubTree> subTrees;

        // Decide if the SubVolumes need further sub-division
        for(auto& subVolume : _subVolumes)
        {
            if(!subVolume._passesPredicate || subVolume._nodeIds.empty())
                continue;

            if(subVolume._nodeIds.size() > _maxNodesPerLeaf &&
               subVolume.divisible() && distinctPositions)
            {
                // Subdivide
                subVolume._subTree = std::make_unique<TreeType>();
                Q_ASSERT(subVolume._boundingBox.volume() > 0.0f);
                subVolume._subTree->_boundingBox = subVolume._boundingBox;
                subTrees.emplace_back(subVolume._subTree.get(), std::move(subVolume._nodeIds));

                subVolume._leaf = false;
                _internalNodes.at(_numInternalNodes++) = &subVolume;
            }
            else
            {
                subVolume._empty = false;
                _nonEmptyLeaves.at(_numNonEmptyLeaves++) = &subVolume;
            }
        }

        return subTrees;
    }

    // The parameter and superset of _subVolumes[x]._nodeIds contain the same data, when this is called
    virtual void initialiseTreeNode(const std::vector<NodeId>&) {}

public:
    BaseOctree() : _predicate([](const BoundingBox3D&) { return true; }) {}
    virtual ~BaseOctree() = default;

    void setMaxNodesPerLeaf(unsigned int maxNodesPerLeaf) { _maxNodesPerLeaf = maxNodesPerLeaf; }
    void setPredicate(std::function<bool(const BoundingBox3D&)> predicate) { _predicate = predicate; }

    void build(const std::vector<NodeId>& nodeIds, const NodePositions& nodePositions,
               const BoundingBox3D &boundingBox)
    {
        _boundingBox = boundingBox;
        _depth = 0;

        std::deque<SubTree> subTrees;
        subTrees.emplace_back(this, nodeIds);

        while(!subTrees.empty())
        {
            _depth = std::max(_depth, subTrees.size());

            BaseOctree* subTree = subTrees.back()._tree;
            std::vector<NodeId> nodeIdsToDistribute = std::move(subTrees.back()._nodeIds);
            subTrees.pop_back();

            subTree->_centre = subTree->_boundingBox.centre();
            subTree->_nodePositions = &nodePositions;

            const auto cx = subTree->_centre.x();
            const auto cy = subTree->_centre.y();
            const auto cz = subTree->_centre.z();
            const auto xh = subTree->_boundingBox.xLength() * 0.5f;
            const auto yh = subTree->_boundingBox.yLength() * 0.5f;
            const auto zh = subTree->_boundingBox.zLength() * 0.5f;

            // clang-format off
            subTree->_subVolumes[0]._boundingBox = {{cx - xh, cy - yh, cz - zh}, {cx,      cy,      cz     }};
            subTree->_subVolumes[1]._boundingBox = {{cx,      cy - yh, cz - zh}, {cx + xh, cy,      cz     }};
            subTree->_subVolumes[2]._boundingBox = {{cx - xh, cy,      cz - zh}, {cx,      cy + yh, cz     }};
            subTree->_subVolumes[3]._boundingBox = {{cx,      cy,      cz - zh}, {cx + xh, cy + yh, cz     }};

            subTree->_subVolumes[4]._boundingBox = {{cx - xh, cy - yh, cz     }, {cx,      cy,      cz + zh}};
            subTree->_subVolumes[5]._boundingBox = {{cx,      cy - yh, cz     }, {cx + xh, cy,      cz + zh}};
            subTree->_subVolumes[6]._boundingBox = {{cx - xh, cy,      cz     }, {cx,      cy + yh, cz + zh}};
            subTree->_subVolumes[7]._boundingBox = {{cx,      cy,      cz     }, {cx + xh, cy + yh, cz + zh}};
            // clang-format on

            for(auto& subVolume : subTree->_subVolumes)
            {
                Q_ASSERT(subVolume._boundingBox.valid());
                subVolume._passesPredicate = _predicate(subVolume._boundingBox);
            }

            auto newSubTrees = subTree->distributeNodesOverSubVolumes(nodeIdsToDistribute);
            subTrees.insert(subTrees.end(), std::make_move_iterator(newSubTrees.begin()),
                std::make_move_iterator(newSubTrees.end()));

            subTree->initialiseTreeNode(nodeIdsToDistribute);
        }
    }

    void build(const IGraphComponent& graph, const NodePositions& nodePositions)
    {
        BoundingBox3D boundingBox = BoundingBox3D(NodePositions::positionsVector(nodePositions, graph.nodeIds()));
        Q_ASSERT(boundingBox.valid());
        build(graph.nodeIds(), nodePositions, boundingBox);
    }

    SubVolumeType& subVolumeForPoint(const QVector3D& point)
    {
        int i = 0;
        QVector3D diff = point - _centre;

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
            const BaseOctree* subTree;
            int treeDepth;
            std::tie(subTree, treeDepth) = stack.top();
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

                QString subVolumeString;
                subVolumeString.sprintf("0x%p", &subVolume);

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
