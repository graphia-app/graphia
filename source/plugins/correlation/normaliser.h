#ifndef NORMALISER_H
#define NORMALISER_H

#include "shared/loading/iparser.h"

#include <vector>
#include <cstdlib>

class Cancellable;

class Normaliser
{
public:
    virtual ~Normaliser() = default;
    virtual bool process(std::vector<double>& data, size_t numColumns, size_t numRows,
                         IParser& parser) const = 0;
};

#endif // NORMALISER_H
