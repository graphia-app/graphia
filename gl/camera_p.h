#ifndef CAMERA_P_H
#define CAMERA_P_H

#include <QMatrix4x4>
#include <QVector3D>

class CameraPrivate
{
public:
    CameraPrivate( Camera* qq )
        : q_ptr( qq )
        , m_position( 0.0f, 0.0f, 1.0f )
        , m_upVector( 0.0f, 1.0f, 0.0f )
        , m_viewCenter( 0.0f, 0.0f, 0.0f )
        , m_cameraToCenter( 0.0f, 0.0f, -1.0f )
        , m_projectionType( Camera::OrthogonalProjection )
        , m_nearPlane( 0.1f )
        , m_farPlane( 1024.0f )
        , m_fieldOfView( 60.0f )
        , m_aspectRatio( 1.0f )
        , m_left( -0.5 )
        , m_right( 0.5f )
        , m_bottom( -0.5f )
        , m_top( 0.5f )
        , m_viewMatrixDirty( true )
        , m_viewProjectionMatrixDirty( true )
    {
        updateOrthogonalProjection();
    }

    ~CameraPrivate()
    {
    }

    inline void updatePerpectiveProjection()
    {
        m_projectionMatrix.setToIdentity();
        m_projectionMatrix.perspective( m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane );
        m_viewProjectionMatrixDirty = true;
    }

    inline void updateOrthogonalProjection()
    {
        m_projectionMatrix.setToIdentity();
        m_projectionMatrix.ortho( m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane );
        m_viewProjectionMatrixDirty = true;
    }

    Q_DECLARE_PUBLIC( Camera )

    Camera* q_ptr;

    QVector3D m_position;
    QVector3D m_upVector;
    QVector3D m_viewCenter;

    QVector3D m_cameraToCenter; // The vector from the camera position to the view center

    Camera::ProjectionType m_projectionType;

    float m_nearPlane;
    float m_farPlane;

    float m_fieldOfView;
    float m_aspectRatio;

    float m_left;
    float m_right;
    float m_bottom;
    float m_top;

    mutable QMatrix4x4 m_viewMatrix;
    mutable QMatrix4x4 m_projectionMatrix;
    mutable QMatrix4x4 m_viewProjectionMatrix;

    mutable bool m_viewMatrixDirty;
    mutable bool m_viewProjectionMatrixDirty;
};

#endif // CAMERA_P_H
