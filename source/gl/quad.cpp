#include "quad.h"

#include <QOpenGLShaderProgram>

#include <cmath>

Quad::Quad(QObject* parent)
    : QObject(parent),
      m_edgeLength(1.0f),
      m_positionBuffer( QOpenGLBuffer::VertexBuffer ),
      m_normalBuffer( QOpenGLBuffer::VertexBuffer ),
      m_textureCoordBuffer( QOpenGLBuffer::VertexBuffer ),
      m_indexBuffer( QOpenGLBuffer::IndexBuffer ),
      m_vao(),
      m_normalLines( QOpenGLBuffer::VertexBuffer ),
      m_tangentLines( QOpenGLBuffer::VertexBuffer )
{
}

void Quad::setMaterial(const MaterialPtr& material)
{
    if ( material == m_material )
        return;
    m_material = material;
    updateVertexArrayObject();
}

MaterialPtr Quad::material() const
{
    return m_material;
}

void Quad::create()
{
    // Allocate some storage to hold per-vertex data
    QVector<float> v;         // Vertices
    QVector<float> n;         // Normals
    QVector<float> tex;       // Tex coords
    QVector<float> tang;      // Tangents
    QVector<unsigned int> el; // Element indices

    // Generate the vertex data
    generateVertexData(v, n, tex, tang, el);

    // Create and populate the buffer objects
    m_positionBuffer.create();
    m_positionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_positionBuffer.bind();
    m_positionBuffer.allocate(v.constData(), v.size() * sizeof(float));

    m_normalBuffer.create();
    m_normalBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_normalBuffer.bind();
    m_normalBuffer.allocate(n.constData(), n.size() * sizeof(float));

    m_textureCoordBuffer.create();
    m_textureCoordBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_textureCoordBuffer.bind();
    m_textureCoordBuffer.allocate(tex.constData(), tex.size() * sizeof(float));

    m_tangentBuffer.create();
    m_tangentBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_tangentBuffer.bind();
    m_tangentBuffer.allocate(tang.constData(), tang.size() * sizeof(float));

    m_indexBuffer.create();
    m_indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_indexBuffer.bind();
    m_indexBuffer.allocate(el.constData(), el.size() * sizeof(unsigned int));

    updateVertexArrayObject();
}

void Quad::generateVertexData( QVector<float>& vertices, QVector<float>& normals,
                                QVector<float>& texCoords, QVector<float>& tangents,
                                QVector<unsigned int>& indices  )
{
    // Resize vector to hold our data
    vertices.resize( 12 );
    normals.resize( 12 );
    indices.resize( 6 );

    int index = 0; //, texCoordIndex = 0, tangentIndex = 0;
    float halfEdgeLength = m_edgeLength * 0.5f;

    vertices[index++] = -halfEdgeLength;
    vertices[index++] = -halfEdgeLength;
    vertices[index++] = 0.0f;

    vertices[index++] = halfEdgeLength;
    vertices[index++] = -halfEdgeLength;
    vertices[index++] = 0.0f;

    vertices[index++] = halfEdgeLength;
    vertices[index++] = halfEdgeLength;
    vertices[index++] = 0.0f;

    vertices[index++] = -halfEdgeLength;
    vertices[index++] = halfEdgeLength;
    vertices[index++] = 0.0f;

    index = 0;
    for(int i = 0; i < 4; i++)
    {
        normals[index++] = 0.0f;
        normals[index++] = 0.0f;
        normals[index++] = 1.0f;
    }

    int elIndex = 0;
    indices[elIndex++] = 2;
    indices[elIndex++] = 0;
    indices[elIndex++] = 1;
    indices[elIndex++] = 0;
    indices[elIndex++] = 2;
    indices[elIndex++] = 3;
}

void Quad::updateVertexArrayObject()
{
    // Ensure that we have a valid material and geometry
    if ( !m_material || !m_positionBuffer.isCreated() )
        return;

    // Create a vertex array object
    if ( !m_vao.isCreated() )
        m_vao.create();
    m_vao.bind();

    bindBuffers();

    // End VAO setup
    m_vao.release();

    // Tidy up after ourselves
    m_tangentBuffer.release();
    m_indexBuffer.release();
}

void Quad::bindBuffers()
{
    QOpenGLShaderProgramPtr shader = m_material->shader();
    shader->bind();

    m_positionBuffer.bind();
    shader->enableAttributeArray( "vertexPosition" );
    shader->setAttributeBuffer( "vertexPosition", GL_FLOAT, 0, 3 );

    m_normalBuffer.bind();
    shader->enableAttributeArray( "vertexNormal" );
    shader->setAttributeBuffer( "vertexNormal", GL_FLOAT, 0, 3 );

    m_textureCoordBuffer.bind();
    shader->enableAttributeArray( "vertexTexCoord" );
    shader->setAttributeBuffer( "vertexTexCoord", GL_FLOAT, 0, 2 );

    m_tangentBuffer.bind();
    shader->enableAttributeArray( "vertexTangent" );
    shader->setAttributeBuffer( "vertexTangent", GL_FLOAT, 0, 4 );

    m_indexBuffer.bind();
}
