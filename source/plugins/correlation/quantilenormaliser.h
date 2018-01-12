#ifndef QUANTILENORMALISER_H
#define QUANTILENORMALISER_H

#include "normaliser.h"

class QuantileNormaliser : public Normaliser
{
public:
    bool process(std::vector<double>& data, size_t numColumns, size_t numRows,
                 Cancellable& cancellable, const ProgressFn& progress) const override;
};

#endif // QUANTILENORMALISER_H
