#ifndef GRAPHCOMPONENTVIEWDATA_H
#define GRAPHCOMPONENTVIEWDATA_H

#include "camera.h"
#include "../graph/graph.h"

class GraphComponentViewData
{
public:
    GraphComponentViewData();
    GraphComponentViewData(const GraphComponentViewData& other);

    Camera _camera;
    float _zoomDistance;
    bool _autoZooming;
    NodeId _focusNodeId;
    QVector3D _focusPosition;

    int _textureSizeDivisor;

    bool _initialised;
};

#endif // GRAPHCOMPONENTVIEWDATA_H
