/* Copyright © 2013-2020 Graphia Technologies Ltd.
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
    case CorrelationType::Pearson:      return std::make_unique<PearsonCorrelation>();
    case CorrelationType::SpearmanRank: return std::make_unique<SpearmanRankCorrelation>();
    default: break;
    }

    return nullptr;
}

std::unique_ptr<DiscreteCorrelation> DiscreteCorrelation::create(CorrelationType correlationType)
{
    switch(correlationType)
    {
    case CorrelationType::Jaccard:      return std::make_unique<JaccardCorrelation>();
    default: break;
    }

    return nullptr;
}

EdgeList JaccardCorrelation::process(const DiscreteDataRows& rows, double minimumThreshold,
    bool treatAsBinary, Cancellable* cancellable, Progressable* progressable) const
{
    if(rows.empty())
        return {};

    size_t numColumns = rows.front().numColumns();

    if(progressable != nullptr)
        progressable->setProgress(-1);

    const auto tokenisedRows = tokeniseDataRows(rows);

    uint64_t totalCost = 0;
    for(const auto& row : rows)
        totalCost += row.computeCostHint();

    std::atomic<uint64_t> cost(0);

    auto results = ThreadPool(QStringLiteral("Correlation")).concurrent_for(tokenisedRows.begin(), tokenisedRows.end(),
    [&](TokenisedDataRows::const_iterator rowAIt)
    {
        EdgeList edges;

        if(cancellable != nullptr && cancellable->cancelled())
            return edges;

        struct Fraction
        {
            int _numerator = 0;
            int _denominator = 0;

            Fraction& operator+=(const Fraction& other)
            {
                _numerator += other._numerator;
                _denominator += other._denominator;

                return *this;
            }

            operator double() const { return static_cast<double>(_numerator) / _denominator; }
        };

        auto binary = [&](auto rowAValue, auto rowBValue) -> Fraction
        {
            return {rowAValue && rowBValue ? 1 : 0, 1};
        };

        auto nonBinary = [&](auto rowAValue, auto rowBValue) -> Fraction
        {
            return {rowAValue == rowBValue ? 1 : 0, 1};
        };

        auto createEdgesForRowPairs = [&](auto&& f)
        {
            for(auto rowBIt = rowAIt + 1; rowBIt != tokenisedRows.end(); ++rowBIt)
            {
                Fraction fraction;
                for(size_t column = 0; column < numColumns; column++)
                {
                    const auto& rowAValue = rowAIt->valueAt(column);
                    const auto& rowBValue = rowBIt->valueAt(column);

                    if(!rowAValue && !rowBValue)
                        fraction += {0, 0};
                    else
                        fraction += f(rowAValue, rowBValue);
                }

                double r = fraction;

                if(std::isfinite(r) && r >= minimumThreshold)
                    edges.push_back({rowAIt->nodeId(), rowBIt->nodeId(), r});
            }
        };

        if(treatAsBinary)
            createEdgesForRowPairs(binary);
        else
            createEdgesForRowPairs(nonBinary);

        cost += rowAIt->computeCostHint();

        if(progressable != nullptr)
            progressable->setProgress(static_cast<int>((cost * 100) / totalCost));

        return edges;
    });

    if(progressable != nullptr)
    {
        // Returning the results might take time
        progressable->setProgress(-1);
    }

    EdgeList edges;
    edges.reserve(std::distance(results.begin(), results.end()));
    edges.insert(edges.end(), std::make_move_iterator(results.begin()),
        std::make_move_iterator(results.end()));

    return edges;
}
