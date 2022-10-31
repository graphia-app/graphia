/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

double PearsonAlgorithm::evaluate(size_t size, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB) const
{
    double productSum = std::inner_product(vectorA->begin(), vectorA->end(), vectorB->begin(), 0.0);
    double numerator = (static_cast<double>(size) * productSum) - (vectorA->sum() * vectorB->sum());
    double denominator = vectorA->variability() * vectorB->variability();

    return numerator / denominator;
}

double EuclideanSimilarityAlgorithm::evaluate(size_t size, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB) const
{
    double sum = 0.0;

    for(size_t i = 0; i < size; i++)
    {
        auto diff = vectorA->valueAt(i) - vectorB->valueAt(i);
        auto diffSq = diff * diff;
        sum += diffSq;
    }

    auto sqrtSum = sum != 0.0 ? std::sqrt(sum) : 0.0;

    return 1.0 / (1.0 + sqrtSum);
}

double CosineSimilarityAlgorithm::evaluate(size_t, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB) const
{
    double productSum = std::inner_product(vectorA->begin(), vectorA->end(), vectorB->begin(), 0.0);
    double magnitudeProduct = vectorA->magnitude() * vectorB->magnitude();

    return magnitudeProduct > 0.0 ? productSum / magnitudeProduct : 0.0;
}

void BicorAlgorithm::preprocess(size_t size, const ContinuousDataVectors& vectors)
{
    _base = &vectors.front();
    _processedVectors.resize(vectors.size());

    for(size_t i = 0; const auto& vector : vectors)
    {
        auto median = u::medianOf(vector.data());

        std::vector<double> absDiffs(size);
        std::vector<double> intermediate(size);

        for(size_t j = 0; auto value : vector)
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

        auto& processedVector = _processedVectors.at(i++);
        processedVector = {intermediate, vector.nodeId(), vector.computeCostHint()};
        processedVector.update();
    }
}

double BicorAlgorithm::evaluate(size_t, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB) const
{
    auto a = static_cast<size_t>(std::distance(_base, vectorA));
    auto b = static_cast<size_t>(std::distance(_base, vectorB));
    const auto& processedVectorA = _processedVectors.at(a);
    const auto& processedVectorB = _processedVectors.at(b);

    double productSum = std::inner_product(processedVectorA.begin(), processedVectorA.end(),
        processedVectorB.begin(), 0.0);

    return productSum / (processedVectorA.magnitude() * processedVectorB.magnitude());
}
