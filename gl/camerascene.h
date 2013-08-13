#ifndef CAMERASCENE_H
#define CAMERASCENE_H

#include "abstractscene.h"

#include "camera.h"

class CameraScene : public AbstractScene
{
    Q_OBJECT

public:
    CameraScene( QObject* parent = 0 );

    virtual void resize( int w, int h );

    Camera* camera() const { return m_camera; }

protected:
    Camera* m_camera;
};

#endif // CAMERASCENE_H
