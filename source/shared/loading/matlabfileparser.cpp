/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "matlabfileparser.h"

#include <QUrl>

struct MatLabMatrix
{
    matvar_t* _var = nullptr;
    size_t _numColumns = 0;
    size_t _numRows = 0;

    bool operator>(const MatLabMatrix& other) const
    {
        return (_numColumns * _numRows) > (other._numColumns * other._numRows);
    }
};

MatLabMatrix findBiggestMatrix(matvar_t* v, MatLabMatrix& m)
{
    switch(v->class_type)
    {
    case MAT_C_STRUCT:
    {
        for(auto i = 0u; i < Mat_VarGetNumberOfFields(v); i++)
        {
            auto* field = Mat_VarGetStructFieldByIndex(v, i, 0);
            findBiggestMatrix(field, m);
        }
        break;
    }

    case MAT_C_SPARSE:
    case MAT_C_DOUBLE:
    case MAT_C_SINGLE:
    case MAT_C_INT8:
    case MAT_C_UINT8:
    case MAT_C_INT16:
    case MAT_C_UINT16:
    case MAT_C_INT32:
    case MAT_C_UINT32:
    case MAT_C_INT64:
    case MAT_C_UINT64:
    {
        if(v->rank != 2)
            break;

        MatLabMatrix candidate{v, v->dims[0], v->dims[1]};
        if(candidate > m)
            m = candidate;

        break;
    }

    // Ignore everything else
    case MAT_C_OBJECT:
    case MAT_C_CHAR:
    case MAT_C_EMPTY:
    case MAT_C_FUNCTION:
    case MAT_C_OPAQUE:
    case MAT_C_CELL:
    default:
        break;
    }

    return m;
}

MatLabMatrix findBiggestMatrix(matvar_t* v)
{
    MatLabMatrix m;
    return findBiggestMatrix(v, m);
}

bool MatLabFileParser::parse(const QUrl& url, IGraphModel*)
{
    setProgress(-1);

    auto* matFile = Mat_Open(url.toLocalFile().toUtf8().data(), MAT_ACC_RDONLY);

    if(matFile == nullptr)
        return false;

    matvar_t* matVar = nullptr;
    bool result = false;

    MatLabMatrix biggestMatrix;

    while((matVar = Mat_VarReadNext(matFile)) != nullptr)
    {
        auto candidate = findBiggestMatrix(matVar);

        if(candidate > biggestMatrix)
        {
            biggestMatrix = candidate;
            const auto* v = biggestMatrix._var;

            switch(v->data_type)
            {
            case MAT_T_DOUBLE:  return storeMatVarAsTabularData<double>(v);
            case MAT_T_SINGLE:  return storeMatVarAsTabularData<float>(v);
            case MAT_T_INT64:   return storeMatVarAsTabularData<mat_int64_t>(v);
            case MAT_T_UINT64:  return storeMatVarAsTabularData<mat_uint64_t>(v);
            case MAT_T_INT32:   return storeMatVarAsTabularData<mat_int32_t>(v);
            case MAT_T_UINT32:  return storeMatVarAsTabularData<mat_uint32_t>(v);
            case MAT_T_INT16:   return storeMatVarAsTabularData<mat_int16_t>(v);
            case MAT_T_UINT16:  return storeMatVarAsTabularData<mat_uint16_t>(v);
            case MAT_T_INT8:    return storeMatVarAsTabularData<mat_int8_t>(v);
            case MAT_T_UINT8:   return storeMatVarAsTabularData<mat_uint8_t>(v);
            default:            return false;
            }
        }

        Mat_VarFree(matVar);
    }

    Mat_Close(matFile);
    return result;
}

bool MatLabFileParser::canLoad(const QUrl& url)
{
    auto* matFile = Mat_Open(url.toLocalFile().toUtf8().data(), MAT_ACC_RDONLY);

    if(matFile == nullptr)
        return false;

    Mat_Close(matFile);
    return true;
}
