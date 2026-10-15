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

#ifndef REMOVELEAVESTRANSFORM_H
#define REMOVELEAVESTRANSFORM_H

#include "app/transform/graphtransform.h"

#include <QObject>
#include <QString>

#include <memory>

using namespace Qt::Literals::StringLiterals;

class RemoveLeavesTransform : public GraphTransform
{
public:
    void apply(TransformedGraph& target) override;
};

class RemoveBranchesTransform : public GraphTransform
{
public:
    void apply(TransformedGraph& target) override;
};

class RemoveLeavesTransformFactory : public GraphTransformFactory
{
public:
    QString name() const override { return u"Remove Leaves"_s; }
    QString displayName() const override { return QObject::tr("Remove Leaves"); }

    QString description() const override
    {
        return QObject::tr("Remove leaf nodes from the graph.");
    }

    QString category() const override { return QObject::tr("Structural"); }

    GraphTransformParameters parameters() const override
    {
        return
        {
            GraphTransformParameter::create("Limit")
                .setType(ValueType::Int)
                .setDescription(QObject::tr("The number of leaves to remove from a branch before stopping."))
                .setInitialValue(1)
                .setMin(1)
        };
    }

    std::unique_ptr<GraphTransform> create(GraphModel& graphModel, const GraphTransformConfig& graphTransformConfig) const override;
};

class RemoveBranchesTransformFactory : public GraphTransformFactory
{
public:
    QString name() const override { return u"Remove Branches"_s; }
    QString displayName() const override { return QObject::tr("Remove Branches"); }

    QString description() const override
    {
        return QObject::tr("Remove branches from the graph, leaving only cycles.");
    }

    QString category() const override { return QObject::tr("Structural"); }

    std::unique_ptr<GraphTransform> create(GraphModel& graphModel, const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // REMOVELEAVESTRANSFORM_H
