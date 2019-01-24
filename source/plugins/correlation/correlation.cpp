#include "correlation.h"

std::unique_ptr<Correlation> Correlation::create(CorrelationType correlationType)
{
    switch(correlationType)
    {
    case CorrelationType::Pearson:      return std::make_unique<PearsonCorrelation>();
    case CorrelationType::SpearmanRank: return std::make_unique<SpearmanRankCorrelation>();
    default: break;
    }

    return nullptr;
}
