#include "camera.h"

#include "../maths/interpolation.h"

#include <QOpenGLShaderProgram>
#include <QDebug>

Camera::Camera()
    : QObject(),
    m_position(0.0f, 0.0f, 1.0f),
    m_upVector(0.0f, 1.0f, 0.0f),
    m_viewTarget(0.0f, 0.0f, 0.0f),
    m_cameraToTarget(0.0f, 0.0f, -1.0f),
    m_projectionType(Camera::OrthogonalProjection),
    m_nearPlane(0.1f),
    m_farPlane(1024.0f),
    m_fieldOfView(60.0f),
    m_aspectRatio(1.0f),
    m_left(-0.5),
    m_right(0.5f),
    m_bottom(-0.5f),
    m_top(0.5f),
    m_viewMatrixDirty(true),
    m_viewProjectionMatrixDirty(true)
{
    updateOrthogonalProjection();
}

Camera::Camera(const Camera &other)
    : QObject(),
    m_position(other.m_position),
    m_upVector(other.m_upVector),
    m_viewTarget(other.m_viewTarget),
    m_cameraToTarget(other.m_cameraToTarget),
    m_projectionType(other.m_projectionType),
    m_nearPlane(other.m_nearPlane),
    m_farPlane(other.m_farPlane),
    m_fieldOfView(other.m_fieldOfView),
    m_aspectRatio(other.m_aspectRatio),
    m_left(other.m_left),
    m_right(other.m_right),
    m_bottom(other.m_bottom),
    m_top(other.m_top),
    m_viewMatrixDirty(other.m_viewMatrixDirty),
    m_viewProjectionMatrixDirty(other.m_viewProjectionMatrixDirty)
{
    switch(m_projectionType)
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
    m_position = other.m_position;
    m_upVector = other.m_upVector;
    m_viewTarget = other.m_viewTarget;
    m_cameraToTarget = other.m_cameraToTarget;
    m_projectionType = other.m_projectionType;
    m_nearPlane = other.m_nearPlane;
    m_farPlane = other.m_farPlane;
    m_fieldOfView = other.m_fieldOfView;
    m_aspectRatio = other.m_aspectRatio;
    m_left = other.m_left;
    m_right = other.m_right;
    m_bottom = other.m_bottom;
    m_top = other.m_top;
    m_viewMatrixDirty = other.m_viewMatrixDirty;
    m_viewProjectionMatrixDirty = other.m_viewProjectionMatrixDirty;
    m_projectionMatrix = other.m_projectionMatrix;

    return *this;
}

Camera::ProjectionType Camera::projectionType() const
{
    return m_projectionType;
}

QVector3D Camera::position() const
{
    return m_position;
}

void Camera::setPosition( const QVector3D& position )
{
    m_position = position;
    m_cameraToTarget = m_viewTarget - position;
    m_viewMatrixDirty = true;
}

void Camera::setUpVector( const QVector3D& upVector )
{
    m_upVector = upVector;
    m_viewMatrixDirty = true;
}

QVector3D Camera::upVector() const
{
    return m_upVector;
}

void Camera::setViewTarget( const QVector3D& viewTarget )
{
    m_viewTarget = viewTarget;
    m_cameraToTarget = viewTarget - m_position;
    m_viewMatrixDirty = true;
}

QVector3D Camera::viewTarget() const
{
    return m_viewTarget;
}

QVector3D Camera::viewVector() const
{
    return m_cameraToTarget;
}

void Camera::setOrthographicProjection( float left, float right,
                                        float bottom, float top,
                                        float nearPlane, float farPlane )
{
    m_left = left;
    m_right = right;
    m_bottom = bottom;
    m_top = top;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_projectionType = OrthogonalProjection;
    updateOrthogonalProjection();
}

void Camera::setPerspectiveProjection( float fieldOfView, float aspectRatio,
                                       float nearPlane, float farPlane )
{
    m_fieldOfView = fieldOfView;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_projectionType = PerspectiveProjection;
    updatePerspectiveProjection();
}

void Camera::setNearPlane( const float& nearPlane )
{
    if ( qFuzzyCompare( m_nearPlane, nearPlane ) )
        return;
    m_nearPlane = nearPlane;
    if ( m_projectionType == PerspectiveProjection )
        updatePerspectiveProjection();
}

float Camera::nearPlane() const
{
    return m_nearPlane;
}

void Camera::setFarPlane( const float& farPlane )
{
    if ( qFuzzyCompare( m_farPlane, farPlane ) )
        return;
    m_farPlane = farPlane;
    if ( m_projectionType == PerspectiveProjection )
        updatePerspectiveProjection();
}

float Camera::farPlane() const
{
    return m_farPlane;
}

void Camera::setFieldOfView( const float& fieldOfView )
{
    if ( qFuzzyCompare( m_fieldOfView, fieldOfView ) )
        return;
    m_fieldOfView = fieldOfView;
    if ( m_projectionType == PerspectiveProjection )
        updatePerspectiveProjection();
}

float Camera::fieldOfView() const
{
    return m_fieldOfView;
}

void Camera::setAspectRatio( const float& aspectRatio )
{
    if ( qFuzzyCompare( m_aspectRatio, aspectRatio ) )
        return;
    m_aspectRatio = aspectRatio;
    if ( m_projectionType == PerspectiveProjection )
        updatePerspectiveProjection();
}

float Camera::aspectRatio() const
{
    return m_aspectRatio;
}

void Camera::setLeft( const float& left )
{
    if ( qFuzzyCompare( m_left, left ) )
        return;
    m_left = left;
    if ( m_projectionType == OrthogonalProjection )
        updateOrthogonalProjection();
}

float Camera::left() const
{
    return m_left;
}

void Camera::setRight( const float& right )
{
    if ( qFuzzyCompare( m_right, right ) )
        return;
    m_right = right;
    if ( m_projectionType == OrthogonalProjection )
        updateOrthogonalProjection();
}

float Camera::right() const
{
    return m_right;
}

void Camera::setBottom( const float& bottom )
{
    if ( qFuzzyCompare( m_bottom, bottom ) )
        return;
    m_bottom = bottom;
    if ( m_projectionType == OrthogonalProjection )
        updateOrthogonalProjection();
}

float Camera::bottom() const
{
    return m_bottom;
}

void Camera::setTop( const float& top )
{
    if ( qFuzzyCompare( m_top, top ) )
        return;
    m_top = top;
    if ( m_projectionType == OrthogonalProjection )
        updateOrthogonalProjection();
}

float Camera::top() const
{
    return m_top;
}

QMatrix4x4 Camera::viewMatrix() const
{
    if ( m_viewMatrixDirty )
    {
        m_viewMatrix.setToIdentity();
        m_viewMatrix.lookAt( m_position, m_viewTarget, m_upVector );
        m_viewMatrixDirty = false;
    }
    return m_viewMatrix;
}

void Camera::resetViewToIdentity()
{
    setPosition( QVector3D( 0.0, 0.0, 0.0) );
    setViewTarget( QVector3D( 0.0, 0.0, 1.0) );
    setUpVector( QVector3D( 0.0, 1.0, 0.0) );
}

QMatrix4x4 Camera::projectionMatrix() const
{
    return m_projectionMatrix;
}

QMatrix4x4 Camera::viewProjectionMatrix() const
{
    if ( m_viewMatrixDirty || m_viewProjectionMatrixDirty )
    {
        m_viewProjectionMatrix = m_projectionMatrix * viewMatrix();
        m_viewProjectionMatrixDirty = false;
    }
    return m_viewProjectionMatrix;
}

void Camera::translate( const QVector3D& vLocal, CameraTranslationOption option )
{
    // Calculate the amount to move by in world coordinates
    QVector3D vWorld;
    if ( !qFuzzyIsNull( vLocal.x() ) )
    {
        // Calculate the vector for the local x axis
        QVector3D x = QVector3D::crossProduct( m_cameraToTarget, m_upVector ).normalized();
        vWorld += vLocal.x() * x;
    }

    if ( !qFuzzyIsNull( vLocal.y() ) )
        vWorld += vLocal.y() * m_upVector;

    if ( !qFuzzyIsNull( vLocal.z() ) )
        vWorld += vLocal.z() * m_cameraToTarget.normalized();

    // Update the camera position using the calculated world vector
    m_position += vWorld;

    // May be also update the view center coordinates
    if ( option == TranslateViewCenter )
        m_viewTarget += vWorld;

    // Refresh the camera -> view center vector
    m_cameraToTarget = m_viewTarget - m_position;

    // Calculate a new up vector. We do this by:
    // 1) Calculate a new local x-direction vector from the cross product of the new
    //    camera to view center vector and the old up vector.
    // 2) The local x vector is the normal to the plane in which the new up vector
    //    must lay. So we can take the cross product of this normal and the new
    //    x vector. The new normal vector forms the last part of the orthonormal basis
    QVector3D x = QVector3D::crossProduct( m_cameraToTarget, m_upVector ).normalized();
    m_upVector = QVector3D::crossProduct( x, m_cameraToTarget ).normalized();

    m_viewMatrixDirty = true;
    m_viewProjectionMatrixDirty = true;
}

void Camera::translateWorld(const QVector3D& vWorld, CameraTranslationOption option )
{
    // Update the camera position using the calculated world vector
    m_position += vWorld;

    // May be also update the view center coordinates
    if ( option == TranslateViewCenter )
        m_viewTarget += vWorld;

    // Refresh the camera -> view center vector
    m_cameraToTarget = m_viewTarget - m_position;

    m_viewMatrixDirty = true;
    m_viewProjectionMatrixDirty = true;
}

QQuaternion Camera::tiltRotation( const float& angle ) const
{
    QVector3D xBasis = QVector3D::crossProduct( m_upVector, m_cameraToTarget.normalized() ).normalized();
    return QQuaternion::fromAxisAndAngle( xBasis, -angle );
}

QQuaternion Camera::panRotation( const float& angle ) const
{
    return QQuaternion::fromAxisAndAngle( m_upVector, angle );
}

QQuaternion Camera::rollRotation( const float& angle ) const
{
    return QQuaternion::fromAxisAndAngle( m_cameraToTarget, -angle );
}

void Camera::tilt( const float& angle )
{
    QQuaternion q = tiltRotation( angle );
    rotate( q );
}

void Camera::pan( const float& angle )
{
    QQuaternion q = panRotation( -angle );
    rotate( q );
}

void Camera::roll( const float& angle )
{
    QQuaternion q = rollRotation( -angle );
    rotate( q );
}

void Camera::tiltAboutViewCenter( const float& angle )
{
    QQuaternion q = tiltRotation( -angle );
    rotateAboutViewTarget( q );
}

void Camera::panAboutViewCenter( const float& angle )
{
    QQuaternion q = panRotation( angle );
    rotateAboutViewTarget( q );
}

void Camera::rollAboutViewCenter( const float& angle )
{
    QQuaternion q = rollRotation( angle );
    rotateAboutViewTarget( q );
}

void Camera::rotate( const QQuaternion& q )
{
    m_upVector = q.rotatedVector( m_upVector );
    m_cameraToTarget = q.rotatedVector( m_cameraToTarget );
    m_viewTarget = m_position + m_cameraToTarget;
    m_viewMatrixDirty = true;
    m_viewProjectionMatrixDirty = true;
}

void Camera::rotateAboutViewTarget( const QQuaternion& q )
{
    m_upVector = q.rotatedVector( m_upVector );
    m_cameraToTarget = q.rotatedVector( m_cameraToTarget );
    m_position = m_viewTarget - m_cameraToTarget;
    m_viewMatrixDirty = true;
    m_viewProjectionMatrixDirty = true;
}

void Camera::setStandardUniforms( const QOpenGLShaderProgramPtr& program, const QMatrix4x4& mm ) const
{
    QMatrix4x4 modelViewMatrix = viewMatrix() * mm;
    QMatrix3x3 normalMatrix = modelViewMatrix.normalMatrix();
    QMatrix4x4 mvp = viewProjectionMatrix() * mm;

    program->setUniformValue( "modelViewMatrix", modelViewMatrix );
    program->setUniformValue( "normalMatrix", normalMatrix );
    program->setUniformValue( "projectionMatrix", projectionMatrix() );
    program->setUniformValue( "mvp", mvp );
}


bool Camera::unproject(int x, int y, int z, QVector3D& result)
{
    QMatrix4x4 A = projectionMatrix() * viewMatrix();

    bool invertable;
    QMatrix4x4 m = A.inverted(&invertable);
    if(!invertable)
        return false;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    y = viewport[3] - y;

    QVector4D normalisedCoordinates;
    normalisedCoordinates.setX((x - (float)viewport[0]) / (float)viewport[2] * 2.0f - 1.0f);
    normalisedCoordinates.setY((y - (float)viewport[1]) / (float)viewport[3] * 2.0f - 1.0f);
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
