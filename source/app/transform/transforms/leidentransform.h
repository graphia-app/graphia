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

#ifndef LEIDENTRANSFORM_H
#define LEIDENTRANSFORM_H

#include "transform/graphtransform.h"

#include "shared/utils/flags.h"
#include "shared/utils/redirects.h"

class LeidenTransform : public GraphTransform
{
public:
    explicit LeidenTransform(GraphModel* graphModel, bool weighted) :
        _graphModel(graphModel), _weighted(weighted) {}
    void apply(TransformedGraph& target) override;

private:
    GraphModel* _graphModel = nullptr;
    bool _weighted = false;
};

class LeidenTransformFactory : public GraphTransformFactory
{
public:
    explicit LeidenTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("%1 is a method for finding clusters by measuring edge "
            "density from within communities to neighbouring communities.")
            .arg(u::redirectLink("leiden", QObject::tr("Leiden Modularity")));
    }

    QString category() const override { return QObject::tr("Clustering"); }

    GraphTransformParameters parameters() const override
    {
        return
        {
            GraphTransformParameter::create("Granularity")
                .setType(ValueType::Float)
                .setDescription(QObject::tr("The size of the resultant clusters. "
                    "A larger granularity value results in smaller clusters."))
                .setInitialValue(0.5)
                .setRange(0.0, 1.0)
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"Leiden Cluster", ValueType::String, {}, QObject::tr("Colour")}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig&) const override
    {
        return std::make_unique<LeidenTransform>(graphModel(), false);
    }
};

class WeightedLeidenTransformFactory : public LeidenTransformFactory
{
public:
    using LeidenTransformFactory::LeidenTransformFactory;

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Weighting Attribute",
                ElementType::Edge, ValueType::Numerical,
                QObject::tr("The attribute whose value is used to weight edges.")
            }
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"Weighted Leiden Cluster", ValueType::String, {}, QObject::tr("Colour")}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig&) const override
    {
        return std::make_unique<LeidenTransform>(graphModel(), true);
    }
};

#endif // LEIDENTRANSFORM_H
