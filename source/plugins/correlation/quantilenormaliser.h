#ifndef QUANTILENORMALISER_H
#define QUANTILENORMALISER_H

#include "normaliser.h"

#include "shared/loading/tabulardata.h"
#include "shared/utils/utils.h"

#include <vector>

#include <QString>

class QuantileNormaliser : public Normaliser
{
private:
    const TabularData& _data;
    size_t _firstDataColumn;
    size_t _firstDataRow;
    size_t _numDataColumns;
    size_t _numDataRows;

    std::vector<double> _rowMeans;
    std::vector<std::vector<double>> _ranking;
public:
    QuantileNormaliser(const TabularData& data, size_t firstDataColumn, size_t firstDataRow);
    double value(size_t column, size_t row);
};

#endif // QUANTILENORMALISER_H
