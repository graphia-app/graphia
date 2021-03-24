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

bool DiscreteCorrelation::isTrue(const QString& value)
{
    if(value.isEmpty())
        return false;

    if(value == "0")
        return false;

    if(QString::compare(value, "false", Qt::CaseInsensitive) == 0)
        return false;

    return true;
}

EdgeList JaccardCorrelation::process(const DiscreteDataRows& rows, double minimumThreshold,
    bool treatAsBinary, Cancellable* cancellable, Progressable* progressable) const
{
    if(rows.empty())
        return {};

    size_t numColumns = rows.front().numColumns();

    if(progressable != nullptr)
        progressable->setProgress(-1);

    uint64_t totalCost = 0;
    for(const auto& row : rows)
        totalCost += row.computeCostHint();

    std::atomic<uint64_t> cost(0);

    auto results = ThreadPool(QStringLiteral("Correlation")).concurrent_for(rows.begin(), rows.end(),
    [&](DiscreteDataRows::const_iterator rowAIt)
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

        auto binary = [&](size_t column, const auto rowA, const auto rowB) -> Fraction
        {
            auto rowAValue = DiscreteCorrelation::isTrue(rowA->valueAt(column));
            auto rowBValue = DiscreteCorrelation::isTrue(rowB->valueAt(column));

            if(!rowAValue && !rowBValue)
                return {0, 0};

            return {rowAValue == rowBValue ? 1 : 0, 1};
        };

        auto nonBinary = [&](size_t column, const auto rowA, const auto rowB) -> Fraction
        {
            const auto& rowAValue = rowA->valueAt(column);
            const auto& rowBValue = rowB->valueAt(column);

            if(rowAValue.isEmpty() && rowBValue.isEmpty())
                return {0, 0};

            return {rowAValue == rowBValue ? 1 : 0, 1};
        };

        auto createEdgesForRowPairs = [&](auto&& f)
        {
            for(auto rowBIt = rowAIt + 1; rowBIt != rows.end(); ++rowBIt)
            {
                Fraction fraction;
                for(size_t column = 0; column < numColumns; column++)
                    fraction += f(column, rowAIt, rowBIt);

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
