#ifndef NORMALISINGLAYOUT_H
#define NORMALISINGLAYOUT_H

#include "layout.h"

class NormalisingLayout : public Layout
{
    Q_OBJECT
private:
    float _spread;
    float _nodeDensity;
    float _maxDimension;

public:
    NormalisingLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        Layout(graph, positions), _spread(10.0f), _nodeDensity(0.0f)
    {}

    void setSpread(float _spread) { this->_spread = _spread; }
    float spread() { return _spread; }

    void setNodeDensity(float _nodeDensity) { this->_nodeDensity = _nodeDensity; }
    float maxDimension() { return _maxDimension; }

    void executeReal();
};

#endif // NORMALISINGLAYOUT_H
