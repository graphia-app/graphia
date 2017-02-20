#ifndef SCALINGLAYOUT_H
#define SCALINGLAYOUT_H

#include "layout.h"

class ScalingLayout : public Layout
{
    Q_OBJECT
private:
    float _scale = 1.0f;

public:
    ScalingLayout(const IGraphComponent& graphComponent, NodePositions& positions) :
        Layout(graphComponent, positions)
    {}

    void setScale(float scale) { _scale = scale; }
    float scale() { return _scale; }

    void executeReal(bool);
};

#endif // SCALINGLAYOUT_H
