#ifndef INSTANCEDGEOMETRYSCENE_H
#define INSTANCEDGEOMETRYSCENE_H

#include "abstractscene.h"

#include "../graph/graphmodel.h"

#include <QOpenGLBuffer>
#include <QMatrix4x4>

class Camera;
class Cube;
class Torus;

class QOpenGLFunctions_3_3_Core;

class GraphScene : public AbstractScene
{
    Q_OBJECT

public:
    GraphScene(QObject* parent = 0);

    virtual void initialise();
    virtual void update( float t );
    virtual void render();
    virtual void resize( int w, int h );

    // Camera motion control
    void setSideSpeed( float vx ) { m_vx = vx; }
    void setVerticalSpeed( float vy ) { m_vy = vy; }
    void setForwardSpeed( float vz ) { m_vz = vz; }
    void setViewCenterFixed( bool b ) { m_viewCenterFixed = b; }

    // Camera orientation control
    void pan( float angle ) { m_panAngle = angle; }
    void tilt( float angle ) { m_tiltAngle = angle; }

    void setGraphModel(GraphModel* graphModel) { this->_graphModel = graphModel; }

private:
    void prepareVertexBuffers();
    void prepareVertexArrayObject();
    void prepareTexture();

    QOpenGLFunctions_3_3_Core* m_funcs;

    Camera* m_camera;
    float m_vx;
    float m_vy;
    float m_vz;
    bool m_viewCenterFixed;
    float m_panAngle;
    float m_tiltAngle;

    Cube* m_cube;

    float m_theta;
    QMatrix4x4 m_modelMatrix;

    GraphModel* _graphModel;

    // The data array and corresponding buffer
    QVector<float> m_data;
    QOpenGLBuffer m_dataBuffer;
};

#endif // INSTANCEDGEOMETRYSCENE_H
