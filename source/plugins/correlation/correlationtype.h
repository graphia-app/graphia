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

#include "shared/utils/qmlenum.h"

#ifndef CORRELATIONTYPE_H
#define CORRELATIONTYPE_H

// Note: the ordering of these enums is important from a save
// file point of view; i.e. only append, don't reorder

DEFINE_QML_ENUM(
    Q_GADGET, CorrelationType,
    Pearson,
    SpearmanRank,
    Jaccard,
    SMC,
    EuclideanSimilarity,
    CosineSimilarity,
    Bicor);

DEFINE_QML_ENUM(
    Q_GADGET, CorrelationDataType,
    Continuous,
    Discrete);

DEFINE_QML_ENUM(
    Q_GADGET, CorrelationPolarity,
    Positive,
    Negative,
    Both);

#endif // CORRELATIONTYPE_H
