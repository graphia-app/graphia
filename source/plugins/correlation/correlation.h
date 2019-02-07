#ifndef CORRELATION_H
#define CORRELATION_H

#include "correlationdatarow.h"
#include "correlationedge.h"

#include "shared/utils/qmlenum.h"
#include "shared/loading/iparser.h"
#include "shared/utils/threadpool.h"

#include <vector>
#include <cmath>

#include <QObject>
#include <QString>

DEFINE_QML_ENUM(
    Q_GADGET, CorrelationType,
    Pearson,
    SpearmanRank);

DEFINE_QML_ENUM(
    Q_GADGET, CorrelationPolarity,
    Positive,
    Negative,
    Both);

class Correlation
{
public:
    virtual ~Correlation() = default;

    virtual std::vector<CorrelationEdge> process(const std::vector<CorrelationDataRow>& rows,
        double minimumThreshold, CorrelationPolarity polarity = CorrelationPolarity::Positive,
        IParser* parser = nullptr) const = 0;

    virtual QString attributeName() const = 0;
    virtual QString attributeDescription() const = 0;

    static std::unique_ptr<Correlation> create(CorrelationType correlationType);
};

enum class RowType
{
    Raw,
    Ranking
};

template<typename Algorithm, RowType rowType = RowType::Raw>
class CovarianceCorrelation : public Correlation
{
public:
    std::vector<CorrelationEdge> process(const std::vector<CorrelationDataRow>& rows,
        double minimumThreshold, CorrelationPolarity polarity = CorrelationPolarity::Positive,
        IParser* parser = nullptr) const final
    {
        if(rows.empty())
            return {};

        size_t numColumns = std::distance(rows.front().begin(), rows.front().end());

        if(parser != nullptr)
            parser->setProgress(-1);

        uint64_t totalCost = 0;
        for(const auto& row : rows)
            totalCost += row.computeCostHint();

        std::atomic<uint64_t> cost(0);

        auto results = ThreadPool(QStringLiteral("Correlation")).concurrent_for(rows.begin(), rows.end(),
        [&](std::vector<CorrelationDataRow>::const_iterator rowAIt)
        {
            const auto* rowA = &(*rowAIt);

            if constexpr(rowType == RowType::Ranking)
                rowA = rowA->ranking();

            std::vector<CorrelationEdge> edges;

            if(parser != nullptr && parser->cancelled())
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

            if(parser != nullptr)
                parser->setProgress((cost * 100) / totalCost);

            return edges;
        });

        if(parser != nullptr)
        {
            // Returning the results might take time
            parser->setProgress(-1);
        }

        std::vector<CorrelationEdge> edges;
        edges.reserve(std::distance(results.begin(), results.end()));
        edges.insert(edges.end(), std::make_move_iterator(results.begin()),
            std::make_move_iterator(results.end()));

        return edges;
    }
};

struct PearsonAlgorithm
{
    static double evaluate(size_t numColumns, const CorrelationDataRow* rowA, const CorrelationDataRow* rowB)
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
    QString attributeName() const override
    {
        return QObject::tr("Pearson Correlation Value");
    }

    QString attributeDescription() const override
    {
        return QObject::tr(R"(The <a href="https://kajeka.com/graphia/pearson">)"
            "Pearson Correlation Coefficient</a> is an indication of "
            "the linear relationship between two variables.");
    }
};

class SpearmanRankCorrelation : public CovarianceCorrelation<PearsonAlgorithm, RowType::Ranking>
{
public:
    QString attributeName() const override
    {
        return QObject::tr("Spearman Rank Correlation Value");
    }

    QString attributeDescription() const override
    {
        return QObject::tr(R"(The <a href="https://kajeka.com/graphia/spearman">)"
            "Spearman Rank Correlation Coefficient</a> is an indication of "
            "the monotomic relationship between two variables.");
    }
};

#endif // CORRELATION_H
