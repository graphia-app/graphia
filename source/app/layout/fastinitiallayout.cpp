/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "fastinitiallayout.h"

#include <QMatrix4x4>
#include <QVector4D>

#include <cmath>
#include <numbers>

void FastInitialLayout::positionNode(QVector3D& offsetPosition, const QMatrix4x4& orientationMatrix,
                                     const QVector3D& parentNodePosition, NodeId childNodeId,
                                     NodeArray<QVector3D>& directionNodeVectors)
{
    const float SPHERE_RADIUS = 20.0f;
    offsetPosition = offsetPosition * SPHERE_RADIUS;
    offsetPosition = (QVector4D(offsetPosition, 1.0f) * orientationMatrix).toVector3D();

    directionNodeVectors.set(childNodeId, offsetPosition.normalized());
    positions().set(childNodeId, parentNodePosition + offsetPosition);
}

void FastInitialLayout::execute(bool, Dimensionality dimensionality)
{
    const auto& graph = graphComponent().graph();
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
        const QVector3D parentNodePosition = positions().get(parentNodeId);

        QMatrix4x4 orientationMatrix;
        orientationMatrix.setToIdentity();
        const QVector3D forward = directionNodeVectors.at(parentNodeId);
        // All except initial node, calculate an orientation matrix. This makes
        // the tree grow outwards from parent nodes
        if(parentNodeId != nodeIds().front())
        {
            QVector3D up = QVector3D(forward.z(), -forward.x(), forward.y());

            auto dot = QVector3D::dotProduct(up, forward);
            up -= (dot * forward);
            up.normalize();

            const QVector3D right = QVector3D::crossProduct(up, forward);
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
            const auto& edge = graph.edgeById(*edgeIdIterator);
            auto childNodeId = edge.oppositeId(parentNodeId);

            const bool visited = visitedNodes.get(childNodeId);
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
            const auto& edge = graph.edgeById(*edgeIdIterator);
            auto childNodeId = edge.oppositeId(parentNodeId);
            bool visited = visitedNodes.get(childNodeId);

            // Find the next connected node which has not been visted already
            while(visited)
            {
                ++edgeIdIterator;
                if(edgeIdIterator != edgeIds.end())
                {
                    const auto &newEdge = graph.edgeById(*edgeIdIterator);
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

        float phi = 0.0f;
        // i = 2 because we've theoretically already positioned the top and bottom
        int i = 2;
        for(;edgeIdIterator != edgeIds.end(); ++edgeIdIterator)
        {
            const auto& edge = graph.edgeById(*edgeIdIterator);
            auto childNodeId = edge.oppositeId(parentNodeId);

            if(visitedNodes.get(childNodeId))
                continue;

            nodeQueue.push(childNodeId);

            auto h = -1.0f + 2.0f * (static_cast<float>(i) - 1.0f) / static_cast<float>(
                static_cast<int>(edgeIds.size()) - 1 + edgeCountOffset);
            auto theta = std::acos(h);
            phi = phi + 3.6f / (std::sqrt((static_cast<float>(
                static_cast<int>(edgeIds.size()) + edgeCountOffset)) * (1.0f - h * h)));
            phi = std::fmod(phi, 2.0f * static_cast<float>(std::numbers::pi_v<float>));

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

    if(dimensionality == Layout::Dimensionality::TwoDee)
        positions().flatten();
}
