#include "camera.h"

#include "../maths/interpolation.h"
#include "../utils/utils.h"

#include <QOpenGLShaderProgram>
#include <QDebug>

Camera::Camera()
    : QObject(),
    _position(0.0f, 0.0f, 1.0f),
    _upVector(0.0f, 1.0f, 0.0f),
    _viewTarget(0.0f, 0.0f, 0.0f),
    _cameraToTarget(0.0f, 0.0f, -1.0f),
    _distance(1.0f),
    _projectionType(Camera::OrthogonalProjection),
    _nearPlane(0.1f),
    _farPlane(1024.0f),
    _fieldOfView(60.0f),
    _aspectRatio(1.0f),
    _left(-0.5),
    _right(0.5f),
    _bottom(-0.5f),
    _top(0.5f),
    _viewMatrixDirty(true),
    _viewProjectionMatrixDirty(true)
{
    updateOrthogonalProjection();
}

Camera::Camera(const Camera &other)
    : QObject(),
    _position(other._position),
    _upVector(other._upVector),
    _viewTarget(other._viewTarget),
    _cameraToTarget(other._cameraToTarget),
    _distance(other._distance),
    _projectionType(other._projectionType),
    _nearPlane(other._nearPlane),
    _farPlane(other._farPlane),
    _fieldOfView(other._fieldOfView),
    _aspectRatio(other._aspectRatio),
    _left(other._left),
    _right(other._right),
    _bottom(other._bottom),
    _top(other._top),
    _viewMatrixDirty(other._viewMatrixDirty),
    _viewProjectionMatrixDirty(other._viewProjectionMatrixDirty)
{
    switch(_projectionType)
    {
    default:
    case ProjectionType::OrthogonalProjection:
        updateOrthogonalProjection();
        break;

    case ProjectionType::PerspectiveProjection:
        updatePerspectiveProjection();
        break;
    }
}

Camera& Camera::operator=(const Camera& other)
{
    _position = other._position;
    _upVector = other._upVector;
    _viewTarget = other._viewTarget;
    _cameraToTarget = other._cameraToTarget;
    _distance = other._distance;
    _projectionType = other._projectionType;
    _nearPlane = other._nearPlane;
    _farPlane = other._farPlane;
    _fieldOfView = other._fieldOfView;
    _aspectRatio = other._aspectRatio;
    _left = other._left;
    _right = other._right;
    _bottom = other._bottom;
    _top = other._top;
    _viewMatrixDirty = other._viewMatrixDirty;
    _viewProjectionMatrixDirty = other._viewProjectionMatrixDirty;
    _projectionMatrix = other._projectionMatrix;

    return *this;
}

Camera::ProjectionType Camera::projectionType() const
{
    return _projectionType;
}

QVector3D Camera::position() const
{
    return _position;
}

void Camera::setPosition(const QVector3D& position)
{
    _position = position;
    _cameraToTarget = _viewTarget - position;
    _viewMatrixDirty = true;
}

void Camera::setPosition(const QVector3D &viewVector, float distance)
{
    setPosition(_viewTarget - (viewVector.normalized() * distance));
}

QVector3D Camera::upVector() const
{
    return _upVector;
}

void Camera::setViewTarget(const QVector3D& viewTarget)
{
    _viewTarget = viewTarget;
    _cameraToTarget = viewTarget - _position;
    _viewMatrixDirty = true;
}

void Camera::setViewTarget(const QVector3D& viewVector, float distance)
{
    setViewTarget(_position + (viewVector.normalized() * distance));
}

void Camera::setRotation(const QQuaternion& rotation)
{
    QMatrix4x4 m;

    m.setToIdentity();
    m.rotate(rotation);

    float distance = _cameraToTarget.length();

    auto viewVector = QVector3D(m.column(2));
    _upVector = QVector3D(m.column(1));

    _position = _viewTarget + (viewVector * distance);
    _cameraToTarget = _viewTarget - _position;

    _viewMatrixDirty = true;
    _viewProjectionMatrixDirty = true;
}


QVector3D Camera::viewTarget() const
{
    return _viewTarget;
}

QVector3D Camera::viewVector() const
{
    return _cameraToTarget;
}

void Camera::setOrthographicProjection(float left, float right,
                                       float bottom, float top,
                                       float nearPlane, float farPlane)
{
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    _projectionType = OrthogonalProjection;
    updateOrthogonalProjection();
}

void Camera::setPerspectiveProjection(float fieldOfView, float aspectRatio,
                                      float nearPlane, float farPlane)
{
    _fieldOfView = fieldOfView;
    _aspectRatio = aspectRatio;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    _projectionType = PerspectiveProjection;
    updatePerspectiveProjection();
}

QMatrix4x4 Camera::viewMatrix() const
{
    if(_viewMatrixDirty)
    {
        _viewMatrix.setToIdentity();
        _viewMatrix.lookAt(_position, _viewTarget, _upVector);
        _viewMatrixDirty = false;
    }
    return _viewMatrix;
}

void Camera::resetViewToIdentity()
{
    setPosition(QVector3D(0.0, 0.0, 0.0));
    setViewTarget(QVector3D(0.0, 0.0, 1.0));
    _upVector = QVector3D(0.0, 1.0, 0.0);

    _viewMatrixDirty = true;
}

QMatrix4x4 Camera::projectionMatrix() const
{
    return _projectionMatrix;
}

QMatrix4x4 Camera::viewProjectionMatrix() const
{
    if(_viewMatrixDirty || _viewProjectionMatrixDirty)
    {
        _viewProjectionMatrix = _projectionMatrix * viewMatrix();
        _viewProjectionMatrixDirty = false;
    }
    return _viewProjectionMatrix;
}

/*void Camera::translate(const QVector3D& vLocal, CameraTranslationOption option)
{
    // Calculate the amount to move by in world coordinates
    QVector3D vWorld;
    if(!qFuzzyIsNull(vLocal.x()))
    {
        // Calculate the vector for the local x axis
        QVector3D x = QVector3D::crossProduct(_cameraToTarget, _upVector).normalized();
        vWorld += vLocal.x() * x;
    }

    if(!qFuzzyIsNull(vLocal.y()))
        vWorld += vLocal.y() * _upVector;

    if(!qFuzzyIsNull(vLocal.z()))
        vWorld += vLocal.z() * _cameraToTarget.normalized();

    // Update the camera position using the calculated world vector
    _position += vWorld;

    // May be also update the view center coordinates
    if(option == TranslateViewCenter)
        _viewTarget += vWorld;

    // Refresh the camera -> view center vector
    _cameraToTarget = _viewTarget - _position;

    // Calculate a new up vector. We do this by:
    // 1) Calculate a new local x-direction vector from the cross product of the new
    //    camera to view center vector and the old up vector.
    // 2) The local x vector is the normal to the plane in which the new up vector
    //    must lay. So we can take the cross product of this normal and the new
    //    x vector. The new normal vector forms the last part of the orthonormal basis
    QVector3D x = QVector3D::crossProduct(_cameraToTarget, _upVector).normalized();
    _upVector = QVector3D::crossProduct(x, _cameraToTarget).normalized();

    _viewMatrixDirty = true;
    _viewProjectionMatrixDirty = true;
}*/

void Camera::translateWorld(const QVector3D& vWorld, CameraTranslationOption option)
{
    // Update the camera position using the calculated world vector
    _position += vWorld;

    // May be also update the view center coordinates
    if(option == TranslateViewCenter)
        _viewTarget += vWorld;

    // Refresh the camera -> view center vector
    _cameraToTarget = _viewTarget - _position;

    _viewMatrixDirty = true;
    _viewProjectionMatrixDirty = true;
}

QQuaternion Camera::rotation() const
{
    QMatrix4x4 m;

    m.setToIdentity();
    m.lookAt(_position, _viewTarget, _upVector);

    return Utils::matrixToQuaternion(m);
}

void Camera::setDistance(float distance)
{
    _distance = distance;

    _viewMatrixDirty = true;
    _viewProjectionMatrixDirty = true;
}

float Camera::distance()
{
    return _distance;
}

void Camera::rotate(const QQuaternion& q)
{
    _upVector = q.rotatedVector(_upVector);
    _cameraToTarget = q.rotatedVector(_cameraToTarget);
    _viewTarget = _position + _cameraToTarget;
    _viewMatrixDirty = true;
    _viewProjectionMatrixDirty = true;
}

void Camera::rotateAboutViewTarget(const QQuaternion& q)
{
    _upVector = q.rotatedVector(_upVector);
    _cameraToTarget = q.rotatedVector(_cameraToTarget);
    _position = _viewTarget - _cameraToTarget;
    _viewMatrixDirty = true;
    _viewProjectionMatrixDirty = true;
}

bool Camera::unproject(int x, int y, int z, QVector3D& result)
{
    QMatrix4x4 A = projectionMatrix() * viewMatrix();

    bool invertable;
    QMatrix4x4 m = A.inverted(&invertable);
    if(!invertable)
        return false;

    y = _viewportHeight - y;

    QVector4D normalisedCoordinates;
    normalisedCoordinates.setX((x / (float)_viewportWidth) * 2.0f - 1.0f);
    normalisedCoordinates.setY((y / (float)_viewportHeight) * 2.0f - 1.0f);
    normalisedCoordinates.setZ(2.0f * z - 1.0f);
    normalisedCoordinates.setW(1.0f);

    QVector4D o = m * normalisedCoordinates;
    if(o.w() == 0.0f)
        return false;

    o.setW(1.0f / o.w());

    result.setX(o.x() * o.w());
    result.setY(o.y() * o.w());
    result.setZ(o.z() * o.w());

    return true;
}

const Ray Camera::rayForViewportCoordinates(int x, int y)
{
    Line3D line = lineForViewportCoordinates(x, y);

    return Ray(line.start(), line.dir());
}

Line3D Camera::lineForViewportCoordinates(int x, int y)
{
    QVector3D start;
    QVector3D end;

    unproject(x, y, 0.0, start);
    unproject(x, y, 1.0, end);

    return Line3D(start, end);
}

Frustum Camera::frustumForViewportCoordinates(int x1, int y1, int x2, int y2)
{
    int minX, maxX, minY, maxY;

    if(x1 < x2)
    {
        minX = x1;
        maxX = x2;
    }
    else
    {
        minX = x2;
        maxX = x1;
    }

    if(y1 < y2)
    {
        minY = y1;
        maxY = y2;
    }
    else
    {
        minY = y2;
        maxY = y1;
    }

    // Lines in clockwise order around view vector
    Line3D line1 = lineForViewportCoordinates(minX, minY);
    Line3D line2 = lineForViewportCoordinates(maxX, minY);
    Line3D line3 = lineForViewportCoordinates(maxX, maxY);
    Line3D line4 = lineForViewportCoordinates(minX, maxY);

    return Frustum(line1, line2, line3, line4);
}

ConicalFrustum Camera::conicalFrustumForViewportCoordinates(int x, int y, int radius)
{
    Line3D centreLine = lineForViewportCoordinates(x, y);
    Line3D surfaceLine = lineForViewportCoordinates(x + radius, y);

    return ConicalFrustum(centreLine, surfaceLine);
}
