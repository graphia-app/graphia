#include "weightededgegraphmodel.h"
#include "../utils/utils.h"

#include <algorithm>

WeightedEdgeGraphModel::WeightedEdgeGraphModel(const QString &name) :
    GraphModel(name),
    _edgeWeights(mutableGraph())
{
    connect(&transformedGraph(), &Graph::graphChanged, this, &WeightedEdgeGraphModel::onGraphChanged, Qt::DirectConnection);

    addDataField(tr("Edge Weight"))
            .setFloatValueFn(FLOAT_EDGE_FN([this](EdgeId edgeId) { return _edgeWeights[edgeId]; }));
}

void WeightedEdgeGraphModel::onGraphChanged(const Graph*)
{
    if(!_edgeWeights.empty())
    {
        float min = *std::min_element(_edgeWeights.begin(), _edgeWeights.end());
        float max = *std::max_element(_edgeWeights.begin(), _edgeWeights.end());

        mutableDataFieldByName(tr("Edge Weight")).setFloatMin(min).setFloatMax(max);
    }
}
