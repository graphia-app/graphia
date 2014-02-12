#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>

#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>
#include <QSharedPointer>

#include "../maths/frustum.h"
#include "../maths/line.h"
#include "../maths/ray.h"

class CameraPrivate;

class QOpenGLShaderProgram;
typedef QSharedPointer<QOpenGLShaderProgram> QOpenGLShaderProgramPtr;

class Camera : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QVector3D position READ position WRITE setPosition )
    Q_PROPERTY( QVector3D upVector READ upVector WRITE setUpVector )
    Q_PROPERTY( QVector3D viewTarget READ viewTarget WRITE setViewTarget )

    Q_PROPERTY( ProjectionType projectionType READ projectionType )
    Q_PROPERTY( float nearPlane READ nearPlane WRITE setNearPlane )
    Q_PROPERTY( float farPlane READ farPlane WRITE setFarPlane )

    Q_PROPERTY( float fieldOfView READ fieldOfView WRITE setFieldOfView )
    Q_PROPERTY( float aspectRatio READ aspectRatio WRITE setAspectRatio )

    Q_PROPERTY( float left READ left WRITE setLeft )
    Q_PROPERTY( float right READ right WRITE setRight )
    Q_PROPERTY( float bottom READ bottom WRITE setBottom )
    Q_PROPERTY( float top READ top WRITE setTop )

    Q_ENUMS( ProjectionType )

public:
    explicit Camera( QObject* parent = 0 );

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

    void setOrthographicProjection( float left, float right,
                                    float bottom, float top,
                                    float nearPlane, float farPlane );

    void setPerspectiveProjection( float fieldOfView, float aspect,
                                   float nearPlane, float farPlane );

    void setNearPlane( const float& nearPlane );
    float nearPlane() const;

    void setFarPlane( const float& nearPlane );
    float farPlane() const;

    void setFieldOfView( const float& fieldOfView );
    float fieldOfView() const;

    void setAspectRatio( const float& aspectRatio );
    float aspectRatio() const;

    void setLeft( const float& left );
    float left() const;

    void setRight( const float& right );
    float right() const;

    void setBottom( const float& bottom );
    float bottom() const;

    void setTop( const float& top );
    float top() const;

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;
    QMatrix4x4 viewProjectionMatrix() const;

    const Ray rayForViewportCoordinates(int x, int y);
    Line3D lineForViewportCoordinates(int x, int y);
    Frustum frustumForViewportCoordinates(int x1, int y1, int x2, int y2);

    QQuaternion tiltRotation( const float& angle ) const;
    QQuaternion panRotation( const float& angle ) const;
    QQuaternion rollRotation( const float& angle ) const;

    /**
     * @brief setStandardUniforms - set the standard transform uniforms on
     * the provided shader program. Standard names are 'mvp', 'modelViewMatrix',
     * 'normalMatrix' and 'projectionMatrix'
     * @param program - the program whose uniforms should be modified
     * @param model - the model matrix to use
     */
    void setStandardUniforms( const QOpenGLShaderProgramPtr& program, const QMatrix4x4& model ) const;

public slots:
    void setPosition( const QVector3D& position );
    void setUpVector( const QVector3D& upVector );
    void setViewTarget( const QVector3D& viewTarget );

    void resetViewToIdentity();

    // Translate relative to camera orientation axes
    void translate( const QVector3D& vLocal, CameraTranslationOption option = TranslateViewCenter );

    // Translate relative to world axes
    void translateWorld( const QVector3D& vWorld, CameraTranslationOption option = TranslateViewCenter );

    void tilt( const float& angle );
    void pan( const float& angle );
    void roll( const float& angle );

    void tiltAboutViewCenter( const float& angle );
    void panAboutViewCenter( const float& angle );
    void rollAboutViewCenter( const float& angle );

    void rotate( const QQuaternion& q );
    void rotateAboutViewTarget( const QQuaternion& q );

protected:
    Q_DECLARE_PRIVATE( Camera )

private:
    bool unproject(int x, int y, int z, QVector3D& result);

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

    QVector3D m_position;
    QVector3D m_upVector;
    QVector3D m_viewTarget;

    QVector3D m_cameraToTarget; // The vector from the camera position to the view center

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

#endif // CAMERA_H
