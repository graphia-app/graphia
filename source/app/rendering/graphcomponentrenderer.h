/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRAPHCOMPONENTRENDERER_H
#define GRAPHCOMPONENTRENDERER_H

#include "camera.h"
#include "transition.h"
#include "projection.h"

#include "maths/boundingbox.h"

#include "shared/graph/igraph.h"
#include "shared/graph/grapharray.h"

#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>
#include <QColor>
#include <QRect>

#include <memory>
#include <vector>

class GraphRenderer;
class GraphModel;
class SelectionManager;
class Camera;
class Octree;

class GraphComponentRenderer
{
public:
    static const float MINIMUM_ZOOM_DISTANCE;
    static const float COMFORTABLE_ZOOM_RADIUS;

    struct CameraAndLighting
    {
        Camera _camera;
        float _lightScale = 0.0f;
    };

    GraphComponentRenderer() = default;

    void initialise(GraphModel* graphModel, ComponentId componentId,
                    SelectionManager* selectionManager,
                    GraphRenderer* graphRenderer);

    bool visible() const { return _initialised && _visible; }
    void setVisible(bool visible);

    void cleanup();
    void synchronise();
    void update(float t);
    void setViewportSize(int viewportWidth, int viewportHeight);
    void setDimensions(const QRectF& dimensions);

    bool transitionActive();

    int viewportWidth() const { return _viewportWidth; }
    int viewportHeight() const { return _viewportHeight; }

    int width() const { return static_cast<int>(_dimensions.width()); }
    int height() const { return static_cast<int>(_dimensions.height()); }

    Projection projection() const { return _projection; }
    void setProjection(Projection projection) { _projection = projection; }

    float alpha() const { return _alpha; }
    void setAlpha(float alpha);

    QMatrix4x4 modelViewMatrix() const;
    QMatrix4x4 projectionMatrix() const;

    void moveFocusToNode(NodeId nodeId, float radius = -1.0f);
    void moveSavedFocusToNode(NodeId nodeId, float radius = -1.0f);
    void moveFocusToCentreOfComponent();
    void moveFocusToNodeClosestCameraVector();
    void moveFocusToNodes(const std::vector<NodeId>& nodeIds, const QQuaternion& rotation);
    void moveFocusTo(const QVector3D& position, float radius, const QQuaternion& rotation);
    void doProjectionTransition();

    ComponentId componentId() const { return _componentId; }
    const std::vector<NodeId>& nodeIds() const { return _nodeIds; }
    std::vector<const IEdge*> edges() const { return _edges; }

    NodeId focusNodeId() const;
    bool focusNodeIsVisible() const;
    QVector3D focusPosition() const;
    void enableFocusTracking() { _trackFocus = true; }
    void disableFocusTracking() { _trackFocus = false; }

    bool focusedOnNodeAtRadius(NodeId nodeId, float radius) const;

    bool trackingCentreOfComponent() const;

    void resetView();
    bool viewIsReset() const { return _viewData.isReset(); }

    Camera* camera() { return &_viewData.camera(); }
    const Camera* camera() const { return &_viewData.camera(); }
    float lightScale() const { return _viewData.lightScale(); }
    CameraAndLighting* cameraAndLighting() { return &_viewData._cameraAndLighting; }
    const CameraAndLighting* cameraAndLighting() const { return &_viewData._cameraAndLighting; }

    void zoom(float delta, bool doTransition);
    void zoomToDistance(float distance);

    void cloneViewDataFrom(const GraphComponentRenderer& other);
    void saveViewData() { _savedViewData = _viewData; }
    bool savedViewIsReset() const { return _savedViewData.isReset(); }
    void restoreViewData();

    bool initialised() const { return _initialised; }

    void freeze();
    void thaw();

    bool componentIsValid() const { return !componentId().isNull() && !_frozen; }

    bool transitionRequired() const;
    void computeTransition();
    void updateTransition(float f);

    static float maxNodeDistanceFromPoint(const GraphModel& graphModel,
        const QVector3D& centre, const std::vector<NodeId>& nodeIds);

    static QMatrix4x4 subViewportMatrix(QRectF sub, QRect viewport);

private:
    GraphRenderer* _graphRenderer = nullptr;

    bool _initialised = false;
    bool _visible = false;

    bool _frozen = false;
    bool _cleanupWhenThawed = false;
    bool _synchroniseWhenThawed = false;

    struct ViewData
    {
        void reset() { *this = {}; }
        bool isReset() const { return _focusNodeId.isNull() && _autoZooming; }

        CameraAndLighting _cameraAndLighting;

        Camera& camera() { return _cameraAndLighting._camera; }
        const Camera& camera() const { return _cameraAndLighting._camera; }
        float lightScale() const { return _cameraAndLighting._lightScale; }

        float _zoomDistance = 1.0f;
        bool _autoZooming = true;
        NodeId _focusNodeId;
        QVector3D _componentCentre;

        CameraAndLighting _transitionStart;
        CameraAndLighting _transitionEnd;
    };

    ViewData _viewData;
    ViewData _savedViewData;

    int _viewportWidth = 0;
    int _viewportHeight = 0;

    Projection _projection = Projection::Perspective;

    QRectF _dimensions;

    float _alpha = 1.0f;

    bool _trackFocus = true;
    float _targetZoomDistance = 0.0f;
    Transition _zoomTransition;
    bool _moveFocusToCentreOfComponentLater = false;

    ComponentId _componentId;
    std::vector<NodeId> _nodeIds;
    std::vector<const IEdge*> _edges;

    float _fovx = 0.0f;
    float _fovy = 0.0f;

    float _entireComponentZoomDistance = 0.0f;
    float _orthoCameraDistance = 0.0f;
    float _maxDistanceFromFocus = 0.0f;

    GraphModel* _graphModel = nullptr;
    SelectionManager* _selectionManager = nullptr;

    // QQuaternion doesn't have a constructor that creates a null QQuaternion (i.e. 0,0,0,0)
    static QQuaternion nullQuaternion() { return QQuaternion(QVector4D()); }

    void updateCameraProjection(Camera& camera);

    void centreNodeInViewport(NodeId nodeId, float zoomDistance = -1.0f,
        QQuaternion rotation = nullQuaternion());
    void centrePositionInViewport(const QVector3D& focus, float zoomDistance = -1.0f,
        QQuaternion rotation = nullQuaternion());

    float zoomDistanceForRadius(float radius, Projection projection = Projection::Unset) const;
    float maxDistanceFor(NodeId nodeId,
        const std::vector<NodeId>* nodeIds = nullptr) const;
    float entireComponentZoomDistanceFor(NodeId nodeId,
        const std::vector<NodeId>* nodeIds = nullptr,
        Projection projection = Projection::Unset) const;

    void updateCentreAndZoomDistance(const std::vector<NodeId>* nodeIds = nullptr);
};

#endif // GRAPHCOMPONENTRENDERER_H
