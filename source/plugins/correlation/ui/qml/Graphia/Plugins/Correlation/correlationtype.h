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

#ifndef CORRELATIONTYPE_H
#define CORRELATIONTYPE_H

#include "shared/utils/qmlenum.h"

// Note: the ordering of these enums is important from a save
// file point of view; i.e. only append, don't reorder

DEFINE_QML_ENUM(CorrelationType,
    Pearson,
    SpearmanRank,
    Jaccard,
    SMC,
    EuclideanSimilarity,
    CosineSimilarity,
    Bicor);

DEFINE_QML_ENUM(CorrelationDataType,
    Continuous,
    Discrete);

DEFINE_QML_ENUM(CorrelationPolarity,
    Positive,
    Negative,
    Both);

DEFINE_QML_ENUM(CorrelationFilterType,
    Threshold,
    Knn);

DEFINE_QML_ENUM(ScalingType,
    None,
    Log2,
    Log10,
    AntiLog2,
    AntiLog10,
    ArcSin);

DEFINE_QML_ENUM(NormaliseType,
    None,
    MinMax,
    Quantile,
    Mean,
    Standarisation,
    UnitScaling,
    Softmax);

DEFINE_QML_ENUM(MissingDataType,
    Constant,
    ColumnAverage,
    RowInterpolation);

DEFINE_QML_ENUM(ClippingType,
    None,
    Constant,
    Winsorization);

bool correlationExceedsThreshold(CorrelationPolarity polarity, double r, double threshold);

#endif // CORRELATIONTYPE_H
