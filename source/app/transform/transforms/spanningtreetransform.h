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

#ifndef SPANNINGTREETRANSFORM_H
#define SPANNINGTREETRANSFORM_H

#include "app/transform/graphtransform.h"

#include "shared/utils/redirects.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>

using namespace Qt::Literals::StringLiterals;

class SpanningTreeTransform : public GraphTransform
{
public:
    void apply(TransformedGraph& target) override;
};

class SpanningTreeTransformFactory : public GraphTransformFactory
{
public:
    QString name() const override { return u"SpanningForest"_s; }
    QString displayName() const override { return QObject::tr("Spanning Forest"); }

    QString description() const override
    {
        return QObject::tr("Find a %1 for each component.")
            .arg(u::redirectLink("spanning_tree", QObject::tr("spanning tree")));
    }

    QString category() const override { return QObject::tr("Structural") ; }

    GraphTransformParameters parameters() const override
    {
        return
        {
            GraphTransformParameter::create("TraversalOrder")
                .setDisplayName(QObject::tr("Traversal Order"))
                .setType(ValueType::StringList)
                .setDescription(QObject::tr("Whether to visit nodes level by level, or by maximising depth."))
                .setInitialValue(QStringList{"BreadthFirst", "DepthFirst"})
                .setDisplayValues({QObject::tr("Breadth First"), QObject::tr("Depth First")})
        };
    }

    std::unique_ptr<GraphTransform> create(GraphModel& graphModel, const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // SPANNINGTREETRANSFORM_H
