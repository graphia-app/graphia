#include "cameracontroller.h"

#include <QMouseEvent>
#include <QKeyEvent>

#include "camera.h"

CameraController::CameraController() :
    m_camera( 0 ),
    m_vx( 0.0f ),
    m_vy( 0.0f ),
    m_vz( 0.0f ),
    m_viewCenterFixed( false ),
    m_panAngle( 0.0f ),
    m_tiltAngle( 0.0f ),
    m_leftButtonPressed( false ),
    m_orbitMode( false ),
    m_linearSpeed( 2.5f ),
    m_time( 0.0f ),
    m_orbitRate( -1.0f ),
    m_lookRate( 0.1f ),
    m_multisampleEnabled( true )
{
}

void CameraController::setCamera( Camera* cam )
{
    m_camera = cam;
    m_camera->setPerspectiveProjection( 25.0, 1.0, 0.1, 1000 );
}

void CameraController::setLinearSpeed( float speed )
{
    if ( qFuzzyCompare( m_linearSpeed, speed ) )
        return;
    m_linearSpeed = speed;
    emit linearSpeedChanged();
}

float CameraController::linearSpeed() const
{
    return m_linearSpeed;
}

float CameraController::orbitRate() const
{
    return m_orbitRate;
}

void CameraController::setOrbitRate( float rate )
{
    if ( qFuzzyCompare( m_orbitRate, rate ) )
        return;
    m_orbitRate = rate;
    emit orbitRateChanged();
}

float CameraController::lookRate() const
{
    return m_lookRate;
}

void CameraController::setLookRate( float rate )
{
    if ( qFuzzyCompare( m_lookRate, rate ) )
        return;
    m_lookRate = rate;
    emit lookRateChanged();
}

void CameraController::update( double t )
{
    if ( !m_camera )
        return;

    // Store the time
    const float dt = t - m_time;
    m_time = t;

    // Update the camera position and orientation
    Camera::CameraTranslationOption option = m_viewCenterFixed
                                         ? Camera::DontTranslateViewCenter
                                         : Camera::TranslateViewCenter;
    m_camera->translate( dt * QVector3D( m_vx, m_vy, m_vz ), option );

    if ( !qFuzzyIsNull( m_panAngle ) )
    {
        m_camera->pan( m_panAngle );
        m_panAngle = 0.0f;
    }

    if ( !qFuzzyIsNull( m_tiltAngle ) )
    {
        m_camera->tilt( m_tiltAngle );
        m_tiltAngle = 0.0f;
    }
}

bool CameraController::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() )
    {
        case Qt::Key_Right:
            m_vx = m_linearSpeed;
            break;

        case Qt::Key_Left:
            m_vx = -m_linearSpeed;
            break;

        case Qt::Key_Up:
            m_vz = m_linearSpeed;
            break;

        case Qt::Key_Down:
            m_vz = -m_linearSpeed;
            break;

        case Qt::Key_PageUp:
            m_vy = m_linearSpeed;
            break;

        case Qt::Key_PageDown:
            m_vy = -m_linearSpeed;
            break;

        case Qt::Key_Shift:
            m_viewCenterFixed = true;
            break;

        case Qt::Key_A:
            toggleMSAA();
            break;

        default:
            return false;
    }

    return true;
}

bool CameraController::keyReleaseEvent( QKeyEvent* e )
{
    switch ( e->key() )
    {
        case Qt::Key_Right:
        case Qt::Key_Left:
            m_vx = 0.0;
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
            m_vz = 0.0;
            break;

        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            m_vy = 0.0;
            break;

        case Qt::Key_Shift:
            m_viewCenterFixed = false;
            break;

        default:
            return false;
    }

    return true;
}

void CameraController::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
    {
        m_leftButtonPressed = true;
        m_pos = m_prevPos = e->pos();
    }
    else if ( e->button() == Qt::RightButton )
    {
        m_orbitMode = true;
        m_pos = m_prevPos = e->pos();
    }
}

void CameraController::mouseReleaseEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
    {
        m_leftButtonPressed = false;
    }
    else if ( e->button() == Qt::RightButton )
    {
        m_orbitMode = false;
    }
}

void CameraController::mouseMoveEvent( QMouseEvent* e )
{
    if (!m_camera )
      return;

    if (!m_leftButtonPressed && !m_orbitMode)
        return;

    m_pos = e->pos();
    float dx = m_pos.x() - m_prevPos.x();
    float dy = m_pos.y() - m_prevPos.y();
    m_prevPos = m_pos;

    if ( m_leftButtonPressed )
    {
        m_camera->pan( dx * m_lookRate );
        m_camera->tilt( dy * m_lookRate );
    }
    else if ( m_orbitMode )
    {
        m_camera->panAboutViewCenter( dx * m_orbitRate );
        m_camera->tiltAboutViewCenter( dy * m_orbitRate );
    }
}

bool CameraController::isMultisampleEnabled() const
{
    return m_multisampleEnabled;
}

void CameraController::toggleMSAA()
{
    m_multisampleEnabled = !m_multisampleEnabled;
    emit multisampleEnabledChanged();
}

