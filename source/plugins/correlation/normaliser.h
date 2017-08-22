#ifndef NORMALISER_H
#define NORMALISER_H

#include "shared/loading/progressfn.h"

#include <vector>
#include <cstdlib>

class Normaliser
{
public:
    virtual ~Normaliser() {}
    virtual bool process(std::vector<double>& data, size_t numColumns, size_t numRows,
                         const std::function<bool()>& cancelled, const ProgressFn& progress) const = 0;
};

#endif // NORMALISER_H
