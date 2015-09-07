#include "weightededgegraphmodel.h"
#include "../utils/utils.h"

WeightedEdgeGraphModel::WeightedEdgeGraphModel(const QString &name) :
    GenericGraphModel(name),
    _edgeWeights(mutableGraph())
{
}

