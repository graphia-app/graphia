#include "mcltransform.h"
#include "graph/graphmodel.h"
#include "shared/utils/threadpool.h"

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
#include <set>
#include <thread>

using MatrixType = blaze::CompressedMatrix<float,blaze::columnMajor>;
using VectorType = blaze::DynamicVector<float,blaze::columnVector>;

static void normaliseColumnsColumnMajor(MatrixType &mclMatrix)
{
    for(size_t column = 0; column<mclMatrix.columns(); column++)
    {
        MatrixType::Iterator lend=mclMatrix.end(column);

        float value = 0.0f;
        for(MatrixType::Iterator lelem=mclMatrix.begin(column); lelem!=lend; ++lelem)
            value += lelem->value();

        Q_ASSERT(value > 0.0f);
        value = 1.0f / value;

        for(MatrixType::Iterator lelem=mclMatrix.begin(column); lelem!=lend; ++lelem)
            lelem->value() = lelem->value() * value;
    }
}

static void sumColumns(MatrixType &mclMatrix, VectorType &output)
{
    for(size_t column = 0; column<mclMatrix.columns(); column++)
    {
        MatrixType::ConstIterator lend=mclMatrix.cend(column);
        for(MatrixType::ConstIterator lelem=mclMatrix.cbegin(column); lelem!=lend; ++lelem)
        {
            output[column] = output[column] + lelem->value();
        }
    }
}

static void expandAndPruneRow(MatrixType &mclMatrix,
                              size_t columnId,
                              std::vector<MCLTransform::SparseMatrixEntry>* matrixStorage,
                              MCLTransform::MCLRowData& rowData, float minValueCutoff)
{
    const bool DEBUG = false;

    // Perhaps make these changable parameters later
    const int RECOVERY_COUNT = 1400;
    const int SELECTION_COUNT = 1100;

    size_t nonzeros = 0;

    size_t minIndex(std::numeric_limits<size_t>::infinity()), maxIndex(0UL);
    MatrixType::ConstIterator lend=mclMatrix.cend(columnId);

    // Perform multiply and populate prune dependant data structures
    for(MatrixType::ConstIterator lelem=mclMatrix.cbegin(columnId); lelem!=lend; ++lelem)
    {
        // For each column (left)
        const MatrixType::ConstIterator rend(mclMatrix.cend(lelem->index()));
        for(MatrixType::ConstIterator relem= mclMatrix.cbegin( lelem->index() ); relem!=rend; ++relem)
        {
            // For each column starting at left column index (right)
            float mult = lelem->value() * relem->value();

            if(!rowData.valid[relem->index()])
            {
                rowData.values[relem->index()] = mult;
                rowData.valid[relem->index()] = 1;
                // Position in the rowData.values vector
                rowData.indices[nonzeros] = relem->index();
                ++nonzeros;

                if( relem->index() < minIndex ) minIndex = relem->index();
                if( relem->index() > maxIndex ) maxIndex = relem->index();
            }
            else
            {
                rowData.values[relem->index()] += mult;
            }
        }
    }

    if(nonzeros > 0UL)
    {
        Q_ASSERT(minIndex <= maxIndex);

        size_t remainCount = nonzeros;
        float rowPruneSum = 0.0f;
        // Mass is always normalised!
        float targetMass = 0.9f;

        for(size_t i = 0UL; i < nonzeros; ++i)
        {
            size_t index = rowData.indices[i];
            if( std::abs( rowData.values[index] ) <= minValueCutoff)
            {
                // Remove from the remainCount;
                remainCount--;
            }
            else
            {
                // Calculate pruned sum
                rowPruneSum += rowData.values[index];
            }
        }

        if(DEBUG)
            qDebug() << "RowPruneSum (mass?)" << rowPruneSum << "targetmass" << targetMass;

        if(remainCount != nonzeros && rowPruneSum < targetMass && remainCount < RECOVERY_COUNT)
        {
            // Recover
            if(DEBUG)
                qDebug() << "RECOVERY" << "MASS:" << rowPruneSum;
            std::nth_element(rowData.indices.begin(), rowData.indices.begin() + RECOVERY_COUNT, rowData.indices.begin() + nonzeros,
                             [&rowData](size_t i1, size_t i2) {return rowData.values[i1] > rowData.values[i2];});
            minValueCutoff = rowData.values[rowData.indices[RECOVERY_COUNT]];
            remainCount = RECOVERY_COUNT;
            rowPruneSum = 0;
            for(size_t i = 0UL; i < RECOVERY_COUNT; ++i)
                rowPruneSum += rowData.values[rowData.indices[i]];
        }
        else if(remainCount > SELECTION_COUNT)
        {
            // Selection prune
            // Refine the cutoff so MAXIMUM SELECTION_COUNT elements remain
            // Should speedtest this vs copy+std::greater<float>
            std::nth_element(rowData.indices.begin(), rowData.indices.begin() + SELECTION_COUNT, rowData.indices.begin() + nonzeros,
                             [&rowData](size_t i1, size_t i2) {return rowData.values[i1] > rowData.values[i2];});

            minValueCutoff = rowData.values[rowData.indices[SELECTION_COUNT]];

            if(DEBUG)
            {
                qDebug() << "Selection Cutoff" << minValueCutoff;
                qDebug() << "Pre selection Remain" << remainCount << "mass" << rowPruneSum;
            }

            // Recalculate new mass
            remainCount = SELECTION_COUNT;
            rowPruneSum = 0;
            for(size_t i = 0UL; i < SELECTION_COUNT; ++i)
                rowPruneSum += rowData.values[rowData.indices[i]];

            if(DEBUG)
                qDebug() << "Post selection Remain" << remainCount << "mass" << rowPruneSum;

            // Do Another recovery if needed
            if(remainCount != nonzeros && rowPruneSum < targetMass && remainCount < RECOVERY_COUNT)
            {
                if(DEBUG)
                    qDebug() << "RECOVERY 2" << "MASS:" << rowPruneSum;

                std::nth_element(rowData.indices.begin(), rowData.indices.begin() + RECOVERY_COUNT, rowData.indices.begin() + nonzeros,
                                 [&rowData](size_t i1, size_t i2) {return rowData.values[i1] > rowData.values[i2];});
                minValueCutoff = rowData.values[rowData.indices[RECOVERY_COUNT]];
                remainCount = RECOVERY_COUNT;
                rowPruneSum = 0;
                for(size_t i = 0UL; i < RECOVERY_COUNT; ++i)
                    rowPruneSum += rowData.values[rowData.indices[i]];
            }
        }

        // Finally, remove unneeded rowData.indices
        if(remainCount < nonzeros)
        {
            if(DEBUG)
            {
                qDebug() << "Prune:" << nonzeros - remainCount;
                qDebug() << "Raw count" << nonzeros;
                qDebug() << "Remain Count" << remainCount;
            }
            for(size_t i = 0UL; i < nonzeros; ++i)
            {
                if(std::abs(rowData.values[rowData.indices[i]]) <= minValueCutoff)
                {
                    rowData.values[rowData.indices[i]] = 0.0f;
                    rowData.valid[rowData.indices[i]] = 0.0f;
                }
                else
                {
                    // Rescale
                    rowData.values[rowData.indices[i]] /= rowPruneSum;
                }
            }
        }

        if(DEBUG)
        {
            qDebug() << columnId << "pruned" << nonzeros - remainCount;
            qDebug() << rowData.values;
        }

        // Populate new matrix
        // If sorting is too big just brute force the whole range
        // If the range is small just do it contiguously
        const float EPSILON = 1e-8f;
        if((nonzeros + nonzeros) < (maxIndex - minIndex))
        {
            std::sort(rowData.indices.begin(), rowData.indices.begin() + nonzeros);

            for(size_t j = 0UL; j<nonzeros; ++j)
            {
                const size_t index = rowData.indices[j];
                if(rowData.values[index] > EPSILON)
                {
                    matrixStorage->emplace_back(index, columnId, rowData.values[index]);
                    rowData.values[index] = 0.0f;
                }
                rowData.valid[index] = 0.0f;
            }
        }
        else
        {
            for(size_t j = minIndex; j <= maxIndex; ++j)
            {
                if(rowData.values[j] > EPSILON)
                {
                    matrixStorage->emplace_back(j, columnId, rowData.values[j]);
                    rowData.values[j] = 0.0f;
                }
                rowData.valid[j] = 0.0f;
            }
        }
    }
}

bool MCLTransform::apply(TransformedGraph& target) const
{
    double granularity = boost::get<double>(
                config().parameterByName("Granularity")->_value);

    if(_debugIteration)
    {
        QElapsedTimer mclTimer;
        mclTimer.start();
        calculateMCL(granularity, target);
        qDebug() << "MCL Elapsed Time" << mclTimer.elapsed();
    }
    else
        calculateMCL(granularity, target);

    return false;
}

void MCLTransform::calculateMCL(float inflation, TransformedGraph& target) const
{
    target.setPhase("MCL Initialising");

    auto& graph = _graphModel->graph();
    int nodeCount = graph.numNodes();

    // Map NodeIds to Matrix index
    std::map<NodeId, size_t> nodeToIndexMap;
    std::map<size_t, NodeId> indexToNodeMap;
    for(NodeId nodeId : graph.nodeIds())
    {
        auto index = nodeToIndexMap.size();
        nodeToIndexMap[nodeId] = index;
        indexToNodeMap[index] = nodeId;
    }

    MatrixType clusterMatrix(nodeCount, nodeCount);
    blaze::setNumThreads(std::thread::hardware_concurrency());

    clusterMatrix.reserve((graph.numEdges()*2) + nodeCount);

    // Populate the Matrix
    for(NodeId nodeId : graph.nodeIds())
    {
        auto nodeIndex = nodeToIndexMap[nodeId];
        auto connectedEdgeIds = graph.edgeIdsForNodeId(nodeId);

        // Add all connected node indexes to sorted set
        std::set<size_t> sortNodeIndexes;
        for(auto connectEdgeId : connectedEdgeIds)
        {
            auto connectedNodeId = graph.edgeById(connectEdgeId).oppositeId(nodeId);
            sortNodeIndexes.insert(nodeToIndexMap[connectedNodeId]);
        }
        // Add self loop
        sortNodeIndexes.insert(nodeIndex);

        // Append to Compressed Matrix
        for(auto sortedConnectIndex : sortNodeIndexes)
            clusterMatrix.append(sortedConnectIndex, nodeIndex, 1.0f);

        // Finalise the row
        clusterMatrix.finalize(nodeIndex);
    }
    clusterMatrix.trim();

    std::stringstream matrixStream;
    if(_debugMatrices)
    {
        matrixStream << clusterMatrix;
        qDebug().noquote() << QString::fromStdString(matrixStream.str());
    }

    // Normalise the columns
    normaliseColumnsColumnMajor(clusterMatrix);

    // Pre-inflation
    clusterMatrix = pow(clusterMatrix, 3.0f);

    // Normalise again...
    normaliseColumnsColumnMajor(clusterMatrix);

    if(_debugIteration)
        qDebug() << "Pre nnz" << clusterMatrix.nonZeros();

    clusterMatrix.erase([this]( double value ){ return value < MCL_PRUNE_LIMIT; } );

    if(_debugIteration)
        qDebug() << "Pre-prune nnz" << clusterMatrix.nonZeros();

    if(_debugMatrices)
    {
        matrixStream << "Pre-inflated Matrix \n";
        matrixStream << clusterMatrix;
        qDebug().noquote() << QString::fromStdString(matrixStream.str());
        matrixStream.clear();
        matrixStream.str(std::string());
    }

    bool isEquiDistrubuted = true;
    // Start the MCL loop
    int iter = 0;
    do
    {
        target.setPhase(QString("MCL Iteration %1").arg(QString::number(iter + 1)));
        if(_debugMatrices)
        {
            matrixStream << "Pre-Expanded Matrix\n";
            matrixStream << clusterMatrix;
            qDebug().noquote() << QString::fromStdString(matrixStream.str());
            matrixStream.clear();
            matrixStream.str(std::string());
        }

        isEquiDistrubuted = true;

        if(_debugIteration)
            qDebug() << "Iteration" << iter;

        std::vector<std::vector<SparseMatrixEntry>> matrixStorage(clusterMatrix.rows());

        QElapsedTimer threadedTimer;
        if(_debugIteration)
            threadedTimer.start();

        // Threaded expansion
        ColumnsIterator<MatrixType> colIterator(clusterMatrix);

        // To reduce vector allocations for every row we do some magic with rowData
        MCLRowData rowData(clusterMatrix.columns());
        // Pass by value rowData, this gives each THREAD a copy of rowData, rather re-allocating vectors per row (slow)
        concurrent_for(colIterator.begin(), colIterator.end(),
                       [this, rowData, &clusterMatrix, &matrixStorage](const size_t iterator) mutable
        {
            expandAndPruneRow(clusterMatrix, iterator, &matrixStorage[iterator], rowData, MCL_PRUNE_LIMIT);
        }, true);

        int totalThreadedTime = 0;
        if(_debugIteration)
        {
            totalThreadedTime = threadedTimer.restart();
            qDebug() << "Threaded Expansion Calculation time ms" << totalThreadedTime;
            threadedTimer.restart();
        }

        size_t newNNZCount = 0;
        for(auto& column : matrixStorage)
            newNNZCount += column.size();

        MatrixType dstMatrix(clusterMatrix.rows(), clusterMatrix.rows());
        dstMatrix.reserve(newNNZCount);
        int row = 0;
        for(auto matrixRow : matrixStorage)
        {
            for(auto matrixEntry: matrixRow)
                dstMatrix.append(matrixEntry._row, matrixEntry._column, matrixEntry._value);

            dstMatrix.finalize(row);
            row++;
        }
        clusterMatrix = dstMatrix;

        if(_debugIteration)
        {
            int populationTime = threadedTimer.restart();
            totalThreadedTime += populationTime;
            qDebug() << "Threaded Expansion Population time LL ms" << populationTime;

            qDebug() << "Expand nnz" << clusterMatrix.nonZeros();
        }

        if(_debugMatrices)
        {
            matrixStream << "Expanded Matrix \n";
            matrixStream << clusterMatrix;
            qDebug().noquote() << QString::fromStdString(matrixStream.str());
            matrixStream.clear();
            matrixStream.str(std::string());
        }

        // Inflate the matrix
        clusterMatrix = blaze::forEach(clusterMatrix, [&inflation](float d)
            { return std::pow(d, inflation); });

        if(_debugMatrices)
        {
            matrixStream << "Inflated Expanded Matrix \n";
            matrixStream << clusterMatrix;
            qDebug().noquote() << QString::fromStdString(matrixStream.str());
            matrixStream.clear();
            matrixStream.str(std::string());
            qDebug() << "Inflated Expanded nnz" << clusterMatrix.nonZeros();
        }

        // Normalise
        normaliseColumnsColumnMajor(clusterMatrix);

        if(_debugMatrices)
        {
            matrixStream << "Normalised Inflated Expanded Matrix \n";
            matrixStream << clusterMatrix;
            qDebug().noquote() << QString::fromStdString(matrixStream.str());
            matrixStream.clear();
            matrixStream.str(std::string());
            qDebug() << "Normalise nnz" << clusterMatrix.nonZeros();
        }

        // Check if matrix is idempotent
        MatrixType sqClusterMatrix = blaze::forEach(clusterMatrix, [](float d) { return d * d; });
        VectorType colSum(nodeCount, 0);

        sumColumns(sqClusterMatrix, colSum);
        for(size_t k=0; k<clusterMatrix.columns(); ++k)
        {
            float max = 0;
            for(MatrixType::ConstIterator it=clusterMatrix.cbegin(k); it!=clusterMatrix.cend(k); ++it )
                max = std::max(it->value(),max);
            if((max - colSum[k]) * clusterMatrix.nonZeros(k) > MCL_CONVERGENCE_LIMIT)
            {
                if(_debugIteration)
                    qDebug() << "No Converge" << (max - colSum[k]) * clusterMatrix.nonZeros(k);
                isEquiDistrubuted = false;
                break;
            }
        }

        iter++;
    } while(!isEquiDistrubuted);

    if(_debugIteration)
        qDebug() << iter << "iterations";

    target.setPhase(QString("MCL Interpreting"));

    // Interpret the matrix
    std::map<size_t, std::set<size_t>> clusterNodesLookup;
    std::vector<size_t> clusterGroups(nodeCount, 0);
    int clusterCount = 0;
    for(size_t k = 0; k<clusterMatrix.columns(); ++k)
    {
        for(auto it = clusterMatrix.cbegin(k); it != clusterMatrix.cend(k); ++it )
        {
            if(it->value() < MCL_PRUNE_LIMIT)
                continue;

            auto rowCluster = clusterGroups[it->index()];
            auto columnCluster = clusterGroups[k];

            // If no cluster exists, make one
            if(rowCluster == 0 && columnCluster == 0)
            {
                clusterCount++;
                clusterGroups[it->index()] = clusterCount;
                clusterGroups[k] = clusterCount;

                std::set<size_t> newClusterNodeIndex;
                newClusterNodeIndex.insert(it->index());
                newClusterNodeIndex.insert(k);
                clusterNodesLookup[clusterCount] = newClusterNodeIndex;
            }
            else if(rowCluster > 0)
            {
                if(columnCluster > 0)
                {
                    if(columnCluster != rowCluster)
                    {
                        // There's overlap with both row and column cluster here
                        // we can preserve overlap in future but for now just
                        // ignore and leave the node in the rowCluster
                    }
                }
                else
                {
                    // Add to row cluster
                    clusterGroups[k] = rowCluster;
                    clusterNodesLookup[rowCluster].insert(k);
                }
            }
            else if(columnCluster > 0)
            {
                // Add to Column Cluster
                clusterGroups[it->index()] = columnCluster;
                clusterNodesLookup[columnCluster].insert(it->index());
            }
        }
    }

    NodeArray<QString> clusters(graph);
    clusterCount = 0;
    for(auto cluster : clusterNodesLookup)
    {
        // Remove singular clusters
        if(cluster.second.size() <= 1)
            continue;

        clusterCount++;
        for(auto mapid : cluster.second)
        {
            clusters[indexToNodeMap.at(mapid)] = QString(QObject::tr("Cluster %1")).arg(QString::number(clusterCount));
        }
    }

    _graphModel->createAttribute(QObject::tr("MCL Cluster"))
        .setDescription("The MCL-calculated cluster in which the node resides.")
        .setStringValueFn([clusters](NodeId nodeId) { return clusters[nodeId]; });
}

std::unique_ptr<GraphTransform> MCLTransformFactory::create(const GraphTransformConfig&) const
{
    auto testTransform = std::make_unique<MCLTransform>(graphModel());

    return std::move(testTransform); //FIXME std::move required because of clang bug
}

