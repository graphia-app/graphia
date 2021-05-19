/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MATLABFILEPARSER_H
#define MATLABFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"

#include <matio.h>

class MatLabFileParser : public IParser
{
private:
    TabularData _tabularData;

    template<class T>
    bool storeMatVarAsTabularData(const matvar_t* v)
    {
        _tabularData.reset();

        uint64_t progress = 0;
        setProgress(-1);

        if(v->class_type == MAT_C_SPARSE)
        {
            auto* sparseMatrix = reinterpret_cast<mat_sparse_t*>(v->data);

            _tabularData.reserve(3, sparseMatrix->njc);
            size_t tabularDataRow = 0;

            auto* dataPtr = reinterpret_cast<T*>(sparseMatrix->data);

            for(int i = 0; i < sparseMatrix->njc - 1; i++ )
            {
                if(cancelled())
                    return false;

                size_t row = static_cast<size_t>(i);

                for(int j = sparseMatrix->jc[i]; j < sparseMatrix->jc[i + 1] && j < sparseMatrix->ndata; j++ )
                {
                    size_t column = static_cast<size_t>(sparseMatrix->ir[j]);

                    T value = *(dataPtr + j);

                    _tabularData.setValueAt(0, tabularDataRow, QString::number(column));
                    _tabularData.setValueAt(1, tabularDataRow, QString::number(row));
                    _tabularData.setValueAt(2, tabularDataRow, QString::number(value));
                    tabularDataRow++;

                    setProgress(static_cast<int>((i * 100) / sparseMatrix->njc));
                }
            }
        }
        else
        {
            Q_ASSERT(v->class_type >= MAT_C_DOUBLE && v->class_type <= MAT_C_UINT64);

            size_t numColumns = v->dims[0];
            size_t numRows = v->dims[1];
            auto totalIterations = static_cast<uint64_t>(numColumns * numRows);

            _tabularData.reserve(numColumns, numRows);

            auto* dataPtr = reinterpret_cast<T*>(v->data);

            for(size_t row = 0; row < numRows; row++)
            {
                size_t rowOffset = row * numColumns;

                for(size_t column = 0; column < numColumns; column++)
                {
                    if(cancelled())
                        return false;

                    T value = *(dataPtr + rowOffset + column);
                    _tabularData.setValueAt(column, row, QString::number(value));

                    setProgress(static_cast<int>((progress++ * 100) / totalIterations));
                }
            }
        }

        setProgress(-1);
        _tabularData.shrinkToFit();

        return true;
    }

public:
    explicit MatLabFileParser(IParser* parent = nullptr)
    {
        if(parent != nullptr)
            setProgressFn([parent](int percent) { parent->setProgress(percent); });
    }

    bool parse(const QUrl& url, IGraphModel* graphModel = nullptr) override;

    TabularData& tabularData() { return _tabularData; }

    static bool canLoad(const QUrl& url);
};

#endif // MATLABFILEPARSER_H
