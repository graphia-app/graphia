#ifndef SCALINGLAYOUT_H
#define SCALINGLAYOUT_H

#include "layout.h"

class ScalingLayout : public NodeLayout
{
    Q_OBJECT
private:
    float _scale;

public:
    ScalingLayout(std::shared_ptr<const ReadOnlyGraph> graph,
                  std::shared_ptr<NodePositions> positions) :
        NodeLayout(graph, positions), _scale(1.0f)
    {}

    void setScale(float _scale) { this->_scale = _scale; }
    float scale() { return _scale; }

    void executeReal(uint64_t);
};

#endif // SCALINGLAYOUT_H
