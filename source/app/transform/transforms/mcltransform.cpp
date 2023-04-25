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

#include "mcltransform.h"
#include "transform/transformedgraph.h"
#include "graph/graphmodel.h"
#include "shared/utils/threadpool.h"

#include <blaze/Blaze.h>

#include <QElapsedTimer>
#include <QDebug>

#include <map>
#include <set>
#include <thread>
#include <algorithm>

using MatrixType = blaze::CompressedMatrix<float, blaze::columnMajor>;
using VectorType = blaze::DynamicVector<float, blaze::columnVector>;

static void normaliseColumnsColumnMajor(MatrixType &mclMatrix)
{
    for(size_t column = 0; column<mclMatrix.columns(); column++)
    {
        MatrixType::Iterator lelem = mclMatrix.begin(column);
        MatrixType::Iterator lend = mclMatrix.end(column);

        if(lelem == lend)
            continue;

        float value = 0.0f;
        for(lelem = mclMatrix.begin(column); lelem != lend; ++lelem)
            value += lelem->value();

        Q_ASSERT(value > 0.0f);
        value = 1.0f / value;

        for(lelem = mclMatrix.begin(column); lelem != lend; ++lelem)
            lelem->value() = lelem->value() * value;
    }
}

static void sumColumns(MatrixType &mclMatrix, VectorType &output)
{
    for(size_t column = 0; column<mclMatrix.columns(); column++)
    {
        MatrixType::ConstIterator lend = mclMatrix.cend(column);
        for(MatrixType::ConstIterator lelem = mclMatrix.cbegin(column); lelem != lend; ++lelem)
        {
            output[column] = output[column] + lelem->value();
        }
    }
}

struct SparseMatrixEntry
{
    size_t _row; size_t _column; float _value;
    SparseMatrixEntry(size_t row, size_t column, float value)
        : _row(row), _column(column), _value(value){}
};

struct MCLRowData
{
    std::vector<float> values;
    std::vector<bool> valid;
    std::vector<size_t> indices;
    explicit MCLRowData(size_t columnCount) : values(columnCount, 0.0f),
        valid(columnCount, false), indices(columnCount, 0UL) {}
};

template<typename CancelledFn>
static void expandAndPruneRow(MatrixType& mclMatrix, size_t columnId,
    std::vector<SparseMatrixEntry>* matrixStorage, MCLRowData& rowData,
    float minValueCutoff, const CancelledFn& cancelledFn)
{
    const bool DEBUG = false;

    // Perhaps make these changable parameters later
    const int RECOVERY_COUNT = 1400;
    const int SELECTION_COUNT = 1100;

    size_t nonzeros = 0;

    size_t minIndex = std::numeric_limits<size_t>::infinity();
    size_t maxIndex = 0UL;
    const auto* const lend = mclMatrix.cend(columnId);

    // Perform multiply and populate prune dependant data structures
    for(const auto* lelem = mclMatrix.cbegin(columnId); lelem != lend; ++lelem)
    {
        if(cancelledFn())
            break;

        // For each column (left)
        const auto* const rend = mclMatrix.cend(lelem->index());
        for(const auto* relem = mclMatrix.cbegin(lelem->index()); relem != rend; ++relem)
        {
            // For each column starting at left column index (right)
            const float mult = lelem->value() * relem->value();

            if(!rowData.valid[relem->index()])
            {
                rowData.values[relem->index()] = mult;
                rowData.valid[relem->index()] = true;
                // Position in the rowData.values vector
                rowData.indices[nonzeros] = relem->index();
                ++nonzeros;

                if(relem->index() < minIndex) minIndex = relem->index();
                if(relem->index() > maxIndex) maxIndex = relem->index();
            }
            else
                rowData.values[relem->index()] += mult;
        }
    }

    if(cancelledFn())
        return;

    if(nonzeros > 0UL)
    {
        Q_ASSERT(minIndex <= maxIndex);

        size_t remainCount = nonzeros;
        float rowPruneSum = 0.0f;
        // Mass is always normalised!
        const float targetMass = 0.9f;

        for(size_t i = 0UL; i < nonzeros; ++i)
        {
            const size_t index = rowData.indices[i];
            if(std::abs(rowData.values[index]) <= minValueCutoff)
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

        auto first = rowData.indices.begin();
        auto last = rowData.indices.begin() + static_cast<std::ptrdiff_t>(nonzeros);

        if(remainCount != nonzeros && rowPruneSum < targetMass && remainCount < RECOVERY_COUNT)
        {
            // Recover
            if(DEBUG)
                qDebug() << "RECOVERY" << "MASS:" << rowPruneSum;
            std::nth_element(first, rowData.indices.begin() + RECOVERY_COUNT, last,
                [&rowData](size_t i1, size_t i2) { return rowData.values[i1] > rowData.values[i2]; });

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
            std::nth_element(first, rowData.indices.begin() + SELECTION_COUNT, last,
                [&rowData](size_t i1, size_t i2) { return rowData.values[i1] > rowData.values[i2]; });

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

            Q_ASSERT(remainCount < RECOVERY_COUNT);

            // Do Another recovery if needed
            if(remainCount != nonzeros && rowPruneSum < targetMass)
            {
                if(DEBUG)
                    qDebug() << "RECOVERY 2" << "MASS:" << rowPruneSum;

                std::nth_element(first, rowData.indices.begin() + RECOVERY_COUNT, last,
                    [&rowData](size_t i1, size_t i2) { return rowData.values[i1] > rowData.values[i2]; });
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
                    rowData.valid[rowData.indices[i]] = false;
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
        if((2 * nonzeros) < (maxIndex - minIndex))
        {
            std::sort(first, last);

            for(size_t j = 0UL; j<nonzeros; ++j)
            {
                const size_t index = rowData.indices[j];
                if(rowData.values[index] > EPSILON)
                {
                    matrixStorage->emplace_back(index, columnId, rowData.values[index]);
                    rowData.values[index] = 0.0f;
                }
                rowData.valid[index] = false;
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
                rowData.valid[j] = false;
            }
        }
    }
}

void MCLTransform::apply(TransformedGraph& target)
{
    auto granularity = std::get<double>(
                config().parameterByName(QStringLiteral("Granularity"))->_value);

    if(_debugIteration)
    {
        QElapsedTimer mclTimer;
        mclTimer.start();
        calculateMCL(static_cast<float>(granularity), target);
        qDebug() << "MCL Elapsed Time" << mclTimer.elapsed();
    }
    else
        calculateMCL(static_cast<float>(granularity), target);
}

template<class MatrixType>
class ColumnsIterator
{
public:
    class iterator
    {
    public:
        using value_type = size_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::input_iterator_tag;
        using difference_type = size_t;

    private:
        size_t _num = 0;

    public:
        explicit iterator(size_t num = 0) : _num(num) {}
        iterator& operator++() { _num = _num + 1; return *this; }
        iterator operator++(int) { iterator retval = *this; ++(*this); return retval; }
        bool operator==(iterator other) const { return _num == other._num; }
        bool operator!=(iterator other) const { return !(*this == other); }
        value_type operator*() const { return _num; }
    };
    MatrixType& matrix;
    explicit ColumnsIterator(MatrixType& _matrix) : matrix(_matrix) {}
    iterator begin() { return iterator(0); }
    iterator end() { return iterator(matrix.columns()); }
};

void MCLTransform::calculateMCL(float inflation, TransformedGraph& target)
{
    setPhase(QStringLiteral("MCL Initialising"));

    auto nodeCount = target.numNodes();

    // Map NodeIds to Matrix index
    NodeIdMap<size_t> nodeToIndexMap;
    std::map<size_t, NodeId> indexToNodeMap;
    for(const NodeId nodeId : target.nodeIds())
    {
        auto index = nodeToIndexMap.size();
        nodeToIndexMap[nodeId] = index;
        indexToNodeMap[index] = nodeId;
    }

    MatrixType clusterMatrix(nodeCount, nodeCount);
    blaze::setNumThreads(std::thread::hardware_concurrency());

    clusterMatrix.reserve((target.numEdges() * 2) + nodeCount);

    // Populate the Matrix
    for(const NodeId nodeId : target.nodeIds())
    {
        auto nodeIndex = nodeToIndexMap[nodeId];
        auto connectedEdgeIds = target.edgeIdsForNodeId(nodeId);

        // Add all connected node indexes to sorted set
        std::set<size_t> sortNodeIndexes;
        for(auto connectEdgeId : connectedEdgeIds)
        {
            auto connectedNodeId = target.edgeById(connectEdgeId).oppositeId(nodeId);
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

    clusterMatrix.erase([this](float value){ return value < MCL_PRUNE_LIMIT; } );

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
        if(cancelled())
            return;

        setPhase(QStringLiteral("MCL Iteration %1").arg(QString::number(iter + 1)));
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

        std::atomic<uint64_t> iteration(0);
        const auto totalIterations = clusterMatrix.columns();
        setProgress(0);

        auto cancelledFn = [this] { return cancelled(); };

        // Capture rowData by value; this gives each thread a copy of rowData, rather re-allocating vectors per row (slow)
        parallel_for(colIterator.begin(), colIterator.end(),
        [&, rowData](size_t iterator) mutable
        {
            expandAndPruneRow(clusterMatrix, iterator, &matrixStorage[iterator],
                rowData, MCL_PRUNE_LIMIT, cancelledFn);

            setProgress(static_cast<int>((iteration++ * 100) / totalIterations));
        });

        setProgress(-1);

        if(cancelled())
            return;

        size_t newNNZCount = 0;
        for(const auto& column : matrixStorage)
            newNNZCount += column.size();

        MatrixType dstMatrix(clusterMatrix.rows(), clusterMatrix.rows());
        dstMatrix.reserve(newNNZCount);
        size_t row = 0;
        for(const auto& matrixRow : matrixStorage)
        {
            for(auto matrixEntry: matrixRow)
                dstMatrix.append(matrixEntry._row, matrixEntry._column, matrixEntry._value);

            dstMatrix.finalize(row);
            row++;
        }
        clusterMatrix = dstMatrix;

        if(_debugIteration)
        {
            auto populationTime = threadedTimer.restart();
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
            float max = 0.0f;
            for(MatrixType::ConstIterator it=clusterMatrix.cbegin(k); it!=clusterMatrix.cend(k); ++it )
                max = std::max(it->value(),max);

            if((max - colSum[k]) * static_cast<float>(clusterMatrix.nonZeros(k)) > MCL_CONVERGENCE_LIMIT)
            {
                if(_debugIteration)
                    qDebug() << "No Converge" << (max - colSum[k]) * static_cast<float>(clusterMatrix.nonZeros(k));
                isEquiDistrubuted = false;
                break;
            }
        }

        iter++;
    } while(!isEquiDistrubuted);

    if(_debugIteration)
        qDebug() << iter << "iterations";

    setPhase(QStringLiteral("MCL Interpreting"));

    // Interpret the matrix
    std::vector<std::set<size_t>> clusters;
    std::vector<size_t> clusterGroups(nodeCount, 0);
    std::vector<bool> clusterGroupAssigned(nodeCount, false);
    for(size_t k = 0; k < clusterMatrix.columns(); ++k)
    {
        for(const auto* it = clusterMatrix.cbegin(k); it != clusterMatrix.cend(k); ++it )
        {
            if(it->value() < MCL_PRUNE_LIMIT)
                continue;

            auto rowCluster = clusterGroups[it->index()];
            auto columnCluster = clusterGroups[k];
            auto rowClusterAssigned = clusterGroupAssigned[it->index()];
            auto columnClusterAssigned = clusterGroupAssigned[k];

            // If no cluster exists, make one
            if(!rowClusterAssigned && !columnClusterAssigned)
            {
                std::set<size_t> newClusterNodeIndex;
                newClusterNodeIndex.insert(it->index());
                newClusterNodeIndex.insert(k);
                clusters.emplace_back(std::move(newClusterNodeIndex));

                auto index = clusters.size() - 1;
                clusterGroups[it->index()] = index;
                clusterGroups[k] = index;
                clusterGroupAssigned[it->index()] = true;
                clusterGroupAssigned[k] = true;
            }
            else if(rowClusterAssigned)
            {
                if(columnClusterAssigned)
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
                    clusterGroupAssigned[k] = true;
                    clusters[rowCluster].insert(k);
                }
            }
            else if(columnClusterAssigned)
            {
                // Add to Column Cluster
                clusterGroups[it->index()] = columnCluster;
                clusterGroupAssigned[it->index()] = true;
                clusters[columnCluster].insert(it->index());
            }
        }
    }

    // Sort clusters descending by size
    std::sort(clusters.begin(), clusters.end(),
    [](const auto& a, const auto& b)
    {
        if(a.size() == b.size())
            return *a.begin() > *b.begin();

        return a.size() > b.size();
    });

    NodeArray<QString> clusterNames(target);
    NodeArray<int> clusterSizes(target);
    int clusterNumber = 1;
    for(const auto& cluster : clusters)
    {
        for(auto index : cluster)
        {
            auto nodeId = indexToNodeMap.at(index);
            auto clusterName = QString(QObject::tr("Cluster %1")).arg(QString::number(clusterNumber));

            clusterNames[nodeId] = clusterName;
            clusterSizes[nodeId] = static_cast<int>(cluster.size());
        }

        clusterNumber++;
    }

    _graphModel->createAttribute(QObject::tr("MCL Cluster"))
        .setDescription(QObject::tr("The MCL cluster in which the node resides."))
        .setStringValueFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId]; })
        .setValueMissingFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId].isEmpty(); })
        .setFlag(AttributeFlag::FindShared)
        .setFlag(AttributeFlag::Searchable);

    _graphModel->createAttribute(QObject::tr("MCL Cluster Size"))
        .setDescription(QObject::tr("The size of the MCL cluster in which the node resides."))
        .setIntValueFn([clusterSizes](NodeId nodeId) { return clusterSizes[nodeId]; })
        .setFlag(AttributeFlag::AutoRange);
}

std::unique_ptr<GraphTransform> MCLTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<MCLTransform>(graphModel());
}

