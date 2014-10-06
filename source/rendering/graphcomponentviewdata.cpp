#include "graphcomponentviewdata.h"

GraphComponentViewData::GraphComponentViewData() :
    _zoomDistance(50.0f),
    _textureSizeDivisor(1),
    _initialised(false)
{
    _camera.setPosition(QVector3D(0.0f, 0.0f, 50.0f));
    _camera.setViewTarget(QVector3D(0.0f, 0.0f, 0.0f));
    _camera.setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
}

GraphComponentViewData::GraphComponentViewData(const GraphComponentViewData& other) :
    _camera(other._camera),
    _zoomDistance(other._zoomDistance),
    _focusNodeId(other._focusNodeId),
    _textureSizeDivisor(other._textureSizeDivisor),
    _initialised(other._initialised)
{
}
