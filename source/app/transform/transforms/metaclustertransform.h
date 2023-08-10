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

#ifndef METACLUSTERTRANSFORM_H
#define METACLUSTERTRANSFORM_H

#include "transform/graphtransform.h"
#include "transform/transformedgraph.h"

#include "shared/graph/elementid_containers.h"
#include "attributes/conditionfncreator.h"

template<typename ClusterTransformFactory>
class MetaClusterTransform : public GraphTransform
{
public:
    explicit MetaClusterTransform(GraphModel* graphModel) : _graphModel(graphModel) {}

    void apply(TransformedGraph& target) override
    {
        setPhase(QObject::tr("Contracting"));

        if(config().attributeNames().empty())
        {
            addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
            return;
        }

        auto attributeName = config().attributeNames().front();

        GraphTransformConfig::TerminalCondition condition
        {
            u"$source.%1"_s.arg(attributeName),
            ConditionFnOp::Equality::Equal,
            u"$target.%1"_s.arg(attributeName),
        };

        auto conditionFn = CreateConditionFnFor::edge(*_graphModel, condition);
        if(conditionFn == nullptr)
        {
            addAlert(AlertType::Error, QObject::tr("Invalid condition"));
            return;
        }

        EdgeIdSet edgeIdsToContract;

        for(auto edgeId : target.edgeIds())
        {
            if(conditionFn(edgeId))
                edgeIdsToContract.insert(edgeId);
        }

        MutableGraph mutableGraph = target.mutableGraph();
        mutableGraph.contractEdges(edgeIdsToContract);
        TransformedGraph contractedGraph(*_graphModel, mutableGraph);
        contractedGraph = mutableGraph;

        ClusterTransformFactory clusterTransformFactory(_graphModel);
        auto clusterTransform = clusterTransformFactory.create({});
        clusterTransform->setConfig(config());
        clusterTransform->apply(contractedGraph);
        //FIXME move the attributes from contractedGraph to target
    }

private:
    GraphModel* _graphModel = nullptr;
};

template<typename ClusterTransformFactory>
class MetaClusterTransformFactory : public GraphTransformFactory
{
private:
    ClusterTransformFactory _clusterTransformFactory;

public:
    explicit MetaClusterTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel), _clusterTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return _clusterTransformFactory.description(); //FIXME
    }

    QString category() const override { return QObject::tr("Clustering"); }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Reference Node Attribute", ElementType::Node, ValueType::String,
                QObject::tr("This attribute is used to guide the contraction of the "
                    "graph which is subsequently used to create the meta-clusters.")
            }
        };
    }

    GraphTransformParameters parameters() const override
    {
        return _clusterTransformFactory.parameters(); //FIXME
    }

    /*DefaultVisualisations defaultVisualisations() const override
    {
        return {{"Louvain Cluster", ValueType::String, {}, QObject::tr("Colour")}};
    }*/

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig&) const override
    {
        return std::make_unique<MetaClusterTransform<ClusterTransformFactory>>(graphModel());
    }
};

#endif // METACLUSTERTRANSFORM_H
