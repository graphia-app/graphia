#include "graphscene.h"

#include "camera.h"
#include "sphere.h"
#include "cylinder.h"
#include "quad.h"
#include "material.h"

#include "../graph/graphmodel.h"
#include "../layout/layout.h"
#include "../layout/spatialoctree.h"
#include "../layout/collision.h"

#include <QObject>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFunctions_3_2_Core>
#if defined(Q_OS_MAC)
#include <QOpenGLExtensions>
#endif

#include <QMutexLocker>

#include <QKeyEvent>
#include <QMouseEvent>

#include <math.h>

GraphScene::GraphScene( QObject* parent )
    : AbstractScene( parent ),
      m_leftButtonPressed( false ),
      m_funcs(nullptr),
#if defined(Q_OS_MAC)
      m_instanceFuncs( 0 ),
#endif
      m_camera( new Camera( this ) ),
      m_vx( 0.0f ),
      m_vy( 0.0f ),
      m_vz( 0.0f ),
      m_viewCenterFixed( false ),
      m_panAngle(0.0f),
      m_tiltAngle(0.0f),
      m_sphere(nullptr),
      m_cylinder(nullptr),
      m_quad(nullptr),
      _graphModel(nullptr),
      m_nodePositionData(0),
      m_edgePositionData(0),
      m_nodeVisualData(0),
      m_edgeVisualData(0),
      m_componentMarkerData(0),
      debugLinesData(0)
{
    update( 0.0f );

    // Initialize the camera position and orientation
    m_camera->setPosition( QVector3D( 0.0f, 0.0f, 50.0f ) );
    m_camera->setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );
    m_camera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
}

void GraphScene::initialise()
{
    // Resolve the OpenGL functions that we need for instanced rendering
#if !defined(Q_OS_MAC)
    m_funcs = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
#else
    m_instanceFuncs = new QOpenGLExtension_ARB_instanced_arrays;
    if ( !m_instanceFuncs->initializeOpenGLFunctions() )
        qFatal( "Could not resolve GL_ARB_instanced_arrays functions" );

    m_funcs = m_context->versionFunctions<QOpenGLFunctions_3_2_Core>();
#endif
    if ( !m_funcs )
        qFatal( "Could not obtain required OpenGL context version" );
    m_funcs->initializeOpenGLFunctions();

    MaterialPtr nodeMaterial(new Material);
    nodeMaterial->setShaders(":/gl/shaders/instancednodes.vert", ":/gl/shaders/ads.frag" );

    // Create a sphere
    m_sphere = new Sphere( this );
    m_sphere->setRadius(1.0f);
    m_sphere->setRings(16);
    m_sphere->setSlices(16);
    m_sphere->setMaterial(nodeMaterial);
    m_sphere->create();

    MaterialPtr edgeMaterial(new Material);
    edgeMaterial->setShaders(":/gl/shaders/instancededges.vert", ":/gl/shaders/ads.frag" );

    m_cylinder = new Cylinder(this);
    m_cylinder->setRadius(1.0f);
    m_cylinder->setLength(1.0f);
    m_cylinder->setSlices(8);
    m_cylinder->setMaterial(edgeMaterial);
    m_cylinder->create();

    MaterialPtr componentMarkerMaterial(new Material);
    componentMarkerMaterial->setShaders(":/gl/shaders/instancedmarkers.vert", ":/gl/shaders/marker.frag" );

    m_quad = new Quad(this);
    m_quad->setEdgeLength(1.0f);
    m_quad->setMaterial(componentMarkerMaterial);
    m_quad->create();

    debugLinesDataVAO.create();
    if(!debugLinesShader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/gl/shaders/debuglines.vert"))
        qCritical() << QObject::tr("Could not compile vertex shader. Log:") << debugLinesShader.log();

    if(!debugLinesShader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/gl/shaders/debuglines.frag"))
        qCritical() << QObject::tr("Could not compile fragment shader. Log:") << debugLinesShader.log();

    if(!debugLinesShader.link())
        qCritical() << QObject::tr("Could not link shader program. Log:") << debugLinesShader.log();

    // Create a pair of VBOs ready to hold our data
    prepareVertexBuffers();

    // Tell OpenGL how to pass the data VBOs to the shader program
    prepareNodeVAO();
    prepareEdgeVAO();
    prepareComponentMarkerVAO();
    prepareDebugLinesVAO();

    // Enable depth testing to prevent artifacts
    glEnable( GL_DEPTH_TEST );

    // Cull back facing triangles to save the gpu some work
    glEnable( GL_CULL_FACE );

    glClearColor( 0.75f, 0.75f, 0.75f, 1.0f );
}

void GraphScene::updateVisualData()
{
    NodeVisuals& nodeVisuals = _graphModel->nodeVisuals();
    EdgeVisuals& edgeVisuals = _graphModel->edgeVisuals();

    m_nodeVisualData.resize(_graphModel->graph().numNodes() * 4);
    m_edgeVisualData.resize(_graphModel->graph().numEdges() * 4);
    int i = 0;
    int j = 0;

    for(ComponentId componentId : *_graphModel->graph().componentIds())
    {
        const ReadOnlyGraph& component = *_graphModel->graph().componentById(componentId);

        for(NodeId nodeId : component.nodeIds())
        {
            m_nodeVisualData[i++] = nodeVisuals[nodeId].size;
            m_nodeVisualData[i++] = nodeVisuals[nodeId].color.redF();
            m_nodeVisualData[i++] = nodeVisuals[nodeId].color.greenF();
            m_nodeVisualData[i++] = nodeVisuals[nodeId].color.blueF();
        }

        for(EdgeId edgeId : component.edgeIds())
        {
            m_edgeVisualData[j++] = edgeVisuals[edgeId].size;
            m_edgeVisualData[j++] = edgeVisuals[edgeId].color.redF();
            m_edgeVisualData[j++] = edgeVisuals[edgeId].color.greenF();
            m_edgeVisualData[j++] = edgeVisuals[edgeId].color.blueF();
        }
    }
}

void GraphScene::onGraphChanged(const Graph*)
{
    updateVisualData();
}

void GraphScene::update( float /*t*/ )
{
    clearDebugLines();

    if(_graphModel != nullptr)
    {
        NodePositions& nodePositions = _graphModel->nodePositions();
        ComponentPositions& componentPositions = _graphModel->componentPositions();

        m_nodePositionData.resize(_graphModel->graph().numNodes() * 3);
        m_edgePositionData.resize(_graphModel->graph().numEdges() * 6);
        m_componentMarkerData.resize(_graphModel->graph().numComponents() * 3);
        int i = 0;
        int j = 0;
        int k = 0;

        for(ComponentId componentId : *_graphModel->graph().componentIds())
        {
            const ReadOnlyGraph& component = *_graphModel->graph().componentById(componentId);

            for(NodeId nodeId : component.nodeIds())
            {
                m_nodePositionData[i++] = nodePositions[nodeId].x() + componentPositions[componentId].x();
                m_nodePositionData[i++] = nodePositions[nodeId].y() + componentPositions[componentId].y();
                m_nodePositionData[i++] = nodePositions[nodeId].z();
            }

            for(EdgeId edgeId : component.edgeIds())
            {
                const Edge& edge = _graphModel->graph().edgeById(edgeId);

                m_edgePositionData[j++] = nodePositions[edge.sourceId()].x() + componentPositions[componentId].x();
                m_edgePositionData[j++] = nodePositions[edge.sourceId()].y() + componentPositions[componentId].y();
                m_edgePositionData[j++] = nodePositions[edge.sourceId()].z();
                m_edgePositionData[j++] = nodePositions[edge.targetId()].x() + componentPositions[componentId].x();
                m_edgePositionData[j++] = nodePositions[edge.targetId()].y() + componentPositions[componentId].y();
                m_edgePositionData[j++] = nodePositions[edge.targetId()].z();
            }

            m_componentMarkerData[k++] = componentPositions[componentId].x();
            m_componentMarkerData[k++] = componentPositions[componentId].y();
            m_componentMarkerData[k++] = NodeLayout::boundingCircleRadiusInXY(component, nodePositions);

            //FIXME debug
            SpatialOctTree octree(NodeLayout::boundingBox(component, nodePositions), component.nodeIds(), nodePositions);
            octree.debugRenderOctTree(this, componentPositions[componentId]);
        }
    }

    submitDebugLines();

    Camera::CameraTranslationOption option = m_viewCenterFixed
                                           ? Camera::DontTranslateViewCenter
                                           : Camera::TranslateViewCenter;
    m_camera->translate( QVector3D( m_vx, m_vy, m_vz ), option );

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

void GraphScene::renderNodes()
{
    m_nodePositionBuffer.bind();
    m_nodePositionBuffer.allocate( m_nodePositionData.data(),
                                       m_nodePositionData.size() * sizeof(GLfloat) );
    m_nodeVisualBuffer.bind();
    m_nodeVisualBuffer.allocate( m_nodeVisualData.data(),
                                       m_nodeVisualData.size() * sizeof(GLfloat) );

    // Bind the shader program
    QOpenGLShaderProgramPtr shader = m_sphere->material()->shader();
    shader->bind();

    // Calculate needed matrices
    QMatrix4x4 modelViewMatrix = m_camera->viewMatrix();
    QMatrix3x3 normalMatrix = modelViewMatrix.normalMatrix();
    shader->setUniformValue( "modelViewMatrix", modelViewMatrix );
    shader->setUniformValue( "normalMatrix", normalMatrix );
    shader->setUniformValue( "projectionMatrix", m_camera->projectionMatrix() );

    // Set the lighting parameters
    shader->setUniformValue( "light.position", QVector4D( -10.0f, 10.0f, 0.0f, 1.0f ) );
    shader->setUniformValue( "light.intensity", QVector3D( 1.0f, 1.0f, 1.0f ) );
    shader->setUniformValue( "material.kd", QVector3D( 0.5f, 0.2f, 0.8f ) );
    shader->setUniformValue( "material.ks", QVector3D( 0.95f, 0.95f, 0.95f ) );
    shader->setUniformValue( "material.ka", QVector3D( 0.1f, 0.1f, 0.1f ) );
    shader->setUniformValue( "material.shininess", 10.0f );

    // Draw the nodes
    m_sphere->vertexArrayObject()->bind();
    m_funcs->glDrawElementsInstanced(GL_TRIANGLES, m_sphere->indexCount(),
                                     GL_UNSIGNED_INT, 0, _graphModel->graph().numNodes());
    m_sphere->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::renderEdges()
{
    m_edgePositionBuffer.bind();
    m_edgePositionBuffer.allocate( m_edgePositionData.data(),
                                       m_edgePositionData.size() * sizeof(GLfloat) );

    m_edgeVisualBuffer.bind();
    m_edgeVisualBuffer.allocate( m_edgeVisualData.data(),
                                       m_edgeVisualData.size() * sizeof(GLfloat) );

    // Bind the shader program
    QOpenGLShaderProgramPtr shader = m_cylinder->material()->shader();
    shader->bind();

    shader->setUniformValue("viewMatrix", m_camera->viewMatrix());
    shader->setUniformValue("projectionMatrix", m_camera->projectionMatrix());

    // Set the lighting parameters
    shader->setUniformValue( "light.position", QVector4D( -10.0f, 10.0f, 0.0f, 1.0f ) );
    shader->setUniformValue( "light.intensity", QVector3D( 1.0f, 1.0f, 1.0f ) );
    shader->setUniformValue( "material.kd", QVector3D( 1.0f, 1.0f, 0.0f ) );
    shader->setUniformValue( "material.ks", QVector3D( 0.95f, 0.95f, 0.95f ) );
    shader->setUniformValue( "material.ka", QVector3D( 0.1f, 0.1f, 0.1f ) );
    shader->setUniformValue( "material.shininess", 10.0f );

    // Draw the edges
    m_cylinder->vertexArrayObject()->bind();
    m_funcs->glDrawElementsInstanced(GL_TRIANGLES, m_cylinder->indexCount(),
                                     GL_UNSIGNED_INT, 0, _graphModel->graph().numEdges());
    m_cylinder->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::renderComponentMarkers()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_componentMarkerDataBuffer.bind();
    m_componentMarkerDataBuffer.allocate( m_componentMarkerData.data(),
                                       m_componentMarkerData.size() * sizeof(GLfloat) );

    // Bind the shader program
    QOpenGLShaderProgramPtr shader = m_quad->material()->shader();
    shader->bind();

    // Calculate needed matrices
    QMatrix4x4 modelViewMatrix = m_camera->viewMatrix();
    shader->setUniformValue( "modelViewMatrix", modelViewMatrix );
    shader->setUniformValue( "projectionMatrix", m_camera->projectionMatrix() );

    // Draw the edges
    m_quad->vertexArrayObject()->bind();
    m_funcs->glDrawElementsInstanced(GL_TRIANGLES, m_quad->indexCount(),
                                     GL_UNSIGNED_INT, 0, _graphModel->graph().numComponents());
    m_quad->vertexArrayObject()->release();
    shader->release();

    glDisable(GL_BLEND);
}

void GraphScene::renderDebugLines()
{
    QMutexLocker locker(&debugLinesMutex);

    debugLinesDataBuffer.bind();
    debugLinesDataBuffer.allocate(debugLinesData.data(), debugLinesData.size() * sizeof(GLfloat));

    debugLinesShader.bind();

    // Calculate needed matrices
    QMatrix4x4 modelViewMatrix = m_camera->viewMatrix();
    debugLinesShader.setUniformValue( "modelViewMatrix", modelViewMatrix );
    debugLinesShader.setUniformValue( "projectionMatrix", m_camera->projectionMatrix() );

    debugLinesDataVAO.bind();
    glDrawArrays(GL_LINES, 0, debugLines.size() * 2);
    debugLinesDataVAO.release();
    debugLinesShader.release();
}

void GraphScene::addDebugBoundingBox(const BoundingBox3D& boundingBox, const QColor color)
{
    const QVector3D& min = boundingBox.min();
    const QVector3D& max = boundingBox.max();

    const QVector3D _0 = QVector3D(min.x(), min.y(), min.z());
    const QVector3D _1 = QVector3D(max.x(), min.y(), min.z());
    const QVector3D _2 = QVector3D(min.x(), max.y(), min.z());
    const QVector3D _3 = QVector3D(max.x(), max.y(), min.z());
    const QVector3D _4 = QVector3D(min.x(), min.y(), max.z());
    const QVector3D _5 = QVector3D(max.x(), min.y(), max.z());
    const QVector3D _6 = QVector3D(min.x(), max.y(), max.z());
    const QVector3D _7 = QVector3D(max.x(), max.y(), max.z());

    addDebugLine(_0, _1, color);
    addDebugLine(_1, _3, color);
    addDebugLine(_3, _2, color);
    addDebugLine(_2, _0, color);

    addDebugLine(_4, _5, color);
    addDebugLine(_5, _7, color);
    addDebugLine(_7, _6, color);
    addDebugLine(_6, _4, color);

    addDebugLine(_0, _4, color);
    addDebugLine(_1, _5, color);
    addDebugLine(_3, _7, color);
    addDebugLine(_2, _6, color);
}

void GraphScene::submitDebugLines()
{
    QMutexLocker locker(&debugLinesMutex);

    debugLinesData.resize(debugLines.size() * 12);

    int i = 0;
    for(const DebugLine debugLine : debugLines)
    {
        debugLinesData[i++] = debugLine.start.x();
        debugLinesData[i++] = debugLine.start.y();
        debugLinesData[i++] = debugLine.start.z();
        debugLinesData[i++] = debugLine.color.redF();
        debugLinesData[i++] = debugLine.color.greenF();
        debugLinesData[i++] = debugLine.color.blueF();
        debugLinesData[i++] = debugLine.end.x();
        debugLinesData[i++] = debugLine.end.y();
        debugLinesData[i++] = debugLine.end.z();
        debugLinesData[i++] = debugLine.color.redF();
        debugLinesData[i++] = debugLine.color.greenF();
        debugLinesData[i++] = debugLine.color.blueF();
    }
}

void GraphScene::render()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    renderNodes();
    renderEdges();
    renderComponentMarkers();
    renderDebugLines();
}

void GraphScene::resize( int w, int h )
{
    // Make sure the viewport covers the entire window
    glViewport( 0, 0, w, h );

    // Update the projection matrix
    float aspect = static_cast<float>( w ) / static_cast<float>( h );
    m_camera->setPerspectiveProjection( 60.0f, aspect, 0.3f, 10000.0f );
}

void GraphScene::mousePressEvent(QMouseEvent* mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        m_leftButtonPressed = true;
        m_pos = m_prevPos = mouseEvent->pos();

        Ray ray = m_camera->rayForViewportCoordinates(m_pos.x(), m_pos.y());
        ComponentPositions& componentPositions = _graphModel->componentPositions();

        for(ComponentId componentId : *_graphModel->graph().componentIds())
        {
            const ReadOnlyGraph& component = *_graphModel->graph().componentById(componentId);

            Collision collision(component, _graphModel->nodeVisuals(), _graphModel->nodePositions());
            collision.setOffset(componentPositions[componentId]);
            NodeId nodeId = collision.closestNodeIntersectingLine(ray.origin(), ray.dir());

            if (nodeId != NullNodeId)
                qDebug() << m_pos << nodeId << "(" << componentId << ")";
        }
    }
}

void GraphScene::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
        m_leftButtonPressed = false;
}

void GraphScene::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    if (m_leftButtonPressed)
    {
        m_pos = mouseEvent->pos();
        float dx = 0.5f * (m_pos.x() - m_prevPos.x());
        float dy = 0.5f * (m_pos.y() - m_prevPos.y());
        m_prevPos = m_pos;

        pan(dx);
        tilt(dy);
    }
}

bool GraphScene::keyPressEvent(QKeyEvent* keyEvent)
{
    const float MOVE_SPEED = 2.0f;

    switch (keyEvent->key())
    {
    case Qt::Key_D:
        setSideSpeed(MOVE_SPEED);
        break;

    case Qt::Key_A:
        setSideSpeed(-MOVE_SPEED);
        break;

    case Qt::Key_W:
        setForwardSpeed(MOVE_SPEED);
        break;

    case Qt::Key_S:
        setForwardSpeed(-MOVE_SPEED);
        break;

    case Qt::Key_Shift:
        setVerticalSpeed(MOVE_SPEED);
        break;

    case Qt::Key_Control:
        setVerticalSpeed(-MOVE_SPEED);
        break;

    case Qt::Key_Space:
        setViewCenterFixed(true);
        break;

    default:
        return false;
    }

    return true;
}

bool GraphScene::keyReleaseEvent(QKeyEvent* keyEvent)
{
    switch (keyEvent->key())
    {
    case Qt::Key_A:
    case Qt::Key_D:
        setSideSpeed( 0.0f );
        break;

    case Qt::Key_W:
    case Qt::Key_S:
        setForwardSpeed( 0.0f );
        break;

    case Qt::Key_Shift:
    case Qt::Key_Control:
        setVerticalSpeed( 0.0f );
        break;

    case Qt::Key_Space:
        setViewCenterFixed( false );

    default:
        return false;
    }

    return true;
}

void GraphScene::setGraphModel(GraphModel* graphModel)
{
    this->_graphModel = graphModel;

    updateVisualData();
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphScene::onGraphChanged);
}

void GraphScene::prepareVertexBuffers()
{
    // Populate the data buffer object
    m_nodePositionBuffer.create();
    m_nodePositionBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_nodePositionBuffer.bind();
    m_nodePositionBuffer.allocate( m_nodePositionData.data(), m_nodePositionData.size() * sizeof(GLfloat) );

    m_edgePositionBuffer.create();
    m_edgePositionBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_edgePositionBuffer.bind();
    m_edgePositionBuffer.allocate( m_edgePositionData.data(), m_edgePositionData.size() * sizeof(GLfloat) );

    m_nodeVisualBuffer.create();
    m_nodeVisualBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_nodeVisualBuffer.bind();
    m_nodeVisualBuffer.allocate( m_nodeVisualData.data(), m_nodeVisualData.size() * sizeof(GLfloat) );

    m_edgeVisualBuffer.create();
    m_edgeVisualBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_edgeVisualBuffer.bind();
    m_edgeVisualBuffer.allocate( m_edgeVisualData.data(), m_edgeVisualData.size() * sizeof(GLfloat) );

    m_componentMarkerDataBuffer.create();
    m_componentMarkerDataBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_componentMarkerDataBuffer.bind();
    m_componentMarkerDataBuffer.allocate( m_componentMarkerData.data(), m_componentMarkerData.size() * sizeof(GLfloat) );

    debugLinesDataBuffer.create();
    debugLinesDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    debugLinesDataBuffer.bind();
    debugLinesDataBuffer.allocate(debugLinesData.data(), debugLinesData.size() * sizeof(GLfloat));
}

void GraphScene::prepareNodeVAO()
{
    // Bind the marker's VAO
    m_sphere->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = m_sphere->material()->shader();
    shader->bind();
    m_nodePositionBuffer.bind();
    shader->enableAttributeArray("point");
    shader->setAttributeBuffer("point", GL_FLOAT, 0, 3);

    m_nodeVisualBuffer.bind();
    shader->enableAttributeArray("size");
    shader->enableAttributeArray("color");
    shader->setAttributeBuffer("size", GL_FLOAT, 0, 1, 4 * sizeof(GLfloat));
    shader->setAttributeBuffer("color", GL_FLOAT, 1 * sizeof(GLfloat), 3, 4 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint pointLocation = shader->attributeLocation("point");
    GLuint sizeLocation = shader->attributeLocation("size");
    GLuint colorLocation = shader->attributeLocation("color");
#if !defined(Q_OS_MAC)
    m_funcs->glVertexAttribDivisor(pointLocation, 1);
    m_funcs->glVertexAttribDivisor(sizeLocation, 1);
    m_funcs->glVertexAttribDivisor(colorLocation, 1);
#else
    m_instanceFuncs->glVertexAttribDivisorARB(pointLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(sizeLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(colorLocation, 1);
#endif
    m_sphere->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::prepareEdgeVAO()
{
    // Bind the marker's VAO
    m_cylinder->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = m_cylinder->material()->shader();
    shader->bind();
    m_edgePositionBuffer.bind();
    shader->enableAttributeArray("source");
    shader->enableAttributeArray("target");
    shader->setAttributeBuffer("source", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    shader->setAttributeBuffer("target", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    m_edgeVisualBuffer.bind();
    shader->enableAttributeArray("size");
    shader->enableAttributeArray("color");
    shader->setAttributeBuffer("size", GL_FLOAT, 0, 1, 4 * sizeof(GLfloat));
    shader->setAttributeBuffer("color", GL_FLOAT, 1 * sizeof(GLfloat), 3, 4 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint sourcePointLocation = shader->attributeLocation("source");
    GLuint targetPointLocation = shader->attributeLocation("target");
    GLuint sizeLocation = shader->attributeLocation("size");
    GLuint colorLocation = shader->attributeLocation("color");
#if !defined(Q_OS_MAC)
    m_funcs->glVertexAttribDivisor(sourcePointLocation, 1);
    m_funcs->glVertexAttribDivisor(targetPointLocation, 1);
    m_funcs->glVertexAttribDivisor(sizeLocation, 1);
    m_funcs->glVertexAttribDivisor(colorLocation, 1);
#else
    m_instanceFuncs->glVertexAttribDivisorARB(sourcePointLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(targetPointLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(sizeLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(colorLocation, 1);
#endif
    m_cylinder->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::prepareComponentMarkerVAO()
{
    // Bind the marker's VAO
    m_quad->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = m_quad->material()->shader();
    shader->bind();
    m_componentMarkerDataBuffer.bind();
    shader->enableAttributeArray("point");
    shader->enableAttributeArray("scale");
    shader->setAttributeBuffer("point", GL_FLOAT, 0, 2, 3 * sizeof(GLfloat));
    shader->setAttributeBuffer("scale", GL_FLOAT, 2 * sizeof(GLfloat), 1, 3 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint pointLocation = shader->attributeLocation("point");
    GLuint scaleLocation = shader->attributeLocation("scale");
#if !defined(Q_OS_MAC)
    m_funcs->glVertexAttribDivisor(pointLocation, 1);
    m_funcs->glVertexAttribDivisor(scaleLocation, 1);
#else
    m_instanceFuncs->glVertexAttribDivisorARB(pointLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(scaleLocation, 1);
#endif
    m_quad->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::prepareDebugLinesVAO()
{
    debugLinesDataVAO.bind();
    debugLinesShader.bind();
    debugLinesDataBuffer.bind();

    debugLinesShader.enableAttributeArray("position");
    debugLinesShader.enableAttributeArray("color");
    debugLinesShader.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    debugLinesShader.setAttributeBuffer("color", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    debugLinesDataVAO.release();
    debugLinesShader.release();
}
