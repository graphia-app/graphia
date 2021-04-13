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

#ifndef CORRELATION_H
#define CORRELATION_H

#include "correlationdatarow.h"

#include "shared/utils/qmlenum.h"
#include "shared/utils/progressable.h"
#include "shared/utils/cancellable.h"
#include "shared/utils/threadpool.h"
#include "shared/utils/redirects.h"

#include "shared/graph/edgelist.h"

#include <vector>
#include <cmath>

#include <QObject>
#include <QString>

// Note: the ordering of these enums is important from a save
// file point of view; i.e. only append, don't reorder

DEFINE_QML_ENUM(
    Q_GADGET, CorrelationType,
    Pearson,
    SpearmanRank,
    Jaccard,
    SMC,
    EuclideanSimilarity,
    CosineSimilarity);

DEFINE_QML_ENUM(
    Q_GADGET, CorrelationDataType,
    Continuous,
    Discrete);

DEFINE_QML_ENUM(
    Q_GADGET, CorrelationPolarity,
    Positive,
    Negative,
    Both);

class ICorrelation
{
public:
    virtual ~ICorrelation() = default;

    virtual QString name() const = 0;
    virtual QString attributeName() const = 0;
    virtual QString attributeDescription() const = 0;
};

class ContinuousCorrelation : public ICorrelation
{
public:
    virtual EdgeList process(const ContinuousDataRows& rows, double minimumThreshold,
        CorrelationPolarity polarity = CorrelationPolarity::Positive,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    static std::unique_ptr<ContinuousCorrelation> create(CorrelationType correlationType);
};

enum class RowType
{
    Raw,
    Ranking
};

template<typename Algorithm, RowType rowType = RowType::Raw>
class CovarianceCorrelation : public ContinuousCorrelation
{
public:
    EdgeList process(const ContinuousDataRows& rows,
        double minimumThreshold, CorrelationPolarity polarity = CorrelationPolarity::Positive,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const final
    {
        if(rows.empty())
            return {};

        size_t numColumns = rows.front().numColumns();

        if(progressable != nullptr)
            progressable->setProgress(-1);

        uint64_t totalCost = 0;
        for(const auto& row : rows)
        {
            totalCost += row.computeCostHint();

            if constexpr(rowType == RowType::Ranking)
                row.generateRanking();
        }

        std::atomic<uint64_t> cost(0);

        auto results = ThreadPool(QStringLiteral("Correlation")).concurrent_for(rows.begin(), rows.end(),
        [&](ContinuousDataRows::const_iterator rowAIt)
        {
            const auto* rowA = &(*rowAIt);

            if constexpr(rowType == RowType::Ranking)
                rowA = rowA->ranking();

            EdgeList edges;

            if(cancellable != nullptr && cancellable->cancelled())
                return edges;

            for(auto rowBIt = rowAIt + 1; rowBIt != rows.end(); ++rowBIt)
            {
                const auto* rowB = &(*rowBIt);

                if constexpr(rowType == RowType::Ranking)
                    rowB = rowB->ranking();

                double r = Algorithm::evaluate(numColumns, rowA, rowB);

                if(!std::isfinite(r))
                    continue;

                bool createEdge = false;

                switch(polarity)
                {
                default:
                case CorrelationPolarity::Positive: createEdge = (r >= minimumThreshold); break;
                case CorrelationPolarity::Negative: createEdge = (r <= -minimumThreshold); break;
                case CorrelationPolarity::Both:     createEdge = (std::abs(r) >= minimumThreshold); break;
                }

                if(createEdge)
                    edges.push_back({rowA->nodeId(), rowB->nodeId(), r});
            }

            cost += rowA->computeCostHint();

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
};

struct PearsonAlgorithm
{
    static double evaluate(size_t numColumns, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB)
    {
        double productSum = std::inner_product(rowA->begin(), rowA->end(), rowB->begin(), 0.0);
        double numerator = (numColumns * productSum) - (rowA->sum() * rowB->sum());
        double denominator = rowA->variability() * rowB->variability();

        return numerator / denominator;
    }
};

class PearsonCorrelation : public CovarianceCorrelation<PearsonAlgorithm>
{
public:
    QString name() const override { return QObject::tr("Pearson"); }

    QString attributeName() const override { return QObject::tr("Pearson Correlation Value"); }
    QString attributeDescription() const override
    {
        return QObject::tr("The %1 is an indication of "
            "the linear relationship between two variables.")
            .arg(u::redirectLink("pearson", QObject::tr("Pearson Correlation Coefficient")));
    }
};

class SpearmanRankCorrelation : public CovarianceCorrelation<PearsonAlgorithm, RowType::Ranking>
{
public:
    QString name() const override { return QObject::tr("Spearman Rank"); }

    QString attributeName() const override { return QObject::tr("Spearman Rank Correlation Value"); }
    QString attributeDescription() const override
    {
        return QObject::tr("The %1 is an indication of "
            "the monotomic relationship between two variables.")
            .arg(u::redirectLink("spearman", QObject::tr("Spearman Rank Correlation Coefficient")));
    }
};

struct EuclideanSimilarityAlgorithm
{
    static double evaluate(size_t numColumns, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB)
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
};

class EuclideanSimilarityCorrelation : public CovarianceCorrelation<EuclideanSimilarityAlgorithm>
{
public:
    QString name() const override { return QObject::tr("Euclidean Similarity"); }

    QString attributeName() const override { return QObject::tr("Euclidean Similarity"); }
    QString attributeDescription() const override
    {
        return QObject::tr("%1 is essentially the inverse "
            "of the Euclidean distance between two vectors.")
            .arg(u::redirectLink("euclidean", QObject::tr("Euclidean Similarity")));
    }
};

struct CosineSimilarityAlgorithm
{
    static double evaluate(size_t, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB)
    {
        double productSum = std::inner_product(rowA->begin(), rowA->end(), rowB->begin(), 0.0);
        double magnitudeProduct = rowA->magnitude() * rowB->magnitude();

        return magnitudeProduct > 0.0 ? productSum / magnitudeProduct : 0.0;
    }
};

class CosineSimilarityCorrelation : public CovarianceCorrelation<CosineSimilarityAlgorithm>
{
public:
    QString name() const override { return QObject::tr("Cosine Similarity"); }

    QString attributeName() const override { return QObject::tr("Cosine Similarity"); }
    QString attributeDescription() const override
    {
        return QObject::tr("%1 is a measure of similarity between two "
            "non-zero vectors of an inner product space.")
            .arg(u::redirectLink("cosine", QObject::tr("Cosine Similarity")));
    }
};

class DiscreteCorrelation : public ICorrelation
{
public:
    virtual EdgeList process(const DiscreteDataRows& rows, double minimumThreshold, bool treatAsBinary,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    static std::unique_ptr<DiscreteCorrelation> create(CorrelationType correlationType);
};

template<int Denominator>
class MatchingCorrelation : public DiscreteCorrelation
{
public:
    EdgeList process(const DiscreteDataRows& rows, double minimumThreshold, bool treatAsBinary,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const final
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
                            fraction += {0, Denominator};
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
};

class JaccardCorrelation : public MatchingCorrelation<0>
{
public:
    QString name() const override { return QObject::tr("Jaccard"); }

    QString attributeName() const override { return QObject::tr("Jaccard Correlation Value"); }
    QString attributeDescription() const override
    {
        return QObject::tr("The %1 is a statistic used for gauging "
            "the similarity and diversity of sample sets.")
            .arg(u::redirectLink("jaccard", QObject::tr("Jaccard Index")));
    }
};

class SMCCorrelation : public MatchingCorrelation<1>
{
public:
    QString name() const override { return QObject::tr("Simple Matching Coefficient"); }

    QString attributeName() const override { return QObject::tr("Simple Matching Coefficient"); }
    QString attributeDescription() const override
    {
        return QObject::tr("The %1 is a statistic used for gauging "
            "the similarity and diversity of sample sets.")
            .arg(u::redirectLink("smc", QObject::tr("Simple Matching Coefficient (SMC)")));
    }
};

#endif // CORRELATION_H
