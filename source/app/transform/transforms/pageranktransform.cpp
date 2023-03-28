/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "pageranktransform.h"

#include "transform/transformedgraph.h"

#include "graph/graphcomponent.h"
#include "graph/graphmodel.h"
#include "graph/componentmanager.h"

#include <blaze/Blaze.h>

#include <QElapsedTimer>
#include <QDebug>

#include <map>

using namespace Qt::Literals::StringLiterals;

using VectorType = blaze::DynamicVector<float>;

void PageRankTransform::apply(TransformedGraph& target)
{
    calculatePageRank(target);
}

void PageRankTransform::calculatePageRank(TransformedGraph& target)
{
    // Performs an estimated pagerank calculation optimised to
    // not use a matrix. This dramatically lowers the memory footprint.
    // http://www.dcs.bbk.ac.uk/~dell/teaching/cc/book/mmds/mmds_ch5_2.pdf
    // http://michaelnielsen.org/blog/using-your-laptop-to-compute-pagerank-for-millions-of-webpages/
    NodeArray<float> pageRankScores(target);

    setPhase(u"PageRank"_s);

    // We must do our own componentisation as the graph's set of components
    // won't necessarily be up-to-date
    const ComponentManager componentManager(target);

    int totalIterationCount = 0;
    for(auto componentId : componentManager.componentIds())
    {        
        const IGraphComponent* component = componentManager.componentById(componentId);
        auto componentNodeCount = component->nodeIds().size();

        // Map NodeIds to Matrix index
        NodeIdMap<size_t> nodeToIndexMap;
        for(auto nodeId : component->nodeIds())
        {
            auto index = nodeToIndexMap.size();
            nodeToIndexMap[nodeId] = index;
        }

        QElapsedTimer timer;
        if (_debug)
            timer.start();

        VectorType pageRankVector(componentNodeCount,
            1.0f / static_cast<float>(componentNodeCount));
        VectorType newPageRankVector(componentNodeCount);
        VectorType delta(componentNodeCount);
        float change = std::numeric_limits<float>::max();
        int iterationCount = 0;
        std::deque<float> changeBuffer;
        float previousBufferChangeAverage = 0.0f;
        float pagerankAcceleration = std::numeric_limits<float>::max();
        while(change > PAGERANK_EPSILON &&
              iterationCount < PAGERANK_ITERATION_LIMIT &&
              pagerankAcceleration > PAGERANK_ACCELERATION_MINIMUM)
        {
            if(cancelled())
                return;

            setPhase(u"PageRank Iteration %1"_s.arg(
                                QString::number(totalIterationCount + 1)));

            // Calculate pagerank
            for(auto nodeId : component->nodeIds())
            {
                auto matrixId = nodeToIndexMap[nodeId];
                float prSum = 0.0f;

                for(auto edgeId : target.edgeIdsForNodeId(nodeId))
                {
                    auto oppositeNodeId = target.edgeById(edgeId).oppositeId(nodeId);
                    prSum += pageRankVector[nodeToIndexMap[oppositeNodeId]] /
                        static_cast<float>(target.nodeById(oppositeNodeId).degree());
                }

                newPageRankVector[matrixId] = (prSum * PAGERANK_DAMPING) +
                    ((1.0f - PAGERANK_DAMPING) / static_cast<float>(componentNodeCount));
            }

            // Normalise result
            float sum = 0.0f;
            for(auto value : newPageRankVector)
                sum += value;
             newPageRankVector = newPageRankVector / sum;

            // Detect PR Change
            change = 0.0f;
            delta = blaze::abs(newPageRankVector - pageRankVector);
            for(size_t i = 0UL; i < newPageRankVector.size(); ++i )
                change += delta[i];

            // Oscillation detection (delta avg)
            changeBuffer.push_front(change);
            if(changeBuffer.size() >= static_cast<size_t>(AVG_COUNT))
                changeBuffer.pop_back();

            // Average the last 10 steps
            float bufferChangeAverage = 0.0f;
            for(auto changes : changeBuffer)
                bufferChangeAverage += changes;
            bufferChangeAverage /= static_cast<float>(AVG_COUNT);

            pagerankAcceleration = std::abs(previousBufferChangeAverage - bufferChangeAverage);

            // Only update the previousAvg after AVG_COUNT steps
            if(iterationCount % AVG_COUNT == 0)
                previousBufferChangeAverage = bufferChangeAverage;

            pageRankVector = newPageRankVector;
            iterationCount++;
            totalIterationCount++;
        }
        if(_debug && iterationCount == PAGERANK_ITERATION_LIMIT)
            qDebug() << "HIT ITERATION LIMIT ON PAGERANK. LIKELY UNSTABLE PAGERANK VECTOR";

        const float maxValue = blaze::max(blaze::abs(pageRankVector) );
        pageRankVector = pageRankVector / maxValue;

        for(auto nodeId : component->nodeIds())
            pageRankScores[nodeId] = pageRankVector[nodeToIndexMap[nodeId]];

        if (_debug)
        {
            std::stringstream matrixStream;
            matrixStream << pageRankVector;
            qDebug() << "PageRank";
            qDebug().noquote() << QString::fromStdString(matrixStream.str());

            qDebug() << "Pagerank took" << iterationCount << "iterations";
            qDebug() << "The efficient pagerank operation took" << timer.elapsed();
        }
    }

    _graphModel->createAttribute(QObject::tr("Node PageRank"))
        .setDescription(QObject::tr("A node's PageRank is a measure of relative importance in the graph."))
        .floatRange().setMin(0.0f)
        .floatRange().setMax(1.0f)
        .setFloatValueFn([pageRankScores](NodeId nodeId) { return pageRankScores[nodeId]; })
        .setFlag(AttributeFlag::VisualiseByComponent);
}

std::unique_ptr<GraphTransform> PageRankTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<PageRankTransform>(graphModel());
}

