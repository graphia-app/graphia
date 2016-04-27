#include "weightededgegraphmodel.h"
#include "../utils/utils.h"

#include <algorithm>

WeightedEdgeGraphModel::WeightedEdgeGraphModel(const QString &name) :
    GraphModel(name),
    _edgeWeights(mutableGraph())
{
    connect(&transformedGraph(), &Graph::graphChanged, this, &WeightedEdgeGraphModel::onGraphChanged, Qt::DirectConnection);
}

void WeightedEdgeGraphModel::setEdgeWeight(EdgeId edgeId, float weight)
{
    if(!_hasEdgeWeights)
    {
        addDataField(tr("Edge Weight"))
                .setFloatValueFn([this](EdgeId edgeId_) { return _edgeWeights[edgeId_]; });

        _hasEdgeWeights = true;
    }

    _edgeWeights[edgeId] = weight;
}

void WeightedEdgeGraphModel::onGraphChanged(const Graph*)
{
    if(!_edgeWeights.empty() && _hasEdgeWeights)
    {
        float min = *std::min_element(_edgeWeights.begin(), _edgeWeights.end());
        float max = *std::max_element(_edgeWeights.begin(), _edgeWeights.end());

        mutableDataFieldByName(tr("Edge Weight")).setFloatMin(min).setFloatMax(max);
    }
}
