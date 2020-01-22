#ifndef SCALINGLAYOUT_H
#define SCALINGLAYOUT_H

#include "layout.h"

class ScalingLayout : public Layout
{
    Q_OBJECT
private:
    float _scale = 1.0f;

public:
    ScalingLayout(const IGraphComponent& graphComponent, NodeLayoutPositions& positions) :
        Layout(graphComponent, positions)
    {}

    void setScale(float scale) { _scale = scale; }
    float scale() { return _scale; }

    void execute(bool) override;
};

#endif // SCALINGLAYOUT_H
