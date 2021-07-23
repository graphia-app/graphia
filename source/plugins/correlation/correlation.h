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

#ifndef CORRELATION_H
#define CORRELATION_H

#include "correlationdatarow.h"
#include "correlationtype.h"

#include "shared/utils/progressable.h"
#include "shared/utils/cancellable.h"
#include "shared/utils/threadpool.h"
#include "shared/utils/redirects.h"
#include "shared/utils/is_detected.h"

#include "shared/graph/edgelist.h"

#include <vector>
#include <cmath>

#include <QObject>
#include <QString>

class ICorrelation
{
public:
    virtual ~ICorrelation() = default;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
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
    template<typename A>
    using preprocess_t = decltype(std::declval<A>().preprocess(0, ContinuousDataRows{}));

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

        Algorithm algorithm;

        constexpr bool AlgorithmHasPreprocess =
            std::experimental::is_detected_v<preprocess_t, Algorithm>;

        if constexpr(AlgorithmHasPreprocess)
            algorithm.preprocess(numColumns, rows);

        std::atomic<uint64_t> cost(0);

        auto results = ThreadPool(QStringLiteral("Correlation")).parallel_for(rows.begin(), rows.end(),
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

                double r = algorithm.evaluate(numColumns, rowA, rowB);

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
    double evaluate(size_t numColumns, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB);
};

class PearsonCorrelation : public CovarianceCorrelation<PearsonAlgorithm>
{
public:
    QString name() const override { return QObject::tr("Pearson"); }
    QString description() const override
    {
        return QObject::tr("The Pearson Correlation Coefficient is an indication "
            "of the linear correlation between two variables.");
    }

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
    QString description() const override
    {
        return QObject::tr("Spearman's rank correlation coefficient is a "
            "nonparametric measure of the statistical dependence between "
            "the rankings of two variables. It assesses how well the "
            "relationship between two variables can be described using a "
            "monotonic function.");
    }

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
    double evaluate(size_t numColumns, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB);
};

class EuclideanSimilarityCorrelation : public CovarianceCorrelation<EuclideanSimilarityAlgorithm>
{
public:
    QString name() const override { return QObject::tr("Euclidean Similarity"); }
    QString description() const override
    {
        return QObject::tr("Euclidean Similarity is essentially the inverse "
            "of the Euclidean distance between two vectors.");
    }

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
    double evaluate(size_t, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB);
};

class CosineSimilarityCorrelation : public CovarianceCorrelation<CosineSimilarityAlgorithm>
{
public:
    QString name() const override { return QObject::tr("Cosine Similarity"); }
    QString description() const override
    {
        return QObject::tr("Cosine Similarity is a measure of similarity between two "
            "non-zero vectors of an inner product space. It is defined to equal "
            "the cosine of the angle between them, which is also the same as the "
            "inner product of the same vectors normalized to both have length 1.");
    }

    QString attributeName() const override { return QObject::tr("Cosine Similarity"); }
    QString attributeDescription() const override
    {
        return QObject::tr("%1 is a measure of similarity between two "
            "non-zero vectors of an inner product space.")
            .arg(u::redirectLink("cosine", QObject::tr("Cosine Similarity")));
    }
};

struct BicorAlgorithm
{
    const ContinuousDataRow* _base = nullptr;
    ContinuousDataRows _processedRows;

    void preprocess(size_t numColumns, const ContinuousDataRows& rows);
    double evaluate(size_t, const ContinuousDataRow* rowA, const ContinuousDataRow* rowB);
};

class BicorCorrelation : public CovarianceCorrelation<BicorAlgorithm>
{
public:
    QString name() const override { return QObject::tr("Bicor"); }
    QString description() const override
    {
        return QObject::tr("Biweight Midcorrelation (also called Bicor) is a "
            "measure of similarity between samples. It is median-based, rather "
            "than mean-based, thus is less sensitive to outliers.");
    }

    QString attributeName() const override { return QObject::tr("Bicor"); }
    QString attributeDescription() const override
    {
        return QObject::tr("%1 is a median-based measure of similarity between samples.")
            .arg(u::redirectLink("bicor", QObject::tr("Bicor")));
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

        auto results = ThreadPool(QStringLiteral("Correlation")).parallel_for(tokenisedRows.begin(), tokenisedRows.end(),
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

                // NOLINTNEXTLINE google-explicit-constructor
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
    QString description() const override
    {
        return QObject::tr("The Jaccard Index is a statistic used for "
            "gauging the similarity and diversity of sample sets.");
    }

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
    QString description() const override
    {
        return QObject::tr("The Simple Matching Coefficient is a statistic used for "
            "gauging the similarity and diversity of sample sets. It is identical "
            "to Jaccard, except that it counts mutual abscence.");
    }

    QString attributeName() const override { return QObject::tr("Simple Matching Coefficient"); }
    QString attributeDescription() const override
    {
        return QObject::tr("The %1 is a statistic used for gauging "
            "the similarity and diversity of sample sets.")
            .arg(u::redirectLink("smc", QObject::tr("Simple Matching Coefficient (SMC)")));
    }
};

#endif // CORRELATION_H
