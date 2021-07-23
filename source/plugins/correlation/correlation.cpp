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

#include "correlation.h"

std::unique_ptr<ContinuousCorrelation> ContinuousCorrelation::create(CorrelationType correlationType)
{
    switch(correlationType)
    {
    case CorrelationType::Pearson:              return std::make_unique<PearsonCorrelation>();
    case CorrelationType::SpearmanRank:         return std::make_unique<SpearmanRankCorrelation>();
    case CorrelationType::EuclideanSimilarity:  return std::make_unique<EuclideanSimilarityCorrelation>();
    case CorrelationType::CosineSimilarity:     return std::make_unique<CosineSimilarityCorrelation>();
    case CorrelationType::Bicor:                return std::make_unique<BicorCorrelation>();
    default: break;
    }

    return nullptr;
}

std::unique_ptr<DiscreteCorrelation> DiscreteCorrelation::create(CorrelationType correlationType)
{
    switch(correlationType)
    {
    case CorrelationType::Jaccard:              return std::make_unique<JaccardCorrelation>();
    case CorrelationType::SMC:                  return std::make_unique<SMCCorrelation>();
    default: break;
    }

    return nullptr;
}

double PearsonAlgorithm::evaluate(size_t numColumns, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB)
{
    double productSum = std::inner_product(rowA->begin(), rowA->end(), rowB->begin(), 0.0);
    double numerator = (static_cast<double>(numColumns) * productSum) - (rowA->sum() * rowB->sum());
    double denominator = rowA->variability() * rowB->variability();

    return numerator / denominator;
}

double EuclideanSimilarityAlgorithm::evaluate(size_t numColumns, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB)
{
    double sum = 0.0;

    for(size_t i = 0; i < numColumns; i++)
    {
        auto diff = rowA->valueAt(i) - rowB->valueAt(i);
        auto diffSq = diff * diff;
        sum += diffSq;
    }

    auto sqrtSum = sum != 0.0 ? std::sqrt(sum) : 0.0;

    return 1.0 / (1.0 + sqrtSum);
}

double CosineSimilarityAlgorithm::evaluate(size_t, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB)
{
    double productSum = std::inner_product(rowA->begin(), rowA->end(), rowB->begin(), 0.0);
    double magnitudeProduct = rowA->magnitude() * rowB->magnitude();

    return magnitudeProduct > 0.0 ? productSum / magnitudeProduct : 0.0;
}

void BicorAlgorithm::preprocess(size_t numColumns, const ContinuousDataRows& rows)
{
    _base = &rows.front();
    _processedRows.resize(rows.size());

    for(size_t i = 0; const auto& row : rows)
    {
        auto median = u::medianOf(row.data());

        std::vector<double> absDiffs(numColumns);
        std::vector<double> intermediate(numColumns);

        for(size_t j = 0; auto value : row)
        {
            intermediate[j] = value - median;
            absDiffs[j] = std::abs(intermediate[j]);
            j++;
        }

        auto mad = u::medianOf(absDiffs);

        for(size_t j = 0; auto value : intermediate)
        {
            auto u = value / (9.0 * mad);
            auto v = 1 - (u * u);
            auto newValue = value * (v * v) * (v > 0.0 ? 1.0 : 0.0);

            intermediate[j++] = newValue;
        }

        auto& processedRow = _processedRows.at(i++);
        processedRow = {intermediate, row.nodeId(), row.computeCostHint()};
        processedRow.update();
    }
}

double BicorAlgorithm::evaluate(size_t, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB)
{
    auto a = std::distance(_base, rowA);
    auto b = std::distance(_base, rowB);
    const auto& processedRowA = _processedRows.at(a);
    const auto& processedRowB = _processedRows.at(b);

    double productSum = std::inner_product(processedRowA.begin(), processedRowA.end(),
        processedRowB.begin(), 0.0);

    return productSum / (processedRowA.magnitude() * processedRowB.magnitude());
}
