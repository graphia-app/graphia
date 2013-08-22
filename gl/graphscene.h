#ifndef INSTANCEDGEOMETRYSCENE_H
#define INSTANCEDGEOMETRYSCENE_H

#include "abstractscene.h"

#include <QOpenGLBuffer>
#include <QMatrix4x4>

class Camera;
class Sphere;
class Cylinder;
class GraphModel;

class QOpenGLFunctions_3_3_Core;

class GraphScene : public AbstractScene
{
    Q_OBJECT

public:
    GraphScene(QObject* parent = 0);

    void initialise();
    void update( float t );
    void render();
    void resize( int w, int h );

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
    void prepareNodeVAO();
    void prepareEdgeVAO();
    void prepareTexture();

    void renderNodes();
    void renderEdges();

    QOpenGLFunctions_3_3_Core* m_funcs;

    Camera* m_camera;
    float m_vx;
    float m_vy;
    float m_vz;
    bool m_viewCenterFixed;
    float m_panAngle;
    float m_tiltAngle;

    Sphere* m_sphere;
    Cylinder* m_cylinder;

    float m_theta;
    QMatrix4x4 m_modelMatrix;

    GraphModel* _graphModel;

    // The data array and corresponding buffer
    QVector<GLfloat> m_nodePositionData;
    QOpenGLBuffer m_nodePositionDataBuffer;

    QVector<GLfloat> m_edgePositionData;
    QOpenGLBuffer m_edgePositionDataBuffer;
};

#endif // INSTANCEDGEOMETRYSCENE_H
