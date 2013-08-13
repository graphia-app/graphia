#include "camera.h"
#include "camera_p.h"

#include <QOpenGLShaderProgram>

Camera::Camera( QObject* parent )
    : QObject( parent )
    , d_ptr( new CameraPrivate( this ) )
{
}

Camera::ProjectionType Camera::projectionType() const
{
    Q_D( const Camera );
    return d->m_projectionType;
}

QVector3D Camera::position() const
{
    Q_D( const Camera );
    return d->m_position;
}

void Camera::setPosition( const QVector3D& position )
{
    Q_D( Camera );
    d->m_position = position;
    d->m_cameraToCenter = d->m_viewCenter - position;
    d->m_viewMatrixDirty = true;
}

void Camera::setUpVector( const QVector3D& upVector )
{
    Q_D( Camera );
    d->m_upVector = upVector;
    d->m_viewMatrixDirty = true;
}

QVector3D Camera::upVector() const
{
    Q_D( const Camera );
    return d->m_upVector;
}

void Camera::setViewCenter( const QVector3D& viewCenter )
{
    Q_D( Camera );
    d->m_viewCenter = viewCenter;
    d->m_cameraToCenter = viewCenter - d->m_position;
    d->m_viewMatrixDirty = true;
}

QVector3D Camera::viewCenter() const
{
    Q_D( const Camera );
    return d->m_viewCenter;
}

QVector3D Camera::viewVector() const
{
    Q_D( const Camera );
    return d->m_cameraToCenter;
}

void Camera::setOrthographicProjection( float left, float right,
                                        float bottom, float top,
                                        float nearPlane, float farPlane )
{
    Q_D( Camera );
    d->m_left = left;
    d->m_right = right;
    d->m_bottom = bottom;
    d->m_top = top;
    d->m_nearPlane = nearPlane;
    d->m_farPlane = farPlane;
    d->m_projectionType = OrthogonalProjection;
    d->updateOrthogonalProjection();
}

void Camera::setPerspectiveProjection( float fieldOfView, float aspectRatio,
                                       float nearPlane, float farPlane )
{
    Q_D( Camera );
    d->m_fieldOfView = fieldOfView;
    d->m_aspectRatio = aspectRatio;
    d->m_nearPlane = nearPlane;
    d->m_farPlane = farPlane;
    d->m_projectionType = PerspectiveProjection;
    d->updatePerpectiveProjection();
}

void Camera::setNearPlane( const float& nearPlane )
{
    Q_D( Camera );
    if ( qFuzzyCompare( d->m_nearPlane, nearPlane ) )
        return;
    d->m_nearPlane = nearPlane;
    if ( d->m_projectionType == PerspectiveProjection )
        d->updatePerpectiveProjection();
}

float Camera::nearPlane() const
{
    Q_D( const Camera );
    return d->m_nearPlane;
}

void Camera::setFarPlane( const float& farPlane )
{
    Q_D( Camera );
    if ( qFuzzyCompare( d->m_farPlane, farPlane ) )
        return;
    d->m_farPlane = farPlane;
    if ( d->m_projectionType == PerspectiveProjection )
        d->updatePerpectiveProjection();
}

float Camera::farPlane() const
{
    Q_D( const Camera );
    return d->m_farPlane;
}

void Camera::setFieldOfView( const float& fieldOfView )
{
    Q_D( Camera );
    if ( qFuzzyCompare( d->m_fieldOfView, fieldOfView ) )
        return;
    d->m_fieldOfView = fieldOfView;
    if ( d->m_projectionType == PerspectiveProjection )
        d->updatePerpectiveProjection();
}

float Camera::fieldOfView() const
{
    Q_D( const Camera );
    return d->m_fieldOfView;
}

void Camera::setAspectRatio( const float& aspectRatio )
{
    Q_D( Camera );
    if ( qFuzzyCompare( d->m_aspectRatio, aspectRatio ) )
        return;
    d->m_aspectRatio = aspectRatio;
    if ( d->m_projectionType == PerspectiveProjection )
        d->updatePerpectiveProjection();
}

float Camera::aspectRatio() const
{
    Q_D( const Camera );
    return d->m_aspectRatio;
}

void Camera::setLeft( const float& left )
{
    Q_D( Camera );
    if ( qFuzzyCompare( d->m_left, left ) )
        return;
    d->m_left = left;
    if ( d->m_projectionType == OrthogonalProjection )
        d->updateOrthogonalProjection();
}

float Camera::left() const
{
    Q_D( const Camera );
    return d->m_left;
}

void Camera::setRight( const float& right )
{
    Q_D( Camera );
    if ( qFuzzyCompare( d->m_right, right ) )
        return;
    d->m_right = right;
    if ( d->m_projectionType == OrthogonalProjection )
        d->updateOrthogonalProjection();
}

float Camera::right() const
{
    Q_D( const Camera );
    return d->m_right;
}

void Camera::setBottom( const float& bottom )
{
    Q_D( Camera );
    if ( qFuzzyCompare( d->m_bottom, bottom ) )
        return;
    d->m_bottom = bottom;
    if ( d->m_projectionType == OrthogonalProjection )
        d->updateOrthogonalProjection();
}

float Camera::bottom() const
{
    Q_D( const Camera );
    return d->m_bottom;
}

void Camera::setTop( const float& top )
{
    Q_D( Camera );
    if ( qFuzzyCompare( d->m_top, top ) )
        return;
    d->m_top = top;
    if ( d->m_projectionType == OrthogonalProjection )
        d->updateOrthogonalProjection();
}

float Camera::top() const
{
    Q_D( const Camera );
    return d->m_top;
}

QMatrix4x4 Camera::viewMatrix() const
{
    Q_D( const Camera );
    if ( d->m_viewMatrixDirty )
    {
        d->m_viewMatrix.setToIdentity();
        d->m_viewMatrix.lookAt( d->m_position, d->m_viewCenter, d->m_upVector );
        d->m_viewMatrixDirty = false;
    }
    return d->m_viewMatrix;
}

void Camera::resetViewToIdentity()
{
    setPosition( QVector3D( 0.0, 0.0, 0.0) );
    setViewCenter( QVector3D( 0.0, 0.0, 1.0) );
    setUpVector( QVector3D( 0.0, 1.0, 0.0) );
}

QMatrix4x4 Camera::projectionMatrix() const
{
    Q_D( const Camera );
    return d->m_projectionMatrix;
}

QMatrix4x4 Camera::viewProjectionMatrix() const
{
    Q_D( const Camera );
    if ( d->m_viewMatrixDirty || d->m_viewProjectionMatrixDirty )
    {
        d->m_viewProjectionMatrix = d->m_projectionMatrix * viewMatrix();
        d->m_viewProjectionMatrixDirty = false;
    }
    return d->m_viewProjectionMatrix;
}

void Camera::translate( const QVector3D& vLocal, CameraTranslationOption option )
{
    Q_D( Camera );

    // Calculate the amount to move by in world coordinates
    QVector3D vWorld;
    if ( !qFuzzyIsNull( vLocal.x() ) )
    {
        // Calculate the vector for the local x axis
        QVector3D x = QVector3D::crossProduct( d->m_cameraToCenter, d->m_upVector ).normalized();
        vWorld += vLocal.x() * x;
    }

    if ( !qFuzzyIsNull( vLocal.y() ) )
        vWorld += vLocal.y() * d->m_upVector;

    if ( !qFuzzyIsNull( vLocal.z() ) )
        vWorld += vLocal.z() * d->m_cameraToCenter.normalized();

    // Update the camera position using the calculated world vector
    d->m_position += vWorld;

    // May be also update the view center coordinates
    if ( option == TranslateViewCenter )
        d->m_viewCenter += vWorld;

    // Refresh the camera -> view center vector
    d->m_cameraToCenter = d->m_viewCenter - d->m_position;

    // Calculate a new up vector. We do this by:
    // 1) Calculate a new local x-direction vector from the cross product of the new
    //    camera to view center vector and the old up vector.
    // 2) The local x vector is the normal to the plane in which the new up vector
    //    must lay. So we can take the cross product of this normal and the new
    //    x vector. The new normal vector forms the last part of the orthonormal basis
    QVector3D x = QVector3D::crossProduct( d->m_cameraToCenter, d->m_upVector ).normalized();
    d->m_upVector = QVector3D::crossProduct( x, d->m_cameraToCenter ).normalized();

    d->m_viewMatrixDirty = true;
    d->m_viewProjectionMatrixDirty = true;
}

void Camera::translateWorld(const QVector3D& vWorld , CameraTranslationOption option )
{
    Q_D( Camera );

    // Update the camera position using the calculated world vector
    d->m_position += vWorld;

    // May be also update the view center coordinates
    if ( option == TranslateViewCenter )
        d->m_viewCenter += vWorld;

    // Refresh the camera -> view center vector
    d->m_cameraToCenter = d->m_viewCenter - d->m_position;

    d->m_viewMatrixDirty = true;
    d->m_viewProjectionMatrixDirty = true;
}

QQuaternion Camera::tiltRotation( const float& angle ) const
{
    Q_D( const Camera );
    QVector3D xBasis = QVector3D::crossProduct( d->m_upVector, d->m_cameraToCenter.normalized() ).normalized();
    return QQuaternion::fromAxisAndAngle( xBasis, -angle );
}

QQuaternion Camera::panRotation( const float& angle ) const
{
    Q_D( const Camera );
    return QQuaternion::fromAxisAndAngle( d->m_upVector, angle );
}

QQuaternion Camera::rollRotation( const float& angle ) const
{
    Q_D( const Camera );
    return QQuaternion::fromAxisAndAngle( d->m_cameraToCenter, -angle );
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
    rotateAboutViewCenter( q );
}

void Camera::panAboutViewCenter( const float& angle )
{
    QQuaternion q = panRotation( angle );
    rotateAboutViewCenter( q );
}

void Camera::rollAboutViewCenter( const float& angle )
{
    QQuaternion q = rollRotation( angle );
    rotateAboutViewCenter( q );
}

void Camera::rotate( const QQuaternion& q )
{
    Q_D( Camera );
    d->m_upVector = q.rotatedVector( d->m_upVector );
    d->m_cameraToCenter = q.rotatedVector( d->m_cameraToCenter );
    d->m_viewCenter = d->m_position + d->m_cameraToCenter;
    d->m_viewMatrixDirty = true;
    d->m_viewProjectionMatrixDirty = true;
}

void Camera::rotateAboutViewCenter( const QQuaternion& q )
{
    Q_D( Camera );
    d->m_upVector = q.rotatedVector( d->m_upVector );
    d->m_cameraToCenter = q.rotatedVector( d->m_cameraToCenter );
    d->m_position = d->m_viewCenter - d->m_cameraToCenter;
    d->m_viewMatrixDirty = true;
    d->m_viewProjectionMatrixDirty = true;
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
