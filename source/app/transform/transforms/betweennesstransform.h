/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef BETWEENNESSTRANSFORM_H
#define BETWEENNESSTRANSFORM_H

#include "transform/graphtransform.h"

#include "shared/utils/flags.h"
#include "shared/utils/redirects.h"

class BetweennessTransform : public GraphTransform
{
public:
    explicit BetweennessTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    void apply(TransformedGraph& target) override;

private:
    GraphModel* _graphModel = nullptr;
};

class BetweennessTransformFactory : public GraphTransformFactory
{
public:
    explicit BetweennessTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr(
            "%1 is a measure of centrality in a graph based on shortest paths between nodes. "
            "The betweenness centrality for each node is the number of these shortest paths "
            "that pass through the node.").arg(u::redirectLink("betweenness", QObject::tr("Betweenness Centrality")));
    }
    QString category() const override { return QObject::tr("Metrics"); }
    ElementType elementType() const override { return ElementType::None; }
    DefaultVisualisations defaultVisualisations() const override
    {
        return
        {
            {"Node Betweenness", ValueType::Float, {AttributeFlag::VisualiseByComponent}, QObject::tr("Colour")},
            {"Edge Betweenness", ValueType::Float, {AttributeFlag::VisualiseByComponent}, QObject::tr("Colour")}
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // BETWEENNESSTRANSFORM_H
