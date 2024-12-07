/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef COVARIANCEMATRIX_H
#define COVARIANCEMATRIX_H

#include <cstddef>
#include <vector>

class CovarianceMatrix
{
private:
    size_t _size = 0;
    std::vector<double> _values;

public:
    CovarianceMatrix() = default;

    // By definition a distance matrix is square, so size refers to the dimension of one side
    explicit CovarianceMatrix(size_t size);

    double valueAt(size_t column, size_t row) const;
    void setValueAt(size_t column, size_t row, double value);

    size_t size() const { return _size; }
};

#endif // COVARIANCEMATRIX_H
