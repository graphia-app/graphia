#ifndef NORMALISER_H
#define NORMALISER_H

#include "shared/loading/progressfn.h"

#include <vector>
#include <cstdlib>

class Cancellable;

class Normaliser
{
public:
    virtual ~Normaliser() = default;
    virtual bool process(std::vector<double>& data, size_t numColumns, size_t numRows,
                         Cancellable& cancellable, const ProgressFn& progress) const = 0;
};

#endif // NORMALISER_H
