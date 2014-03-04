#ifndef SCALINGLAYOUT_H
#define SCALINGLAYOUT_H

#include "layout.h"

class ScalingLayout : public NodeLayout
{
    Q_OBJECT
private:
    float _scale;

public:
    ScalingLayout(const ReadOnlyGraph& graph, NodePositions& positions) :
        NodeLayout(graph, positions), _scale(1.0f)
    {}

    void setScale(float _scale) { this->_scale = _scale; }
    float scale() { return _scale; }

    void executeReal(uint64_t);
};

#endif // SCALINGLAYOUT_H
