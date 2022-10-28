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

#ifndef CORRELATION_H
#define CORRELATION_H

#include "correlationdatavector.h"
#include "correlationtype.h"

#include "shared/utils/progressable.h"
#include "shared/utils/cancellable.h"
#include "shared/utils/threadpool.h"
#include "shared/utils/redirects.h"
#include "shared/utils/is_detected.h"

#include "shared/graph/edgelist.h"
#include "shared/graph/covariancematrix.h"

#include <vector>
#include <iterator>
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
    virtual EdgeList edgeList(const ContinuousDataVectors& vectors, double threshold,
        CorrelationPolarity polarity = CorrelationPolarity::Positive,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    virtual CovarianceMatrix matrix(const ContinuousDataVectors& vectors,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    static std::unique_ptr<ContinuousCorrelation> create(CorrelationType correlationType);
};

enum class VectorType
{
    Raw,
    Ranking
};

template<typename Algorithm, VectorType vectorType = VectorType::Raw>
class CovarianceCorrelation : public ContinuousCorrelation
{
    template<typename A>
    using preprocess_t = decltype(std::declval<A>().preprocess(0, ContinuousDataVectors{}));

private:
    struct ContinuousDataVectorRelation
    {
        ContinuousDataVectors::const_iterator _a;
        ContinuousDataVectors::const_iterator _b;
        double _r = 0.0;
    };

    using CorrelationList = std::vector<ContinuousDataVectorRelation>;

    auto process(const ContinuousDataVectors& vectors,
        double threshold, CorrelationPolarity polarity = CorrelationPolarity::Positive,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const
    {
        size_t size = vectors.front().size();

        if(progressable != nullptr)
            progressable->setProgress(-1);

        uint64_t totalCost = 0;
        for(const auto& vector : vectors)
        {
            totalCost += vector.computeCostHint();

            if constexpr(vectorType == VectorType::Ranking)
                vector.generateRanking();
        }

        Algorithm algorithm;

        constexpr bool AlgorithmHasPreprocess =
            std::experimental::is_detected_v<preprocess_t, Algorithm>;

        if constexpr(AlgorithmHasPreprocess)
            algorithm.preprocess(size, vectors);

        std::atomic<uint64_t> cost(0);

        auto results = ThreadPool(QStringLiteral("Correlation")).parallel_for(vectors.begin(), vectors.end(),
        [&](ContinuousDataVectors::const_iterator vectorAIt)
        {
            const auto* vectorA = &(*vectorAIt);

            if constexpr(vectorType == VectorType::Ranking)
                vectorA = vectorA->ranking();

            CorrelationList correlations;

            if(cancellable != nullptr && cancellable->cancelled())
                return correlations;

            for(auto vectorBIt = vectorAIt + 1; vectorBIt != vectors.end(); ++vectorBIt)
            {
                const auto* vectorB = &(*vectorBIt);

                if constexpr(vectorType == VectorType::Ranking)
                    vectorB = vectorB->ranking();

                double r = algorithm.evaluate(size, vectorA, vectorB);

                if(!std::isfinite(r))
                    continue;

                if(correlationExceedsThreshold(polarity, r, threshold))
                    correlations.push_back({vectorAIt, vectorBIt, r});
            }

            cost += vectorA->computeCostHint();

            if(progressable != nullptr)
                progressable->setProgress(static_cast<int>((cost * 100) / totalCost));

            return correlations;
        });

        if(progressable != nullptr)
        {
            // Returning the results might take time
            progressable->setProgress(-1);
        }

        return results;
    }

public:
    EdgeList edgeList(const ContinuousDataVectors& vectors,
        double threshold, CorrelationPolarity polarity = CorrelationPolarity::Positive,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const final
    {
        if(vectors.empty())
            return {};

        auto results = process(vectors, threshold, polarity, cancellable, progressable);

        if(cancellable != nullptr && cancellable->cancelled())
            return {};

        EdgeList edges;
        edges.reserve(std::distance(results.begin(), results.end()));

        std::transform(results.begin(), results.end(), std::back_inserter(edges),
        [](const auto& result)
        {
            return EdgeListEdge{result._a->nodeId(), result._b->nodeId(), result._r};
        });

        return edges;
    }

    CovarianceMatrix matrix(const ContinuousDataVectors& vectors,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const final
    {
        if(vectors.empty())
            return {};

        auto results = process(vectors, 0.0, CorrelationPolarity::Both, cancellable, progressable);

        if(cancellable != nullptr && cancellable->cancelled())
            return {};

        CovarianceMatrix matrix(vectors.size());

        for(const auto& result : results)
        {
            auto a = std::distance(vectors.begin(), result._a);
            auto b = std::distance(vectors.begin(), result._b);
            matrix.setValueAt(a, b, result._r);
        }

        return matrix;
    }
};

struct PearsonAlgorithm
{
    double evaluate(size_t size, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB);
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

class SpearmanRankCorrelation : public CovarianceCorrelation<PearsonAlgorithm, VectorType::Ranking>
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
    double evaluate(size_t, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB);
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
    double evaluate(size_t, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB);
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
    const ContinuousDataVector* _base = nullptr;
    ContinuousDataVectors _processedVectors;

    void preprocess(size_t size, const ContinuousDataVectors& vectors);
    double evaluate(size_t, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB);
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

    QString attributeName() const override { return QObject::tr("Bicor Correlation Value"); }
    QString attributeDescription() const override
    {
        return QObject::tr("%1 is a median-based measure of similarity between samples.")
            .arg(u::redirectLink("bicor", QObject::tr("Bicor")));
    }
};

class DiscreteCorrelation : public ICorrelation
{
public:
    virtual EdgeList edgeList(const DiscreteDataVectors& vectors, double threshold, bool treatAsBinary,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    static std::unique_ptr<DiscreteCorrelation> create(CorrelationType correlationType);
};

template<int Denominator>
class MatchingCorrelation : public DiscreteCorrelation
{
public:
    EdgeList edgeList(const DiscreteDataVectors& vectors, double threshold, bool treatAsBinary,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const final
    {
        if(vectors.empty())
            return {};

        size_t size = vectors.front().size();

        if(progressable != nullptr)
            progressable->setProgress(-1);

        const auto tokenisedVectors = tokeniseDataVectors(vectors);

        uint64_t totalCost = 0;
        for(const auto& vector : vectors)
            totalCost += vector.computeCostHint();

        std::atomic<uint64_t> cost(0);

        auto results = ThreadPool(QStringLiteral("Correlation")).parallel_for(tokenisedVectors.begin(), tokenisedVectors.end(),
        [&](TokenisedDataVectors::const_iterator vectorAIt)
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

            auto binary = [&](auto vectorAValue, auto vectorBValue) -> Fraction
            {
                return {vectorAValue && vectorBValue ? 1 : 0, 1};
            };

            auto nonBinary = [&](auto vectorAValue, auto vectorBValue) -> Fraction
            {
                return {vectorAValue == vectorBValue ? 1 : 0, 1};
            };

            auto createEdgesForVectorPairs = [&](auto&& f)
            {
                for(auto vectorBIt = vectorAIt + 1; vectorBIt != tokenisedVectors.end(); ++vectorBIt)
                {
                    Fraction fraction;
                    for(size_t i = 0; i < size; i++)
                    {
                        const auto& vectorAValue = vectorAIt->valueAt(i);
                        const auto& vectorBValue = vectorBIt->valueAt(i);

                        if(!vectorAValue && !vectorBValue)
                            fraction += {0, Denominator};
                        else
                            fraction += f(vectorAValue, vectorBValue);
                    }

                    double r = fraction;

                    if(std::isfinite(r) && r >= threshold)
                        edges.push_back({vectorAIt->nodeId(), vectorBIt->nodeId(), r});
                }
            };

            if(treatAsBinary)
                createEdgesForVectorPairs(binary);
            else
                createEdgesForVectorPairs(nonBinary);

            cost += vectorAIt->computeCostHint();

            if(progressable != nullptr)
                progressable->setProgress(static_cast<int>((cost * 100) / totalCost));

            return edges;
        });

        if(progressable != nullptr)
        {
            // Returning the results might take time
            progressable->setProgress(-1);
        }

        if(cancellable != nullptr && cancellable->cancelled())
            return {};

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
            "to Jaccard, except that it counts mutual absence.");
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
