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

#ifndef ECCENTRICITYTRANSFORM_H
#define ECCENTRICITYTRANSFORM_H

#include "transform/graphtransform.h"
#include "shared/utils/flags.h"

class EccentricityTransform : public GraphTransform
{
public:
    explicit EccentricityTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    void apply(TransformedGraph& target) const override;

private:
    GraphModel* _graphModel = nullptr;
    void calculateDistances(TransformedGraph& target) const;
};

class EccentricityTransformFactory : public GraphTransformFactory
{
public:
    explicit EccentricityTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr(
            R"-(<a href="https://graphia.app/redirects/eccentricity">Eccentricity</a> )-"
            "calculates the shortest path between every node and assigns the longest path length found for that node. "
            "This is a measure of a node's position within the overall graph structure.");
    }
    QString category() const override { return QObject::tr("Metrics"); }
    ElementType elementType() const override { return ElementType::None; }
    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"Node Eccentricity", ValueType::Float, {AttributeFlag::VisualiseByComponent}, QObject::tr("Colour")}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // ECCENTRICITYTRANSFORM_H
