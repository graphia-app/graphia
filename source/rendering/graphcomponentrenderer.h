#ifndef GRAPHCOMPONENTRENDERER_H
#define GRAPHCOMPONENTRENDERER_H

#include "camera.h"
#include "primitives/cylinder.h"
#include "primitives/sphere.h"
#include "transition.h"
#include "../maths/boundingbox.h"
#include "../graph/graph.h"
#include "../graph/grapharray.h"

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
    void initialise(std::shared_ptr<GraphModel> graphModel, ComponentId componentId,
                    std::shared_ptr<SelectionManager> selectionManager,
                    GraphRenderer* graphRenderer);

    bool visible() { return _initialised && _visible; }
    void setVisible(bool visible);

    void cleanup();
    void synchronise();
    void update(float t);
    void setViewportSize(int viewportWidth, int viewportHeight);
    void setDimensions(const QRect& dimensions);

    bool transitionActive();

    int viewportWidth() const { return _viewportWidth; }
    int viewportHeight() const { return _viewportHeight; }

    int width() const { return _dimensions.width(); }
    int height() const { return _dimensions.height(); }

    float alpha() const { return _alpha; }
    bool fading() const { return _alpha < 1.0f; }
    void setAlpha(float alpha);

    QMatrix4x4 modelViewMatrix() const;
    QMatrix4x4 projectionMatrix() const;

    void moveFocusToNode(NodeId nodeId);
    void moveFocusToCentreOfComponent();
    void moveFocusToNodeClosestCameraVector();
    void moveFocusToPositionAndRadius(const QVector3D& position, float radius,
                                      const QQuaternion& rotation);

    ComponentId componentId() const { return _componentId; }
    const std::vector<NodeId>& nodeIds() const { return _nodeIds; }
    const std::vector<Edge> edges() const { return _edges; }

    NodeId focusNodeId();
    QVector3D focusPosition();
    void enableFocusTracking() { _trackFocus = true; }
    void disableFocusTracking() { _trackFocus = false; }

    bool trackingCentreOfComponent();
    bool autoZooming();

    void resetView();
    bool viewIsReset() { return _viewData.isReset(); }

    Camera* camera() { return &_viewData._camera; }
    const Camera* camera() const { return &_viewData._camera; }
    void zoom(float delta, bool doTransition);
    void zoomToDistance(float distance);

    void cloneViewDataFrom(const GraphComponentRenderer& other);
    void saveViewData() { _savedViewData = _viewData; }
    bool savedViewIsReset() { return _savedViewData.isReset(); }
    void restoreViewData();

    bool initialised() { return _initialised; }

    void freeze();
    void thaw();

    void updateTransition(float f);

    static float maxNodeDistanceFromPoint(const GraphModel& graphModel,
                                          const QVector3D& centre,
                                          const std::vector<NodeId>& nodeIds);

private:
    GraphRenderer* _graphRenderer;

    bool _initialised = false;
    bool _visible = false;

    bool _frozen = false;
    bool _cleanupWhenThawed = false;
    bool _synchroniseWhenThawed = false;

    struct ViewData
    {
        bool isReset() { return _focusNodeId.isNull() && _autoZooming; }

        Camera _camera;
        float _zoomDistance = 1.0f;
        bool _autoZooming = true;
        NodeId _focusNodeId;
        QVector3D _focusPosition;

        Camera _transitionStart;
        Camera _transitionEnd;
    };

    ViewData _viewData;
    ViewData _savedViewData;

    int _viewportWidth = 0;
    int _viewportHeight = 0;

    QRect _dimensions;

    float _alpha = 1.0f;

    bool _trackFocus = true;
    float _targetZoomDistance = 0.0f;
    Transition _zoomTransition;

    ComponentId _componentId;
    std::vector<NodeId> _nodeIds;
    std::vector<Edge> _edges;

    float _fovx = 0.0f;
    float _fovy = 0.0f;

    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<SelectionManager> _selectionManager;

    QMatrix4x4 subViewportMatrix() const;

    void centreNodeInViewport(NodeId nodeId, float cameraDistance = -1.0f);
    void centrePositionInViewport(const QVector3D& focus,
                                  float cameraDistance = -1.0f,
                                  // Odd constructor makes a null quaternion
                                  const QQuaternion rotation = QQuaternion(QVector4D()));

    float _entireComponentZoomDistance;
    float zoomDistanceForRadius(float radius);
    void updateFocusPosition();
    void updateEntireComponentZoomDistance();
};

#endif // GRAPHCOMPONENTRENDERER_H
