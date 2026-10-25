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

#ifndef ECCENTRICITYTRANSFORM_H
#define ECCENTRICITYTRANSFORM_H

#include "app/transform/graphtransform.h"

#include <QObject>
#include <QString>

#include <memory>

using namespace Qt::Literals::StringLiterals;

class EccentricityTransform : public GraphTransform
{
public:
    explicit EccentricityTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    void apply(TransformedGraph& target) override;

private:
    GraphModel* _graphModel = nullptr;
    void calculateDistances(TransformedGraph& target);
};

class EccentricityTransformFactory : public GraphTransformFactory
{
public:
    QString name() const override { return u"Eccentricity"_s; }
    QString displayName() const override { return QObject::tr("Eccentricity"); }

    static AttributeDeclaration nodeEccentricityAttribute() { return {._name = u"NodeEccentricity"_s, ._displayName = QObject::tr("Node Eccentricity")}; }
    AttributeDeclarations attributeDeclarations() const override { return {nodeEccentricityAttribute()}; }

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
        return
        {
            {
                ._attributeName = nodeEccentricityAttribute()._name, ._elementType = ElementType::Node,
                ._attributeValueType = ValueType::Int, ._attributeFlags = {AttributeFlag::VisualiseByComponent},
                ._channel = "Colour"
            }
        };
    }

    std::unique_ptr<GraphTransform> create(GraphModel& graphModel, const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // ECCENTRICITYTRANSFORM_H
