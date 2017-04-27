#include "pageranktransform.h"
#include "graph/graphmodel.h"
#ifdef _MSC_VER
#if _MSC_VER > 1900
#error Check if blaze is still giving strange unreferenced parameter warnings
#endif
#pragma warning( push )
#pragma warning( disable : 4100 ) // Unreferenced formal parameter
#endif
#include "blaze/Blaze.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <QElapsedTimer>

#include <map>

using VectorType = blaze::DynamicVector<float>;

bool PageRankTransform::apply(TransformedGraph& target) const
{
    calculatePageRank(target);
    return false;
}

void PageRankTransform::calculatePageRank(TransformedGraph& target) const
{
    // Performs an estimated pagerank calculation optimised to
    // not use a matrix. This dramatically lowers the memory footprint.
    // http://www.dcs.bbk.ac.uk/~dell/teaching/cc/book/mmds/mmds_ch5_2.pdf
    // http://michaelnielsen.org/blog/using-your-laptop-to-compute-pagerank-for-millions-of-webpages/
    NodeArray<float> pageRankScores(_graphModel->graph());

    target.setPhase(QString("PageRank"));

    int totalIterationCount = 0;
    for(auto componentId : _graphModel->graph().componentIds())
    {        
        const IGraphComponent* component = _graphModel->graph().componentById(componentId);
        int componentNodeCount = static_cast<int>(component->nodeIds().size());

        // Map NodeIds to Matrix index
        std::map<NodeId, int> nodeToIndexMap;
        std::map<int, NodeId> indexToNodeMap;
        for(auto nodeId : component->nodeIds())
        {
            int index = static_cast<int>(nodeToIndexMap.size());
            nodeToIndexMap[nodeId] = index;
            indexToNodeMap[index] = nodeId;
        }

        QElapsedTimer timer;
        if (_debug)
            timer.start();

        VectorType pageRankVector(componentNodeCount, 1.0f / componentNodeCount);
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
            target.setPhase(QString("PageRank Iteration %1").arg(
                                QString::number(totalIterationCount + 1)));

            // Calculate pagerank
            for(auto nodeId : component->nodeIds())
            {
                auto matrixId = nodeToIndexMap[nodeId];
                float prSum = 0.0f;
                for(auto edgeId : _graphModel->graph().edgeIdsForNodeId(nodeId))
                {
                    auto oppositeNodeId = _graphModel->graph().edgeById(edgeId).oppositeId(nodeId);
                    prSum += pageRankVector[nodeToIndexMap[oppositeNodeId]] /
                            _graphModel->graph().nodeById(oppositeNodeId).degree();
                }
                 newPageRankVector[matrixId] = (prSum * PAGERANK_DAMPING) +
                         ((1.0f - PAGERANK_DAMPING) / componentNodeCount);
            }

            // Normalise result
            float sum = 0.0f;
            for(size_t i = 0UL; i < newPageRankVector.size(); ++i )
                sum += newPageRankVector[i];
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

        float maxValue = blaze::max(blaze::abs(pageRankVector) );
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

    _graphModel->createAttribute(QObject::tr("Node PageRank")).setDescription(
                "A node's PageRank is a measure of relative importance in the graph")
            .floatRange().setMin(0.0f)
            .floatRange().setMax(1.0f)
            .setFloatValueFn([pageRankScores](NodeId nodeId) { return pageRankScores[nodeId]; });
}

std::unique_ptr<GraphTransform> PageRankTransformFactory::create(const GraphTransformConfig&) const
{
    auto testTransform = std::make_unique<PageRankTransform>(graphModel());

    return std::move(testTransform); //FIXME std::move required because of clang bug
}

