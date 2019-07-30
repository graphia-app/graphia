#ifndef FEATURESCALING_H
#define FEATURESCALING_H

#include "normaliser.h"

#include <vector>

// https://en.wikipedia.org/wiki/Feature_scaling

class MinMaxNormaliser : public Normaliser
{
public:
    bool process(std::vector<CorrelationDataRow>& dataRows, IParser* parser) const override;
};

class MeanNormaliser : public Normaliser
{
public:
    bool process(std::vector<CorrelationDataRow>& dataRows, IParser* parser) const override;
};

class StandardisationNormaliser : public Normaliser
{
public:
    bool process(std::vector<CorrelationDataRow>& dataRows, IParser* parser) const override;
};

class UnitScalingNormaliser : public Normaliser
{
public:
    bool process(std::vector<CorrelationDataRow>& dataRows, IParser* parser) const override;
};

#endif // FEATURESCALING_H
