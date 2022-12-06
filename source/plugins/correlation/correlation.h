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
#include "knnprotograph.h"

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
#include <QVariantMap>

class ICorrelationInfo
{
public:
    virtual ~ICorrelationInfo() = default;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString attributeName() const = 0;
    virtual QString attributeDescription() const = 0;
};

class ContinuousCorrelation : public ICorrelationInfo
{
public:
    virtual EdgeList edgeList(const ContinuousDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    virtual CovarianceMatrix matrix(const ContinuousDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    static std::unique_ptr<ContinuousCorrelation> create(CorrelationType correlationType,
        CorrelationFilterType correlationFilterType);
};

template<typename DataVectors>
class ThresholdFilter
{
private:
    CorrelationPolarity _polarity = CorrelationPolarity::Positive;
    double _threshold = 0.0;

public:
    ThresholdFilter(const DataVectors&, const QVariantMap& parameters)
    {
        _threshold = parameters[QStringLiteral("threshold")].toDouble();
        _polarity = NORMALISE_QML_ENUM(CorrelationPolarity, parameters[QStringLiteral("correlationPolarity")].toInt());
    }

    using Results = CorrelationVector<DataVectors>;

    void add(Results* results,
        typename DataVectors::const_iterator a,
        typename DataVectors::const_iterator b,
        double r)
    {
        if(correlationExceedsThreshold(_polarity, r, _threshold))
            results->push_back({a, b, r});
    }
};

template<typename DataVectors>
class KnnFilter
{
private:
    KnnProtoGraph<DataVectors> _protoGraph;

public:
    KnnFilter(const DataVectors& vectors, const QVariantMap& parameters) :
        _protoGraph(vectors, parameters)
    {}

    using Results = std::monostate;

    void add(Results*,
        typename DataVectors::const_iterator a,
        typename DataVectors::const_iterator b,
        double r)
    {
        _protoGraph.add(a, b, r);
    }

    KnnProtoGraph<DataVectors>&& results() { return std::move(_protoGraph); }
};

struct RequiresRanking {};

template<typename Algorithm, template<typename> class FilterMethod>
class CovarianceCorrelation : public ContinuousCorrelation
{
private:
    template<typename A>
    using preprocess_t = decltype(std::declval<A>().preprocess(0, ContinuousDataVectors{}));

    template<typename F>
    using results_t = decltype(std::declval<F>().results());

    auto process(const ContinuousDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const
    {
        size_t size = vectors.front().size();

        if(progressable != nullptr)
            progressable->setProgress(-1);

        uint64_t totalCost = 0;
        for(const auto& vector : vectors)
        {
            totalCost += vector.computeCostHint();

            if constexpr(std::is_base_of_v<RequiresRanking, Algorithm>)
                vector.generateRanking();
        }

        Algorithm algorithm;

        constexpr bool AlgorithmHasPreprocess =
            std::experimental::is_detected_v<preprocess_t, Algorithm>;

        if constexpr(AlgorithmHasPreprocess)
            algorithm.preprocess(size, vectors);

        using FM = FilterMethod<ContinuousDataVectors>;
        FM filterMethod(vectors, parameters);

        std::atomic<uint64_t> cost(0);

        auto results = ThreadPool(name()).parallel_for(vectors.begin(), vectors.end(),
        [&](ContinuousDataVectors::const_iterator vectorAIt)
        {
            const auto* vectorA = &(*vectorAIt);

            if constexpr(std::is_base_of_v<RequiresRanking, Algorithm>)
                vectorA = vectorA->ranking();

            typename FM::Results threadResults;

            if(cancellable != nullptr && cancellable->cancelled())
                return threadResults;

            for(auto vectorBIt = vectorAIt + 1; vectorBIt != vectors.end(); ++vectorBIt)
            {
                const auto* vectorB = &(*vectorBIt);

                if constexpr(std::is_base_of_v<RequiresRanking, Algorithm>)
                    vectorB = vectorB->ranking();

                const double r = algorithm.evaluate(size, vectorA, vectorB);

                if(!std::isfinite(r))
                    continue;

                filterMethod.add(&threadResults, vectorAIt, vectorBIt, r);
            }

            cost += vectorA->computeCostHint();

            if(progressable != nullptr)
                progressable->setProgress(static_cast<int>((cost * 100) / totalCost));

            return threadResults;
        });

        if(progressable != nullptr)
        {
            // Returning the results might take time
            progressable->setProgress(-1);
        }

        constexpr bool FilterMethodHasResults =
            std::experimental::is_detected_v<results_t, FM>;

        if constexpr(FilterMethodHasResults)
            return filterMethod.results();
        else
            return results;
    }

public:
    EdgeList edgeList(const ContinuousDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const final
    {
        if(vectors.empty())
            return {};

        auto results = process(vectors, parameters, cancellable, progressable);

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

    CovarianceMatrix matrix(const ContinuousDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const final
    {
        if(vectors.empty())
            return {};

        auto results = process(vectors, parameters, cancellable, progressable);

        if(cancellable != nullptr && cancellable->cancelled())
            return {};

        CovarianceMatrix matrix(vectors.size());

        for(const auto& result : results)
        {
            auto a = static_cast<size_t>(std::distance(vectors.begin(), result._a));
            auto b = static_cast<size_t>(std::distance(vectors.begin(), result._b));
            matrix.setValueAt(a, b, result._r);
        }

        return matrix;
    }

    QString name() const override                   { return Algorithm::name(); }
    QString description() const override            { return Algorithm::description(); }

    QString attributeName() const override          { return Algorithm::attributeName(); }
    QString attributeDescription() const override   { return Algorithm::attributeDescription(); }
};

class PearsonAlgorithm
{
public:
    double evaluate(size_t size, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB) const;

    static QString name() { return QObject::tr("Pearson"); }
    static QString description()
    {
        return QObject::tr("The Pearson Correlation Coefficient is an indication "
            "of the linear correlation between two variables.");
    }

    static QString attributeName() { return QObject::tr("Pearson Correlation Value"); }
    static QString attributeDescription()
    {
        return QObject::tr("The %1 is an indication of "
            "the linear relationship between two variables.")
            .arg(u::redirectLink("pearson", QObject::tr("Pearson Correlation Coefficient")));
    }
};

using PearsonCorrelation = CovarianceCorrelation<PearsonAlgorithm, ThresholdFilter>;
using PearsonCorrelationKnn = CovarianceCorrelation<PearsonAlgorithm, KnnFilter>;

class SpearmanRankAlgorithm : public PearsonAlgorithm, RequiresRanking
{
public:
    static QString name() { return QObject::tr("Spearman Rank"); }
    static QString description()
    {
        return QObject::tr("Spearman's rank correlation coefficient is a "
            "nonparametric measure of the statistical dependence between "
            "the rankings of two variables. It assesses how well the "
            "relationship between two variables can be described using a "
            "monotonic function.");
    }

    static QString attributeName() { return QObject::tr("Spearman Rank Correlation Value"); }
    static QString attributeDescription()
    {
        return QObject::tr("The %1 is an indication of "
            "the monotomic relationship between two variables.")
            .arg(u::redirectLink("spearman", QObject::tr("Spearman Rank Correlation Coefficient")));
    }
};

using SpearmanRankCorrelation = CovarianceCorrelation<SpearmanRankAlgorithm, ThresholdFilter>;
using SpearmanRankCorrelationKnn = CovarianceCorrelation<SpearmanRankAlgorithm, KnnFilter>;

class EuclideanSimilarityAlgorithm
{
public:
    double evaluate(size_t, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB) const;

    static QString name() { return QObject::tr("Euclidean Similarity"); }
    static QString description()
    {
        return QObject::tr("Euclidean Similarity is essentially the inverse "
            "of the Euclidean distance between two vectors.");
    }

    static QString attributeName() { return QObject::tr("Euclidean Similarity"); }
    static QString attributeDescription()
    {
        return QObject::tr("%1 is essentially the inverse "
            "of the Euclidean distance between two vectors.")
            .arg(u::redirectLink("euclidean", QObject::tr("Euclidean Similarity")));
    }
};

using EuclideanSimilarityCorrelation = CovarianceCorrelation<EuclideanSimilarityAlgorithm, ThresholdFilter>;
using EuclideanSimilarityCorrelationKnn = CovarianceCorrelation<EuclideanSimilarityAlgorithm, KnnFilter>;

class CosineSimilarityAlgorithm
{
public:
    double evaluate(size_t, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB) const;

    static QString name() { return QObject::tr("Cosine Similarity"); }
    static QString description()
    {
        return QObject::tr("Cosine Similarity is a measure of similarity between two "
            "non-zero vectors of an inner product space. It is defined to equal "
            "the cosine of the angle between them, which is also the same as the "
            "inner product of the same vectors normalized to both have length 1.");
    }

    static QString attributeName() { return QObject::tr("Cosine Similarity"); }
    static QString attributeDescription()
    {
        return QObject::tr("%1 is a measure of similarity between two "
            "non-zero vectors of an inner product space.")
            .arg(u::redirectLink("cosine", QObject::tr("Cosine Similarity")));
    }
};

using CosineSimilarityCorrelation = CovarianceCorrelation<CosineSimilarityAlgorithm, ThresholdFilter>;
using CosineSimilarityCorrelationKnn = CovarianceCorrelation<CosineSimilarityAlgorithm, KnnFilter>;

class BicorAlgorithm
{
private:
    const ContinuousDataVector* _base = nullptr;
    ContinuousDataVectors _processedVectors;

public:
    void preprocess(size_t size, const ContinuousDataVectors& vectors);
    double evaluate(size_t, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB) const;

    static QString name() { return QObject::tr("Bicor"); }
    static QString description()
    {
        return QObject::tr("Biweight Midcorrelation (also called Bicor) is a "
            "measure of similarity between samples. It is median-based, rather "
            "than mean-based, thus is less sensitive to outliers.");
    }

    static QString attributeName() { return QObject::tr("Bicor Correlation Value"); }
    static QString attributeDescription()
    {
        return QObject::tr("%1 is a median-based measure of similarity between samples.")
            .arg(u::redirectLink("bicor", QObject::tr("Bicor")));
    }
};

using BicorCorrelation = CovarianceCorrelation<BicorAlgorithm, ThresholdFilter>;
using BicorCorrelationKnn = CovarianceCorrelation<BicorAlgorithm, KnnFilter>;

class DiscreteCorrelation : public ICorrelationInfo
{
public:
    virtual EdgeList edgeList(const DiscreteDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    virtual CovarianceMatrix matrix(const DiscreteDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    static std::unique_ptr<DiscreteCorrelation> create(CorrelationType correlationType,
        CorrelationFilterType correlationFilterType);
};

template<int Denominator, typename AlgorithmInfo, template<typename> class FilterMethod>
class MatchingCorrelation : public DiscreteCorrelation
{
private:
    template<typename F>
    using results_t = decltype(std::declval<F>().results());

    auto process(const TokenisedDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const
    {
        const size_t size = vectors.front().size();
        auto treatAsBinary = parameters[QStringLiteral("treatAsBinary")].toBool();

        if(progressable != nullptr)
            progressable->setProgress(-1);

        uint64_t totalCost = 0;
        for(const auto& vector : vectors)
            totalCost += vector.computeCostHint();

        using FM = FilterMethod<TokenisedDataVectors>;
        FM filterMethod(vectors, parameters);

        std::atomic<uint64_t> cost(0);

        auto results = ThreadPool(name()).parallel_for(vectors.begin(), vectors.end(),
        [&](TokenisedDataVectors::const_iterator vectorAIt)
        {
            typename FM::Results threadResults;

            if(cancellable != nullptr && cancellable->cancelled())
                return threadResults;

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
                for(auto vectorBIt = vectorAIt + 1; vectorBIt != vectors.end(); ++vectorBIt)
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

                    const double r = fraction;

                    if(!std::isfinite(r))
                        continue;

                    filterMethod.add(&threadResults, vectorAIt, vectorBIt, r);
                }
            };

            if(treatAsBinary)
                createEdgesForVectorPairs(binary);
            else
                createEdgesForVectorPairs(nonBinary);

            cost += vectorAIt->computeCostHint();

            if(progressable != nullptr)
                progressable->setProgress(static_cast<int>((cost * 100) / totalCost));

            return threadResults;
        });

        if(progressable != nullptr)
        {
            // Returning the results might take time
            progressable->setProgress(-1);
        }

        constexpr bool FilterMethodHasResults =
            std::experimental::is_detected_v<results_t, FM>;

        if constexpr(FilterMethodHasResults)
            return filterMethod.results();
        else
            return results;
    }

public:
    EdgeList edgeList(const DiscreteDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const final
    {
        if(vectors.empty())
            return {};

        const auto tokenisedVectors = tokeniseDataVectors(vectors);
        auto results = process(tokenisedVectors, parameters, cancellable, progressable);

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

    CovarianceMatrix matrix(const DiscreteDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const final
    {
        if(vectors.empty())
            return {};

        const auto tokenisedVectors = tokeniseDataVectors(vectors);
        auto results = process(tokenisedVectors, parameters, cancellable, progressable);

        if(cancellable != nullptr && cancellable->cancelled())
            return {};

        CovarianceMatrix matrix(tokenisedVectors.size());

        for(const auto& result : results)
        {
            auto a = static_cast<size_t>(std::distance(tokenisedVectors.begin(), result._a));
            auto b = static_cast<size_t>(std::distance(tokenisedVectors.begin(), result._b));
            matrix.setValueAt(a, b, result._r);
        }

        return matrix;
    }

    QString name() const override                   { return AlgorithmInfo::name(); }
    QString description() const override            { return AlgorithmInfo::description(); }

    QString attributeName() const override          { return AlgorithmInfo::attributeName(); }
    QString attributeDescription() const override   { return AlgorithmInfo::attributeDescription(); }
};

struct JaccardCorrelationInfo
{
    static QString name() { return QObject::tr("Jaccard"); }
    static QString description()
    {
        return QObject::tr("The Jaccard Index is a statistic used for "
            "gauging the similarity and diversity of sample sets.");
    }

    static QString attributeName() { return QObject::tr("Jaccard Correlation Value"); }
    static QString attributeDescription()
    {
        return QObject::tr("The %1 is a statistic used for gauging "
            "the similarity and diversity of sample sets.")
            .arg(u::redirectLink("jaccard", QObject::tr("Jaccard Index")));
    }
};

using JaccardCorrelation = MatchingCorrelation<0, JaccardCorrelationInfo, ThresholdFilter>;
using JaccardCorrelationKnn = MatchingCorrelation<0, JaccardCorrelationInfo, KnnFilter>;

struct SMCCorrelationInfo
{
    static QString name() { return QObject::tr("Simple Matching Coefficient"); }
    static QString description()
    {
        return QObject::tr("The Simple Matching Coefficient is a statistic used for "
            "gauging the similarity and diversity of sample sets. It is identical "
            "to Jaccard, except that it counts mutual absence.");
    }

    static QString attributeName() { return QObject::tr("Simple Matching Coefficient"); }
    static QString attributeDescription()
    {
        return QObject::tr("The %1 is a statistic used for gauging "
            "the similarity and diversity of sample sets.")
            .arg(u::redirectLink("smc", QObject::tr("Simple Matching Coefficient (SMC)")));
    }
};

using SMCCorrelation = MatchingCorrelation<1, SMCCorrelationInfo, ThresholdFilter>;
using SMCCorrelationKnn = MatchingCorrelation<1, SMCCorrelationInfo, KnnFilter>;

#endif // CORRELATION_H
