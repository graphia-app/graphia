#ifndef INSTANCEDGEOMETRYSCENE_H
#define INSTANCEDGEOMETRYSCENE_H

#include "abstractscene.h"
#include "camera.h"
#include "../maths/boundingbox.h"

#include <QList>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QColor>
#include <QMutex>

class Sphere;
class Cylinder;
class Quad;
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
    struct DebugLine
    {
        DebugLine(const QVector3D& start, const QVector3D& end, const QColor& color) :
            start(start), end(end), color(color)
        {}

        QVector3D start;
        QVector3D end;
        QColor color;
    };

    void prepareVertexBuffers();
    void prepareNodeVAO();
    void prepareEdgeVAO();
    void prepareComponentMarkerVAO();
    void prepareDebugLinesVAO();
    void prepareTexture();

    void renderNodes();
    void renderEdges();
    void renderComponentMarkers();
    void renderDebugLines();

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
    Quad* m_quad;

    float m_theta;
    QMatrix4x4 m_modelMatrix;

    GraphModel* _graphModel;

    QVector<GLfloat> m_nodePositionData;
    QOpenGLBuffer m_nodePositionDataBuffer;

    QVector<GLfloat> m_edgePositionData;
    QOpenGLBuffer m_edgePositionDataBuffer;

    QVector<GLfloat> m_componentMarkerData;
    QOpenGLBuffer m_componentMarkerDataBuffer;

    QList<DebugLine> debugLines;
    QMutex debugLinesMutex;
    QVector<GLfloat> debugLinesData;
    QVector<QVector3D> debugLinesVertices;
    QOpenGLBuffer debugLinesDataBuffer;
    QOpenGLVertexArrayObject debugLinesDataVAO;
    QOpenGLShaderProgram debugLinesShader;

public:
    const QVector3D viewPosition() { return m_camera->position(); }
    const QVector3D viewVector() { return m_camera->viewVector(); }

    void addDebugLine(const QVector3D& start, const QVector3D& end, const QColor color = QColor(Qt::GlobalColor::white))
    {
        DebugLine debugLine(start, end, color);
        debugLines.append(debugLine);
    }
    void addDebugBoundingBox(const BoundingBox3D& boundingBox, const QColor color = QColor(Qt::GlobalColor::white));
    void clearDebugLines() { debugLines.clear(); }
    void submitDebugLines();
};

#endif // INSTANCEDGEOMETRYSCENE_H
