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

#include "app/transform/graphtransform.h"

#include "shared/utils/namedbool.h"
#include "shared/utils/redirects.h"

#include <QObject>
#include <QString>

#include <memory>

using namespace Qt::Literals::StringLiterals;

class LeidenTransform : public GraphTransform
{
public:
    explicit LeidenTransform(GraphModel* graphModel, NamedBool<"weighted"> weighted) :
        _graphModel(graphModel), _weighted(weighted) {}
    void apply(TransformedGraph& target) override;

private:
    GraphModel* _graphModel = nullptr;
    NamedBool<"weighted"> _weighted;
};

class LeidenTransformFactory : public GraphTransformFactory
{
public:
    QString name() const override { return u"LeidenCluster"_s; }
    QString displayName() const override { return QObject::tr("Leiden Cluster"); }

    static AttributeDeclaration clusterAttribute(NamedBool<"weighted"> weighted)
    {
        return weighted ?
            AttributeDeclaration{._name = u"WeightedLeidenCluster"_s,      ._displayName = QObject::tr("Weighted Leiden Cluster")} :
            AttributeDeclaration{._name = u"LeidenCluster"_s,              ._displayName = QObject::tr("Leiden Cluster")};
    }

    static AttributeDeclaration clusterSizeAttribute(NamedBool<"weighted"> weighted)
    {
        return weighted ?
            AttributeDeclaration{._name = u"WeightedLeidenClusterSize"_s,  ._displayName = QObject::tr("Weighted Leiden Cluster Size")} :
            AttributeDeclaration{._name = u"LeidenClusterSize"_s,          ._displayName = QObject::tr("Leiden Cluster Size")};
    }

    AttributeDeclarations attributeDeclarations() const override { return {clusterAttribute("weighted"_no), clusterSizeAttribute("weighted"_no)}; }

    QString description() const override
    {
        return QObject::tr("%1 is a method for finding clusters by measuring edge "
            "density from within communities to neighbouring communities. "
            "Unlike Louvain, every cluster it finds is guaranteed to be well connected.")
            .arg(u::redirectLink("leiden", QObject::tr("Leiden Clustering")));
    }

    Category category() const override { return Category::Clustering; }

    GraphTransformParameters parameters() const override
    {
        return
        {
            GraphTransformParameter::create("Granularity")
                .setDisplayName(QObject::tr("Granularity"))
                .setType(ValueType::Float)
                .setDescription(QObject::tr("The size of the resultant clusters. "
                    "A larger granularity value results in smaller clusters."))
                .setInitialValue(0.5)
                .setRange(0.0, 1.0)
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return
        {
            {
                ._attributeName = clusterAttribute("weighted"_no)._name, ._elementType = ElementType::Node,
                ._attributeValueType = ValueType::String, ._attributeFlags = {},
                ._channel = "Colour"
            }
        };
    }

    std::unique_ptr<GraphTransform> create(GraphModel& graphModel, const GraphTransformConfig&) const override
    {
        return std::make_unique<LeidenTransform>(&graphModel, "weighted"_no);
    }
};

class WeightedLeidenTransformFactory : public LeidenTransformFactory
{
public:
    QString name() const override { return u"WeightedLeidenCluster"_s; }
    QString displayName() const override { return QObject::tr("Weighted Leiden Cluster"); }

    AttributeDeclarations attributeDeclarations() const override { return {clusterAttribute("weighted"_yes), clusterSizeAttribute("weighted"_yes)}; }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                QObject::tr("Weighting Attribute"),
                ElementType::Edge, ValueType::Numerical,
                QObject::tr("The attribute whose value is used to weight edges.")
            }
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return
        {
            {
                ._attributeName = clusterAttribute("weighted"_yes)._name, ._elementType = ElementType::Node,
                ._attributeValueType = ValueType::String, ._attributeFlags = {},
                ._channel = "Colour"
            }
        };
    }

    std::unique_ptr<GraphTransform> create(GraphModel& graphModel, const GraphTransformConfig&) const override
    {
        return std::make_unique<LeidenTransform>(&graphModel, "weighted"_yes);
    }
};

#endif // LEIDENTRANSFORM_H
