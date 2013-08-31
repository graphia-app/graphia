#ifndef SCALINGLAYOUT_H
#define SCALINGLAYOUT_H

#include "layout.h"

class ScalingLayout : public Layout
{
    Q_OBJECT
private:
    float _scale;

public:
    ScalingLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        Layout(graph, positions), _scale(1.0f)
    {}

    void setScale(float _scale) { this->_scale = _scale; }
    float scale() { return _scale; }

    void executeReal();
};

#endif // SCALINGLAYOUT_H
