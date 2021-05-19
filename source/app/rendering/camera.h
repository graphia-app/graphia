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

#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>
#include <QSharedPointer>

#include "maths/frustum.h"
#include "maths/conicalfrustum.h"
#include "maths/line.h"
#include "maths/ray.h"

class CameraPrivate;

class QOpenGLShaderProgram;
using QOpenGLShaderProgramPtr = QSharedPointer<QOpenGLShaderProgram>;

class Camera
{
public:
    Camera();
    Camera(const Camera& other);
    Camera& operator=(const Camera& other) = default;

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

    bool valid() const { return _distance > 0.0f; }

    void setFocus(const QVector3D& focus);
    void setRotation(const QQuaternion& rotation);
    void setDistance(float distance);

    ProjectionType projectionType() const;

    void setOrthographicProjection(float left, float right,
                                   float bottom, float top,
                                   float nearPlane, float farPlane);

    void setPerspectiveProjection(float fieldOfView, float aspectRatio,
                                  float nearPlane, float farPlane);

    QRectF viewport() const { return _viewport; }
    void setViewport(const QRectF& viewport) { _viewport = viewport; }

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;
    void setProjectionMatrix(const QMatrix4x4& projectionMatrix);
    QMatrix4x4 viewProjectionMatrix() const;

    Ray rayForViewportCoordinates(int x, int y) const;
    Line3D lineForViewportCoordinates(int x, int y) const;
    Frustum frustumForViewportCoordinates(int x1, int y1, int x2, int y2) const;
    ConicalFrustum conicalFrustumForViewportCoordinates(int x, int y, int radius) const;

private:
    bool unproject(int x, int y, int z, QVector3D& result) const;

    void updatePerspectiveProjection();
    void updateOrthogonalProjection();

    QVector3D _focus;
    QQuaternion _rotation;
    float _distance = -1.0f;

    Camera::ProjectionType _projectionType = Camera::OrthogonalProjection;

    float _nearPlane = 0.1f;
    float _farPlane = 1024.0f;

    float _fieldOfView = 60.0f;
    float _aspectRatio = 1.0f;

    float _left = -0.5f;
    float _right = 0.5f;
    float _bottom = -0.5f;
    float _top = 0.5f;

    QRectF _viewport;

    mutable QMatrix4x4 _viewMatrix;
    mutable QMatrix4x4 _projectionMatrix;
    mutable QMatrix4x4 _viewProjectionMatrix;

    mutable bool _viewMatrixDirty = true;
    mutable bool _viewProjectionMatrixDirty = true;
};

#endif // CAMERA_H
