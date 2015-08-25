#ifndef CAMERA_H
#define CAMERA_H

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

class Camera
{
public:
    Camera();
    Camera(const Camera& other);
    Camera& operator=(const Camera& other);

    enum ProjectionType
    {
        OrthogonalProjection,
        PerspectiveProjection
    };

    QVector3D viewVector() const;
    QVector3D position() const;
    QVector3D focus() const;
    QQuaternion rotation() const;
    float distance() const;

    void setFocus(const QVector3D& focus);
    void setRotation(const QQuaternion& rotation);
    void setDistance(float distance);

    void translate(const QVector3D& translation);
    void rotate(const QQuaternion& q);

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

    QVector3D _focus;
    QQuaternion _rotation;
    float _distance = 1.0f;

    Camera::ProjectionType _projectionType = Camera::OrthogonalProjection;

    float _nearPlane = 0.1f;
    float _farPlane = 1024.0f;

    float _fieldOfView = 60.0f;
    float _aspectRatio = 1.0f;

    float _left = -0.5f;
    float _right = 0.5f;
    float _bottom = -0.5f;
    float _top = 0.5f;

    int _viewportWidth;
    int _viewportHeight;

    mutable QMatrix4x4 _viewMatrix;
    mutable QMatrix4x4 _projectionMatrix;
    mutable QMatrix4x4 _viewProjectionMatrix;

    mutable bool _viewMatrixDirty = true;
    mutable bool _viewProjectionMatrixDirty = true;
};

#endif // CAMERA_H
