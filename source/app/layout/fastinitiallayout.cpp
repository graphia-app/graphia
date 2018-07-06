#include "fastinitiallayout.h"

#include "maths/constants.h"

#include <QMatrix4x4>
#include <QVector4D>

#include <cmath>

void FastInitialLayout::positionNode(QVector3D& offsetPosition, const QMatrix4x4& orientationMatrix,
                                     const QVector3D& parentNodePosition, NodeId childNodeId,
                                     NodeArray<QVector3D>& directionNodeVectors)
{
    offsetPosition = offsetPosition * SPHERE_RADIUS;
    offsetPosition = offsetPosition * orientationMatrix;

    directionNodeVectors.set(childNodeId, offsetPosition.normalized());
    positions().set(childNodeId, parentNodePosition + offsetPosition);
}

void FastInitialLayout::executeReal(bool)
{
    auto& graph = graphComponent().graph();
    NodeArray<bool> visitedNodes(graph);
    NodeArray<QVector3D> directionNodeVectors(graph);

    std::queue<NodeId> nodeQueue;
    nodeQueue.push(nodeIds().front());
    visitedNodes.set(nodeIds().front(), true);

    // This performs a breadth-first tree layout, positioning child nodes in a "spiral"
    // configuration around the parent. This approximates equal distrubution on a sphere.
    // The end result is a quick and dirty "spanning tree" layout
    while(!nodeQueue.empty())
    {
        auto parentNodeId = nodeQueue.front();
        nodeQueue.pop();

        auto edgeIds = graph.nodeById(parentNodeId).edgeIds();
        QVector3D parentNodePosition = positions().get(parentNodeId);

        QMatrix4x4 orientationMatrix;
        orientationMatrix.setToIdentity();
        QVector3D forward = directionNodeVectors.at(parentNodeId);
        // All except initial node, calculate an orientation matrix. This makes
        // the tree grow outwards from parent nodes
        if(parentNodeId != nodeIds().front())
        {
            QVector3D up = QVector3D(forward.z(), -forward.x(), forward.y());

            double dot = QVector3D::dotProduct(up, forward);
            up -= (dot * forward);
            up.normalize();

            QVector3D right = QVector3D::crossProduct(up, forward);
            orientationMatrix.setRow(0, QVector4D(right));
            orientationMatrix.setRow(1, QVector4D(up));
            orientationMatrix.setRow(2, QVector4D(forward));
        }

        auto edgeIdIterator = edgeIds.begin();
        int edgeCountOffset = 0;

        // First node (bottom)
        // First two nodes connected to base node should be positioned top and
        // bottom, UNLESS the node has a parent node, in which case do not position
        // the first node as it would be in the parents position!
        if(edgeIdIterator != edgeIds.end() && parentNodeId == nodeIds().front())
        {
            auto& edge = graph.edgeById(*edgeIdIterator);
            auto childNodeId = edge.oppositeId(parentNodeId);

            bool visited = visitedNodes.get(childNodeId);
            if(!visited)
            {
                QVector3D offsetPosition(1.0, 0.0, 0.0);
                positionNode(offsetPosition,
                             orientationMatrix,
                             parentNodePosition,
                             childNodeId,
                             directionNodeVectors);

                nodeQueue.push(childNodeId);
                visitedNodes.set(childNodeId, true);
                ++edgeIdIterator;
            }
        }
        else
        {
            // Add a dummy edge to the size to calculate spiral positions for the extra node
            edgeCountOffset++;
        }

        // Second node (top)
        if(edgeIdIterator != edgeIds.end())
        {
            auto& edge = graph.edgeById(*edgeIdIterator);
            auto childNodeId = edge.oppositeId(parentNodeId);
            bool visited = visitedNodes.get(childNodeId);

            // Find the next connected node which has not been visted already
            while(visited)
            {
                ++edgeIdIterator;
                if(edgeIdIterator != edgeIds.end())
                {
                    auto &newEdge = graph.edgeById(*edgeIdIterator);
                    childNodeId = newEdge.oppositeId(parentNodeId);
                    visited = visitedNodes.get(childNodeId);
                }
                else
                    break;
            }

            if(!visited)
            {
                nodeQueue.push(childNodeId);

                QVector3D offsetPosition(1.0, 0.0, 0.0);

                positionNode(offsetPosition,
                             orientationMatrix,
                             parentNodePosition,
                             childNodeId,
                             directionNodeVectors);

                visitedNodes.set(childNodeId, true);

                ++edgeIdIterator;
            }
        }

        double phi = 0.0;
        // i = 2 because we've theoretically already positioned the top and bottom
        int i = 2;
        for(;edgeIdIterator != edgeIds.end(); ++edgeIdIterator)
        {
            auto& edge = graph.edgeById(*edgeIdIterator);
            auto childNodeId = edge.oppositeId(parentNodeId);

            if(visitedNodes.get(childNodeId))
                continue;

            nodeQueue.push(childNodeId);

            double h = -1.0 + 2.0 * (i - 1.0) / static_cast<double>(edgeIds.size() - 1 + edgeCountOffset);
            double theta = std::acos(h);
            phi = phi + 3.6 / (std::sqrt((static_cast<double>(edgeIds.size() + edgeCountOffset)) * (1.0 - h * h)));
            phi = std::fmod(phi, 2.0 * static_cast<double>(Constants::Pi()));

            QVector3D offsetPosition(h, std::cos(phi) * std::sin(theta), std::sin(phi) * std::sin(theta));

            positionNode(offsetPosition,
                         orientationMatrix,
                         parentNodePosition,
                         childNodeId,
                         directionNodeVectors);

            visitedNodes.set(childNodeId, true);
            i++;
        }
    }
}
