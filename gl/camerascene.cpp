#include "camerascene.h"

#include <QOpenGLContext>

CameraScene::CameraScene( QObject* parent )
    : AbstractScene( parent ),
      m_camera( new Camera( this ) )
{

}

void CameraScene::resize( int w, int h )
{
    // Make sure the viewport covers the entire window
    glViewport( 0, 0, w, h );

    // Update the projection matrix
    float aspect = static_cast<float>( w ) / static_cast<float>( h );
    m_camera->setAspectRatio( aspect );
}
