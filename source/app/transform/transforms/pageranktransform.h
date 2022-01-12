/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef PAGERANKTRANSFORM_H
#define PAGERANKTRANSFORM_H

#include "transform/graphtransform.h"

#include "shared/utils/flags.h"
#include "shared/utils/redirects.h"

class PageRankTransform : public GraphTransform
{
public:
    explicit PageRankTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    void apply(TransformedGraph& target) const override;

    void enableDebug() { _debug = true; }
    void disableDebug() { _debug = false; }

private:
    const float PAGERANK_DAMPING = 0.8f;
    const float PAGERANK_EPSILON = 1e-6f;
    const float PAGERANK_ACCELERATION_MINIMUM = 1e-10f;
    const int PAGERANK_ITERATION_LIMIT = 1000;
    const int AVG_COUNT = 10;

    bool _debug = false;

    void calculatePageRank(TransformedGraph& target) const;
    GraphModel* _graphModel = nullptr;
};

class PageRankTransformFactory : public GraphTransformFactory
{
public:
    explicit PageRankTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Calculate a %1 centrality measurement for each node. This can be viewed as "
            "measure of a node's relative importance in the graph.")
            .arg(u::redirectLink("pagerank", QObject::tr("PageRank")));
    }
    QString category() const override { return QObject::tr("Metrics"); }
    ElementType elementType() const override { return ElementType::None; }
    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"Node PageRank", ValueType::Float, {AttributeFlag::VisualiseByComponent}, QObject::tr("Colour")}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // PAGERANKTRANSFORM_H
