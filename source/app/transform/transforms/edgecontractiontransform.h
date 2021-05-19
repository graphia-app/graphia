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

#ifndef EDGECONTRACTIONTRANSFORM_H
#define EDGECONTRACTIONTRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include "shared/utils/redirects.h"

#include <vector>

class EdgeContractionTransform : public GraphTransform
{
public:
    explicit EdgeContractionTransform(const GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

private:
    const GraphModel* _graphModel;
};

class EdgeContractionTransformFactory : public GraphTransformFactory
{
public:
    explicit EdgeContractionTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("%1 which match the specified condition while simultaneously "
            "merging the pairs of nodes that they previously joined.")
            .arg(u::redirectLink("contraction", QObject::tr("Remove edges")));
    }
    QString category() const override { return QObject::tr("Structural"); }
    ElementType elementType() const override { return ElementType::Edge; }
    bool requiresCondition() const override { return true; }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // EDGECONTRACTIONTRANSFORM_H
