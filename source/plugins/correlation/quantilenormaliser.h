#ifndef QUANTILENORMALISER_H
#define QUANTILENORMALISER_H

#include "normaliser.h"

class QuantileNormaliser : public Normaliser
{
public:
    bool process(std::vector<double>& data, size_t numColumns, size_t numRows,
                 const std::function<bool()>& cancelled, const ProgressFn& progress) const;
};

#endif // QUANTILENORMALISER_H
