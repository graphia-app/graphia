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

#ifndef MCLTRANSFORM_H
#define MCLTRANSFORM_H

#include "transform/graphtransform.h"

#include "shared/utils/flags.h"
#include "shared/utils/redirects.h"

class MCLTransform : public GraphTransform
{
public:
    explicit MCLTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    void apply(TransformedGraph& target) const override;

private:
    void enableDebugIteration(){ _debugIteration = true; }
    void enableDebugMatrices(){ _debugMatrices = true; }
    void disableDebugIteration(){ _debugIteration = false; }
    void disableDebugMatrices(){ _debugMatrices = false; }

private:
    const float MCL_PRUNE_LIMIT = 1e-4f;
    const float MCL_CONVERGENCE_LIMIT = 1e-3f;

    bool _debugIteration = false;
    bool _debugMatrices = false;

    void calculateMCL(float inflation, TransformedGraph& target) const;

private:
    GraphModel* _graphModel = nullptr;
};

class MCLTransformFactory : public GraphTransformFactory
{
public:
    explicit MCLTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("%1 finds discrete groups (clusters) of nodes based on a flow simulation model.")
            .arg(u::redirectLink("mcl", QObject::tr("MCL - Markov Clustering")));
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
                .setInitialValue(2.0)
                .setRange(1.1, 3.5)
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"MCL Cluster", ValueType::String, {}, QObject::tr("Colour")}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // MCLTRANSFORM_H
