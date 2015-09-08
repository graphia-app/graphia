#ifndef WEIGHTEDEDGEGRAPHMODEL_H
#define WEIGHTEDEDGEGRAPHMODEL_H

#include "graphmodel.h"

class WeightedEdgeGraphModel : public GraphModel
{
    Q_OBJECT
public:
    WeightedEdgeGraphModel(const QString& name);

private:
    EdgeArray<float> _edgeWeights;

public:
    EdgeArray<float>& edgeWeights() { return _edgeWeights; }
    const EdgeArray<float>& edgeWeights() const { return _edgeWeights; }
};

#endif // WEIGHTEDEDGEGRAPHMODEL_H
