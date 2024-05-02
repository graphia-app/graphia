/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#ifndef ADJACENCYMATRIXUTILS_H
#define ADJACENCYMATRIXUTILS_H

#include "shared/loading/tabulardata.h"
#include "shared/utils/string.h"

#include <QObject>
#include <QString>
#include <QPoint>

#include <map>

namespace AdjacencyMatrixUtils
{
static MatrixTypeResult isAdjacencyMatrix(const TabularData& tabularData, QPoint* topLeft = nullptr, size_t maxRows = 5)
{
    // A matrix can optionally have column or row headers. Or none.
    // A matrix data rect must be square.
    std::vector<QString> potentialColumnHeaders;

    bool headerMatch = true;
    bool firstColumnAllDouble = true;
    bool firstRowAllDouble = true;

    if(tabularData.numColumns() < 2)
        return {false, QObject::tr("Matrix must have at least 2 rows/columns.")};

    for(size_t rowIndex = 0; rowIndex < std::min(tabularData.numRows(), maxRows); rowIndex++)
    {
        for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            const auto& value = tabularData.valueAt(columnIndex, rowIndex);

            if(rowIndex == 0)
            {
                if(!u::isNumeric(value) && !value.isEmpty() && columnIndex > 0)
                    firstRowAllDouble = false;

                potentialColumnHeaders.push_back(value);
            }

            if(columnIndex == 0)
            {
                if(rowIndex >= potentialColumnHeaders.size() ||
                   potentialColumnHeaders[rowIndex] != value)
                {
                    headerMatch = false;
                }

                       // The first entry could be headers so don't enforce check for a double
                if(rowIndex > 0)
                {
                    if(!u::isNumeric(value) && !value.isEmpty())
                        firstColumnAllDouble = false;
                }
            }
            else if(rowIndex > 0)
            {
                // Check non header elements are doubles
                // This will prevent loading obviously non-matrix files
                // We could handle non-double matrix symbols in future (X, -, I, O etc)
                if(!u::isNumeric(value) && !value.isEmpty())
                    return {false, QObject::tr("Matrix has non-numeric values.")};
            }
        }
    }

    auto numDataColumns = tabularData.numColumns() - (firstColumnAllDouble ? 1 : 0);
    auto numDataRows = tabularData.numRows() - (firstRowAllDouble ? 1 : 0);

    // Note we can't test for equality as we may not be seeing all the rows (due to a row limit)
    if(numDataColumns < numDataRows)
        return {false, QObject::tr("Matrix is not square.")};

    if(topLeft != nullptr)
    {
        topLeft->setX(firstColumnAllDouble ? 1 : 0);
        topLeft->setY(firstRowAllDouble ? 1 : 0);
    }

    if(headerMatch || firstColumnAllDouble || firstRowAllDouble)
        return {true, {}};

    return {false, !headerMatch ? QObject::tr("Matrix headers don't match.") :
        QObject::tr("Matrix doesn't have numeric first row or column.")};
}

static MatrixTypeResult isEdgeList(const TabularData& tabularData, size_t maxRows = 5)
{
    if(tabularData.numColumns() != 3)
        return {false, QObject::tr("Edge list doesn't have 3 columns.")};

    for(size_t rowIndex = 0; rowIndex < std::min(tabularData.numRows(), maxRows); rowIndex++)
    {
        if(!u::isInteger(tabularData.valueAt(0, rowIndex)))
            return {false, QObject::tr("1st edge list column has a non-integral value.")};

        if(!u::isInteger(tabularData.valueAt(1, rowIndex)))
            return {false, QObject::tr("2nd edge list column has a non-integral value.")};

        if(!u::isNumeric(tabularData.valueAt(2, rowIndex)))
            return {false, QObject::tr("3rd edge list column has a non-numeric value.")};
    }

    return {true, {}};
}

} // namespace AdjacencyMatrixUtils

#endif // ADJACENCYMATRIXUTILS_H
