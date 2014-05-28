#include "cylinder.h"

#include <QOpenGLShaderProgram>

#include <math.h>

const float pi = 3.14159265358979323846f;
const float twoPi = 2.0f * pi;

Cylinder::Cylinder( QObject* parent )
    : QObject( parent ),
      m_radius( 1.0f ),
      m_length( 2.0f ),
      m_slices( 30 ),
      m_positionBuffer( QOpenGLBuffer::VertexBuffer ),
      m_normalBuffer( QOpenGLBuffer::VertexBuffer ),
      m_textureCoordBuffer( QOpenGLBuffer::VertexBuffer ),
      m_indexBuffer( QOpenGLBuffer::IndexBuffer ),
      m_vao(),
      m_normalLines( QOpenGLBuffer::VertexBuffer ),
      m_tangentLines( QOpenGLBuffer::VertexBuffer )
{
}

void Cylinder::setMaterial( const MaterialPtr& material )
{
    if ( material == m_material )
        return;
    m_material = material;
    updateVertexArrayObject();
}

MaterialPtr Cylinder::material() const
{
    return m_material;
}

void Cylinder::create()
{
    // Allocate some storage to hold per-vertex data
    QVector<float> v;         // Vertices
    QVector<float> n;         // Normals
    QVector<float> tex;       // Tex coords
    QVector<float> tang;      // Tangents
    QVector<unsigned int> el; // Element indices

    // Generate the vertex data
    generateVertexData( v, n, tex, tang, el );

    // Create and populate the buffer objects
    m_positionBuffer.create();
    m_positionBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_positionBuffer.bind();
    m_positionBuffer.allocate( v.constData(), v.size() * sizeof( float ) );

    m_normalBuffer.create();
    m_normalBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_normalBuffer.bind();
    m_normalBuffer.allocate( n.constData(), n.size() * sizeof( float ) );

    m_textureCoordBuffer.create();
    m_textureCoordBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_textureCoordBuffer.bind();
    m_textureCoordBuffer.allocate( tex.constData(), tex.size() * sizeof( float ) );

    m_tangentBuffer.create();
    m_tangentBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_tangentBuffer.bind();
    m_tangentBuffer.allocate( tang.constData(), tang.size() * sizeof( float )  );

    m_indexBuffer.create();
    m_indexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_indexBuffer.bind();
    m_indexBuffer.allocate( el.constData(), el.size() * sizeof( unsigned int ) );

    updateVertexArrayObject();
}

void Cylinder::generateVertexData( QVector<float>& vertices, QVector<float>& normals,
                                QVector<float>& texCoords, QVector<float>& tangents,
                                QVector<unsigned int>& indices  )
{
    int faces = (m_slices);
    int nVerts  = ((m_slices + 1) * 2);

    // Resize vector to hold our data
    vertices.resize( 3 * nVerts );
    normals.resize( 3 * nVerts );
    //tangents.resize( 4 * nVerts );
    //texCoords.resize( 2 * nVerts );
    indices.resize( 6 * faces );

    const float dTheta = twoPi / static_cast<float>( m_slices );
    //const float du = 1.0f / static_cast<float>( m_slices );
    //const float dv = 1.0f / static_cast<float>( m_rings );

    int index = 0; //, texCoordIndex = 0, tangentIndex = 0;

    // Iterate over longitudes (slices)
    for( int slice = 0; slice < m_slices + 1; slice++ )
    {
        const float theta = static_cast<float>( slice ) * dTheta;
        const float cosTheta = cosf( theta );
        const float sinTheta = sinf( theta );
        //const float u = static_cast<float>( slice ) * du;

        vertices[index+0] = m_radius * cosTheta;
        vertices[index+1] = m_length * 0.5f;
        vertices[index+2] = m_radius * sinTheta;

        normals[index+0] = cosTheta;
        normals[index+1] = 0.0f;
        normals[index+2] = sinTheta;

        index += 3;

        vertices[index+0] = m_radius * cosTheta;
        vertices[index+1] = m_length * -0.5f;
        vertices[index+2] = m_radius * sinTheta;

        normals[index+0] = cosTheta;
        normals[index+1] = 0.0f;
        normals[index+2] = sinTheta;

        index += 3;

        /*tangents[tangentIndex] = sinTheta;
        tangents[tangentIndex + 1] = 0.0;
        tangents[tangentIndex + 2] = -cosTheta;
        tangents[tangentIndex + 3] = 1.0;
        tangentIndex += 4;

        texCoords[texCoordIndex] = u;
        texCoords[texCoordIndex+1] = v;

        texCoordIndex += 2;*/
    }

    int elIndex = 0;

    for(int slice = 0; slice < m_slices; slice++)
    {
        int baseIndex = slice * 2;

        indices[elIndex+0] = baseIndex + 3;
        indices[elIndex+1] = baseIndex + 1;
        indices[elIndex+2] = baseIndex + 0;
        indices[elIndex+3] = baseIndex + 0;
        indices[elIndex+4] = baseIndex + 2;
        indices[elIndex+5] = baseIndex + 3;

        elIndex += 6;
    }
}

void Cylinder::updateVertexArrayObject()
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

void Cylinder::bindBuffers()
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
