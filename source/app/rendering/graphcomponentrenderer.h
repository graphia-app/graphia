#ifndef GRAPHCOMPONENTRENDERER_H
#define GRAPHCOMPONENTRENDERER_H

#include "camera.h"
#include "transition.h"
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

    GraphComponentRenderer() = default;
    GraphComponentRenderer(const GraphComponentRenderer& r) : _initialised(r._initialised),
        _visible(r._visible), _frozen(r._frozen), _cleanupWhenThawed(r._cleanupWhenThawed),
        _synchroniseWhenThawed(r._synchroniseWhenThawed), _viewData(r._viewData),
        _savedViewData(r._savedViewData), _viewportWidth(r._viewportWidth), _viewportHeight(r._viewportHeight),
        _dimensions(r._dimensions), _alpha(r._alpha), _trackFocus(r._trackFocus), _targetZoomDistance(r._targetZoomDistance),
        _moveFocusToCentreOfComponentLater(r._moveFocusToCentreOfComponentLater), _componentId(r._componentId),
        _nodeIds(r._nodeIds), _edges(r._edges), _fovx(r._fovx), _fovy(r._fovy), _graphModel(r._graphModel),
        _selectionManager(r._selectionManager), _entireComponentZoomDistance(r._entireComponentZoomDistance)
    {}

    GraphComponentRenderer& operator=(GraphComponentRenderer& other)
    {
        if(&other == this)
            return *this;

        _initialised = other._initialised;
        _visible = other._visible;
        _frozen = other._frozen;
        _cleanupWhenThawed = other._cleanupWhenThawed;
        _synchroniseWhenThawed = other._synchroniseWhenThawed;
        _viewData = other._viewData;
        _savedViewData = other._savedViewData;
        _viewportWidth = other._viewportWidth;
        _viewportHeight = other._viewportHeight;
        _dimensions = other._dimensions;
        _alpha = other._alpha;
        _trackFocus = other._trackFocus;
        _targetZoomDistance = other._targetZoomDistance;
        _moveFocusToCentreOfComponentLater = other._moveFocusToCentreOfComponentLater;
        _componentId = other._componentId;
        _nodeIds = other._nodeIds;
        _edges = other._edges;
        _fovx = other._fovx;
        _fovy = other._fovy;
        _graphModel = other._graphModel;
        _selectionManager = other._selectionManager;
        _entireComponentZoomDistance = other._entireComponentZoomDistance;

        return *this;
    }

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

    int width() const { return _dimensions.width(); }
    int height() const { return _dimensions.height(); }

    float alpha() const { return _alpha; }
    void setAlpha(float alpha);

    QMatrix4x4 modelViewMatrix() const;
    QMatrix4x4 projectionMatrix() const;
    QMatrix4x4 screenshotTileProjectionMatrix(float tileSize) const;

    void moveFocusToNode(NodeId nodeId, float radius = -1.0f);
    void moveSavedFocusToNode(NodeId nodeId, float radius = -1.0f);
    void moveFocusToCentreOfComponent();
    void moveFocusToNodeClosestCameraVector();
    void moveFocusToPositionAndRadius(const QVector3D& position, float radius,
                                      const QQuaternion& rotation);

    ComponentId componentId() const { return _componentId; }
    const std::vector<NodeId>& nodeIds() const { return _nodeIds; }
    const std::vector<const IEdge*> edges() const { return _edges; }

    NodeId focusNodeId() const;
    bool focusNodeIsVisible() const;
    QVector3D focusPosition() const;
    void enableFocusTracking() { _trackFocus = true; }
    void disableFocusTracking() { _trackFocus = false; }

    bool focusedOnNodeAtRadius(NodeId nodeId, float radius) const;

    bool trackingCentreOfComponent() const;

    void resetView();
    bool viewIsReset() const { return _viewData.isReset(); }

    Camera* camera() { return &_viewData._camera; }
    const Camera* camera() const { return &_viewData._camera; }
    void zoom(float delta, bool doTransition);
    void zoomToDistance(float distance);

    void cloneViewDataFrom(const GraphComponentRenderer& other);
    void saveViewData() { _savedViewData = _viewData; }
    bool savedViewIsReset() const { return _savedViewData.isReset(); }
    void restoreViewData();

    bool initialised() const { return _initialised; }

    void freeze();
    void thaw();

    bool transitionRequired();
    void computeTransition();
    void updateTransition(float f);

    static float maxNodeDistanceFromPoint(const GraphModel& graphModel,
                                          const QVector3D& centre,
                                          const std::vector<NodeId>& nodeIds);

private:
    GraphRenderer* _graphRenderer = nullptr;

    bool _initialised = false;
    bool _visible = false;

    bool _frozen = false;
    bool _cleanupWhenThawed = false;
    bool _synchroniseWhenThawed = false;

    struct ViewData
    {
        bool isReset() const { return _focusNodeId.isNull() && _autoZooming; }

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

    GraphModel* _graphModel = nullptr;
    SelectionManager* _selectionManager = nullptr;

    QMatrix4x4 subViewportMatrix() const;

    void centreNodeInViewport(NodeId nodeId, float zoomDistance = -1.0f);
    void centrePositionInViewport(const QVector3D& focus,
                                  float zoomDistance = -1.0f,
                                  // Odd constructor makes a null quaternion
                                  QQuaternion rotation = QQuaternion(QVector4D()));

    float _entireComponentZoomDistance = 0.0f;
    float zoomDistanceForRadius(float radius) const;
    void updateFocusPosition();
    float entireComponentZoomDistanceFor(NodeId nodeId);
    void updateEntireComponentZoomDistance();
};

#endif // GRAPHCOMPONENTRENDERER_H
