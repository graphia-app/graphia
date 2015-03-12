#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>

#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>
#include <QSharedPointer>

#include "../maths/frustum.h"
#include "../maths/conicalfrustum.h"
#include "../maths/line.h"
#include "../maths/ray.h"

class CameraPrivate;

class QOpenGLShaderProgram;
typedef QSharedPointer<QOpenGLShaderProgram> QOpenGLShaderProgramPtr;

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera();
    Camera(const Camera& other);
    Camera& operator=(const Camera& other);

    enum ProjectionType
    {
        OrthogonalProjection,
        PerspectiveProjection
    };

    enum CameraTranslationOption
    {
        TranslateViewCenter,
        DontTranslateViewCenter
    };

    QVector3D position() const;
    QVector3D upVector() const;
    QVector3D viewTarget() const;

    QVector3D viewVector() const;

    ProjectionType projectionType() const;

    void setOrthographicProjection(float left, float right,
                                   float bottom, float top,
                                   float nearPlane, float farPlane);

    void setPerspectiveProjection(float fieldOfView, float aspect,
                                  float nearPlane, float farPlane);

    void setViewportWidth(const float viewportWidth) { _viewportWidth = viewportWidth; }
    void setViewportHeight(const float viewportHeight) { _viewportHeight = viewportHeight; }

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;
    QMatrix4x4 viewProjectionMatrix() const;

    const Ray rayForViewportCoordinates(int x, int y);
    Line3D lineForViewportCoordinates(int x, int y);
    Frustum frustumForViewportCoordinates(int x1, int y1, int x2, int y2);
    ConicalFrustum conicalFrustumForViewportCoordinates(int x, int y, int radius);

    void setPosition(const QVector3D& position);
    void setPosition(const QVector3D& viewVector, float distance);
    void setViewTarget(const QVector3D& viewTarget);
    void setViewTarget(const QVector3D& viewVector, float distance);

    void setRotation(const QQuaternion& rotation);
    QQuaternion rotation() const;

    void setDistance(float distance);
    float distance();

    void resetViewToIdentity();

    /*// Translate relative to camera orientation axes
    void translate(const QVector3D& vLocal, CameraTranslationOption option = TranslateViewCenter);*/

    // Translate relative to world axes
    void translateWorld(const QVector3D& vWorld, CameraTranslationOption option = TranslateViewCenter);

    void rotate(const QQuaternion& q);
    void rotateAboutViewTarget(const QQuaternion& q);

protected:
    Q_DECLARE_PRIVATE(Camera)

private:
    bool unproject(int x, int y, int z, QVector3D& result);

    inline void updatePerspectiveProjection()
    {
        _projectionMatrix.setToIdentity();

        if(_fieldOfView > 0.0f && _aspectRatio > 0.0f && _nearPlane < _farPlane)
            _projectionMatrix.perspective(_fieldOfView, _aspectRatio, _nearPlane, _farPlane);

        _viewProjectionMatrixDirty = true;
    }

    inline void updateOrthogonalProjection()
    {
        _projectionMatrix.setToIdentity();

        if(_left < _right && _bottom < _top && _nearPlane < _farPlane)
            _projectionMatrix.ortho(_left, _right, _bottom, _top, _nearPlane, _farPlane);

        _viewProjectionMatrixDirty = true;
    }

    QVector3D _position;
    QVector3D _upVector;
    QVector3D _viewTarget;

    QVector3D _cameraToTarget; // The vector from the camera position to the view center
    float _distance;

    Camera::ProjectionType _projectionType;

    float _nearPlane;
    float _farPlane;

    float _fieldOfView;
    float _aspectRatio;

    float _left;
    float _right;
    float _bottom;
    float _top;

    int _viewportWidth;
    int _viewportHeight;

    mutable QMatrix4x4 _viewMatrix;
    mutable QMatrix4x4 _projectionMatrix;
    mutable QMatrix4x4 _viewProjectionMatrix;

    mutable bool _viewMatrixDirty;
    mutable bool _viewProjectionMatrixDirty;
};

#endif // CAMERA_H
