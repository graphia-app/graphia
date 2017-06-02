#ifndef MINMAXNORMALISER_H
#define MINMAXNORMALISER_H

#include "normaliser.h"

#include "shared/loading/tabulardata.h"
#include "shared/utils/utils.h"

#include <vector>
#include <set>

#include <QString>

class MinMaxNormaliser : public Normaliser
{
private:
    const TabularData& _data;
    std::vector<double> _minColumn;
    std::vector<double> _maxColumn;
    size_t _firstDataColumn;
    size_t _firstDataRow;
public:
    MinMaxNormaliser(const TabularData& data, size_t firstDataColumn,
                     size_t firstDataRow);
    double value(size_t column, size_t row);
};

#endif // MINMAXNORMALISER_H
