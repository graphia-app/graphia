#ifndef CORRELATION_H
#define CORRELATION_H

#include "correlationdatarow.h"
#include "correlationedge.h"

#include "shared/loading/iparser.h"
#include "shared/utils/threadpool.h"
#include "shared/utils/iterator_range.h"

#include <vector>

class Correlation
{
public:
    Correlation(const std::vector<CorrelationDataRow>& rows) :
        _rows(&rows)
    {}

    std::vector<CorrelationEdge> process(double minimumThreshold, IParser* parser = nullptr)
    {
        if(_rows->empty())
            return {};

        size_t numColumns = std::distance(_rows->front().begin(), _rows->front().end());

        if(parser != nullptr)
            parser->setProgress(-1);

        uint64_t totalCost = 0;
        for(const auto& row : *_rows)
            totalCost += row.computeCostHint();

        std::atomic<uint64_t> cost(0);

        auto results = ThreadPool(QStringLiteral("Correlation")).concurrent_for(_rows->begin(), _rows->end(),
        [&](std::vector<CorrelationDataRow>::const_iterator rowAIt)
        {
            const auto& rowA = *rowAIt;
            std::vector<CorrelationEdge> edges;

            if(parser != nullptr && parser->cancelled())
                return edges;

            for(const auto& rowB : make_iterator_range(rowAIt + 1, _rows->end()))
            {
                double productSum = std::inner_product(rowA.begin(), rowA.end(), rowB.begin(), 0.0);
                double numerator = (numColumns * productSum) - (rowA.sum() * rowB.sum());
                double denominator = rowA.variability() * rowB.variability();

                double r = numerator / denominator;

                if(std::isfinite(r) && r >= minimumThreshold)
                    edges.push_back({rowA.nodeId(), rowB.nodeId(), r});
            }

            cost += rowA.computeCostHint();

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
        edges.insert(edges.end(), std::make_move_iterator(results.begin()), std::make_move_iterator(results.end()));

        return edges;
    }

private:
    const std::vector<CorrelationDataRow>* _rows = nullptr;
};

using PearsonCorrelation = Correlation;

#endif // CORRELATION_H
