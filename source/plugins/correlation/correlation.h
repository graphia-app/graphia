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

#ifndef CORRELATION_H
#define CORRELATION_H

#include "correlationdatavector.h"
#include "correlationtype.h"

#include "shared/utils/threadpool.h"

#include "shared/graph/covariancematrix.h"
#include "shared/graph/edgelist.h"

#include <QString>
#include <QVariantMap>

#include <memory>

class Cancellable;
class Progressable;

class ICorrelationInfo
{
public:
    virtual ~ICorrelationInfo() = default;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString attributeName() const = 0;
    virtual QString attributeDescription() const = 0;
};

class ContinuousCorrelation : public ICorrelationInfo, public ThreadPoolProvider
{
public:
    virtual EdgeList edgeList(const ContinuousDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    virtual CovarianceMatrix matrix(const ContinuousDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    static std::unique_ptr<ContinuousCorrelation> create(CorrelationType correlationType,
        CorrelationFilterType correlationFilterType);

    // Distance (as opposed to similarity) isn't exposed as a CorrelationType,
    // being an implementation detail of hierarchical clustering
    static std::unique_ptr<ContinuousCorrelation> createEuclideanDistance();
};

class DiscreteCorrelation : public ICorrelationInfo, public ThreadPoolProvider
{
public:
    virtual EdgeList edgeList(const DiscreteDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    virtual CovarianceMatrix matrix(const DiscreteDataVectors& vectors, const QVariantMap& parameters,
        Cancellable* cancellable = nullptr, Progressable* progressable = nullptr) const = 0;

    static std::unique_ptr<DiscreteCorrelation> create(CorrelationType correlationType,
        CorrelationFilterType correlationFilterType);
};

#endif // CORRELATION_H
