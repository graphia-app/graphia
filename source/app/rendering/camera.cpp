/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "camera.h"

#include "maths/interpolation.h"

#include <QOpenGLShaderProgram>
#include <QDebug>

Camera::Camera()
{
    updateOrthogonalProjection();
}

Camera::Camera(const Camera& other) :
    _focus(other._focus),
    _rotation(other._rotation),
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
    _viewport(other._viewport),
    _viewMatrix(other._viewMatrix),
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

    _viewProjectionMatrix = other._viewProjectionMatrix;
}

QVector3D Camera::viewVector() const
{
    return -QVector3D(viewMatrix().row(2));
}

QVector3D Camera::position() const
{
    return _focus - (viewVector() * _distance);
}

QVector3D Camera::focus() const
{
    return _focus;
}

QQuaternion Camera::rotation() const
{
    return _rotation;
}

float Camera::distance() const
{
    return _distance;
}

void Camera::setFocus(const QVector3D& focus)
{
    if(_focus != focus)
    {
        _focus = focus;
        _viewMatrixDirty = true;
    }
}

void Camera::setDistance(float distance)
{
    if(_distance != distance)
    {
        _distance = distance;
        _viewMatrixDirty = true;
    }
}

void Camera::setRotation(const QQuaternion& rotation)
{
    if(_rotation != rotation)
    {
        _rotation = rotation;
        _viewMatrixDirty = true;
    }
}

Camera::ProjectionType Camera::projectionType() const
{
    return _projectionType;
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
        _viewMatrix.rotate(_rotation.conjugated());

        auto viewVector = -QVector3D(_viewMatrix.row(2));

        QVector3D eye = _focus - (viewVector * _distance);
        _viewMatrix.translate(-eye);

        _viewMatrixDirty = false;
    }

    return _viewMatrix;
}

QMatrix4x4 Camera::projectionMatrix() const
{
    return _projectionMatrix;
}

void Camera::setProjectionMatrix(const QMatrix4x4& projectionMatrix)
{
    _projectionMatrix = projectionMatrix;
    _viewProjectionMatrixDirty = true;
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

bool Camera::unproject(int x, int y, int z, QVector3D& result) const
{
    QMatrix4x4 A = projectionMatrix() * viewMatrix();

    bool invertable = false;
    QMatrix4x4 m = A.inverted(&invertable);
    if(!invertable)
        return false;

    auto invertedY = static_cast<float>(_viewport.height()) - static_cast<float>(y);

    QVector4D normalisedCoordinates;
    normalisedCoordinates.setX((static_cast<float>(x) /
        static_cast<float>(_viewport.width())) * 2.0f - 1.0f);
    normalisedCoordinates.setY((static_cast<float>(invertedY) /
        static_cast<float>(_viewport.height())) * 2.0f - 1.0f);
    normalisedCoordinates.setZ(2.0f * static_cast<float>(z) - 1.0f);
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

void Camera::updatePerspectiveProjection()
{
    _projectionMatrix.setToIdentity();

    if(_fieldOfView > 0.0f && _aspectRatio > 0.0f && _nearPlane < _farPlane)
        _projectionMatrix.perspective(_fieldOfView, _aspectRatio, _nearPlane, _farPlane);

    _viewProjectionMatrixDirty = true;
}

void Camera::updateOrthogonalProjection()
{
    _projectionMatrix.setToIdentity();

    if(_left < _right && _bottom < _top && _nearPlane < _farPlane)
        _projectionMatrix.ortho(_left, _right, _bottom, _top, _nearPlane, _farPlane);

    _viewProjectionMatrixDirty = true;
}

Ray Camera::rayForViewportCoordinates(int x, int y) const
{
    Line3D line = lineForViewportCoordinates(x, y);

    return {line.start(), line.dir()};
}

Line3D Camera::lineForViewportCoordinates(int x, int y) const
{
    QVector3D start;
    QVector3D end;

    unproject(x, y, 0.0, start);
    unproject(x, y, 1.0, end);

    return {start, end};
}

Frustum Camera::frustumForViewportCoordinates(int x1, int y1, int x2, int y2) const
{
    int minX = 0, maxX = 0, minY = 0, maxY = 0;

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

    return {line1, line2, line3, line4};
}

ConicalFrustum Camera::conicalFrustumForViewportCoordinates(int x, int y, int radius) const
{
    Line3D centreLine = lineForViewportCoordinates(x, y);
    Line3D surfaceLine = lineForViewportCoordinates(x + radius, y);

    return {centreLine, surfaceLine};
}
