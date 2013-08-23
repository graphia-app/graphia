#include "graphscene.h"

#include "camera.h"
#include "sphere.h"
#include "cylinder.h"
#include "material.h"

#include "../graph/graphmodel.h"

#include <QObject>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFunctions_3_2_Core>
#if defined(Q_OS_MAC)
#include <QOpenGLExtensions>
#endif

#include <QMutexLocker>

#include <math.h>

GraphScene::GraphScene( QObject* parent )
    : AbstractScene( parent ),
      m_funcs(nullptr),
#if defined(Q_OS_MAC)
      m_instanceFuncs( 0 ),
#endif
      m_camera( new Camera( this ) ),
      m_vx( 0.0f ),
      m_vy( 0.0f ),
      m_vz( 0.0f ),
      m_viewCenterFixed( false ),
      m_sphere(nullptr),
      m_cylinder(nullptr),
      m_theta( 0.0f ),
      m_modelMatrix(),
      _graphModel(nullptr),
      m_nodePositionData(0),
      m_edgePositionData(0)
{
    m_modelMatrix.setToIdentity();
    update( 0.0f );

    // Initialize the camera position and orientation
    m_camera->setPosition( QVector3D( 0.0f, 0.0f, 20.0f ) );
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
    m_sphere->setRadius(0.8f);
    m_sphere->setRings(12);
    m_sphere->setSlices(12);
    m_sphere->setMaterial(nodeMaterial);
    m_sphere->create();

    MaterialPtr edgeMaterial(new Material);
    edgeMaterial->setShaders(":/gl/shaders/instancededges.vert", ":/gl/shaders/ads.frag" );

    m_cylinder = new Cylinder(this);
    m_cylinder->setRadius(0.2f);
    m_cylinder->setLength(1.0f);
    m_cylinder->setSlices(8);
    m_cylinder->setMaterial(edgeMaterial);
    m_cylinder->create();

    // Create a pair of VBOs ready to hold our data
    prepareVertexBuffers();

    // Tell OpenGL how to pass the data VBOs to the shader program
    prepareNodeVAO();
    prepareEdgeVAO();

    // Enable depth testing to prevent artifacts
    glEnable( GL_DEPTH_TEST );

    // Cull back facing triangles to save the gpu some work
    glEnable( GL_CULL_FACE );

    glClearColor( 0.75f, 0.75f, 0.75f, 1.0f );
}

void GraphScene::update( float /*t*/ )
{
    if(_graphModel != nullptr)
    {
        NodeArray<QVector3D>& layout = _graphModel->layout();
        QMutexLocker locker(&layout.mutex());

        m_nodePositionData.resize(_graphModel->graph().numNodes() * 3);
        int i = 0;

        for(auto position : layout)
        {
            m_nodePositionData[i++] = position.x();
            m_nodePositionData[i++] = position.y();
            m_nodePositionData[i++] = position.z();
        }

        Q_ASSERT(i == m_nodePositionData.size());

        m_edgePositionData.resize(_graphModel->graph().numEdges() * 6);
        i = 0;

        for(EdgeId edgeId : _graphModel->graph().edgeIds())
        {
            const Edge& edge = _graphModel->graph().edgeById(edgeId);
            QVector3D& sourcePosition = layout[edge.sourceId()];
            QVector3D& targetPosition = layout[edge.targetId()];

            m_edgePositionData[i++] = sourcePosition.x();
            m_edgePositionData[i++] = sourcePosition.y();
            m_edgePositionData[i++] = sourcePosition.z();
            m_edgePositionData[i++] = targetPosition.x();
            m_edgePositionData[i++] = targetPosition.y();
            m_edgePositionData[i++] = targetPosition.z();
        }

        Q_ASSERT(i == m_edgePositionData.size());
    }

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
    m_nodePositionDataBuffer.bind();
    m_nodePositionDataBuffer.allocate( m_nodePositionData.data(),
                                       m_nodePositionData.size() * sizeof(GLfloat) );

    // Bind the shader program
    QOpenGLShaderProgramPtr shader = m_sphere->material()->shader();
    shader->bind();

    // Calculate needed matrices
    m_modelMatrix.setToIdentity();
    m_modelMatrix.rotate( m_theta, 0.0f, 1.0f, 0.0f );

    //FIXME: use UBOs
    QMatrix4x4 modelViewMatrix = m_camera->viewMatrix() * m_modelMatrix;
    QMatrix3x3 normalMatrix = modelViewMatrix.normalMatrix();
    shader->setUniformValue( "modelViewMatrix", modelViewMatrix );
    shader->setUniformValue( "normalMatrix", normalMatrix );
    shader->setUniformValue( "projectionMatrix", m_camera->projectionMatrix() );

    // Set the lighting parameters
    shader->setUniformValue( "light.position", QVector4D( -10.0f, 10.0f, 0.0f, 1.0f ) );
    shader->setUniformValue( "light.intensity", QVector3D( 1.0f, 1.0f, 1.0f ) );
    shader->setUniformValue( "material.kd", QVector3D( 0.0f, 0.0f, 1.0f ) );
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
    m_edgePositionDataBuffer.bind();
    m_edgePositionDataBuffer.allocate( m_edgePositionData.data(),
                                       m_edgePositionData.size() * sizeof(GLfloat) );

    // Bind the shader program
    QOpenGLShaderProgramPtr shader = m_cylinder->material()->shader();
    shader->bind();

    //FIXME: use UBOs
    shader->setUniformValue("viewMatrix", m_camera->viewMatrix());
    shader->setUniformValue("projectionMatrix", m_camera->projectionMatrix());

    // Set the lighting parameters
    shader->setUniformValue( "light.position", QVector4D( -10.0f, 10.0f, 0.0f, 1.0f ) );
    shader->setUniformValue( "light.intensity", QVector3D( 1.0f, 1.0f, 1.0f ) );
    shader->setUniformValue( "material.kd", QVector3D( 0.0f, 1.0f, 0.0f ) );
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

void GraphScene::render()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    renderNodes();
    renderEdges();
}

void GraphScene::resize( int w, int h )
{
    // Make sure the viewport covers the entire window
    glViewport( 0, 0, w, h );

    // Update the projection matrix
    float aspect = static_cast<float>( w ) / static_cast<float>( h );
    m_camera->setPerspectiveProjection( 60.0f, aspect, 0.3f, 10000.0f );
}

void GraphScene::prepareVertexBuffers()
{
    // Populate the data buffer object
    m_nodePositionDataBuffer.create();
    m_nodePositionDataBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_nodePositionDataBuffer.bind();
    m_nodePositionDataBuffer.allocate( m_nodePositionData.data(), m_nodePositionData.size() * sizeof(GLfloat) );

    m_edgePositionDataBuffer.create();
    m_edgePositionDataBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_edgePositionDataBuffer.bind();
    m_edgePositionDataBuffer.allocate( m_edgePositionData.data(), m_edgePositionData.size() * sizeof(GLfloat) );
}

void GraphScene::prepareNodeVAO()
{
    // Bind the marker's VAO
    m_sphere->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = m_sphere->material()->shader();
    shader->bind();
    m_nodePositionDataBuffer.bind();
    shader->enableAttributeArray("point");
    shader->setAttributeBuffer("point", GL_FLOAT, 0, 3);

    // We only vary the point attribute once per instance
    GLuint pointLocation = shader->attributeLocation("point");
#if !defined(Q_OS_MAC)
    m_funcs->glVertexAttribDivisor(pointLocation, 1);
#else
    m_instanceFuncs->glVertexAttribDivisorARB(pointLocation, 1);
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
    m_edgePositionDataBuffer.bind();
    shader->enableAttributeArray("source");
    shader->enableAttributeArray("target");
    shader->setAttributeBuffer("source", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    shader->setAttributeBuffer("target", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint sourcePointLocation = shader->attributeLocation("source");
    GLuint targetPointLocation = shader->attributeLocation("target");
#if !defined(Q_OS_MAC)
    m_funcs->glVertexAttribDivisor(sourcePointLocation, 1);
    m_funcs->glVertexAttribDivisor(targetPointLocation, 1);
#else
    m_instanceFuncs->glVertexAttribDivisorARB(sourcePointLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(targetPointLocation, 1);
#endif
    m_cylinder->vertexArrayObject()->release();
    shader->release();
}
