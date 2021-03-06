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
