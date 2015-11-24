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
    const EdgeArray<float>& edgeWeights() const { return _edgeWeights; }
    void setEdgeWeight(EdgeId edgeId, float weight) { _edgeWeights[edgeId] = weight; }

private slots:
    void onGraphChanged(const Graph*);
};

#endif // WEIGHTEDEDGEGRAPHMODEL_H
