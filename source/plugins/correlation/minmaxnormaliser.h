#ifndef MINMAXNORMALISER_H
#define MINMAXNORMALISER_H

#include "normaliser.h"

#include <vector>

class MinMaxNormaliser : public Normaliser
{
public:
    bool process(std::vector<double>& data, size_t numColumns, size_t numRows,
                 const std::function<bool()>& cancelled, const ProgressFn& progress) const override;
};

#endif // MINMAXNORMALISER_H
