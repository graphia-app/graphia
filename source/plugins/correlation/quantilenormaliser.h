#ifndef QUANTILENORMALISER_H
#define QUANTILENORMALISER_H

#include "normaliser.h"

class QuantileNormaliser : public Normaliser
{
public:
    bool process(std::vector<CorrelationDataRow>& dataRows, IParser* parser) const override;
};

#endif // QUANTILENORMALISER_H
