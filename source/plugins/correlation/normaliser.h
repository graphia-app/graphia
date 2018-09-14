#ifndef NORMALISER_H
#define NORMALISER_H

#include "correlationdatarow.h"

#include <vector>
#include <cstdlib>

class Cancellable;
class IParser;

class Normaliser
{
public:
    virtual ~Normaliser() = default;
    virtual bool process(std::vector<CorrelationDataRow>& dataRows, IParser* parser = nullptr) const = 0;
};

#endif // NORMALISER_H
