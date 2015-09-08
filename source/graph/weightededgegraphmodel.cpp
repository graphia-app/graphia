#include "weightededgegraphmodel.h"
#include "../utils/utils.h"

WeightedEdgeGraphModel::WeightedEdgeGraphModel(const QString &name) :
    GraphModel(name),
    _edgeWeights(mutableGraph())
{
}

