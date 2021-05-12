/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include "shared/utils/redirects.h"

#include <vector>

class SpanningTreeTransform : public GraphTransform
{
public:
    void apply(TransformedGraph& target) const override;
};

class SpanningTreeTransformFactory : public GraphTransformFactory
{
public:
    explicit SpanningTreeTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

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
            GraphTransformParameter::create("Traversal Order")
                .setType(ValueType::StringList)
                .setDescription(QObject::tr("Whether to visit nodes level by level, or by maximising depth."))
                .setInitialValue(QStringList{"Breadth First", "Depth First"})
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // SPANNINGTREETRANSFORM_H
