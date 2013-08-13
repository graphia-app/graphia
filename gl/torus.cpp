#include "torus.h"

#include <QOpenGLShaderProgram>

#include <math.h>

const float pi = 3.14159265358979323846;
const float twoPi = 2.0 * pi;

Torus::Torus( QObject* parent )
    : QObject( parent ),
      m_minorRadius( 0.75f ),
      m_majorRadius( 1.0f ),
      m_rings( 50 ),
      m_sides( 50 ),
      m_positionBuffer( QOpenGLBuffer::VertexBuffer ),
      m_normalBuffer( QOpenGLBuffer::VertexBuffer ),
      m_textureCoordBuffer( QOpenGLBuffer::VertexBuffer ),
      m_indexBuffer( QOpenGLBuffer::IndexBuffer ),
      m_vao()
{
}

void Torus::setMaterial( const MaterialPtr& material )
{
    if ( material == m_material )
        return;
    m_material = material;
    updateVertexArrayObject();
}

MaterialPtr Torus::material() const
{
    return m_material;
}

void Torus::create()
{
    int faces = m_sides * m_rings;
    int nVerts  = m_sides * ( m_rings + 1 );   // One extra ring to duplicate first ring

    // Allocate some storage to hold per-vertex data
    float* v = new float[3*nVerts];               // Vertices
    float* n = new float[3*nVerts];               // Normals
    float* tex = new float[2*nVerts];             // Tex coords
    unsigned int* el = new unsigned int[6*faces]; // Elements

    // Generate the vertex data
    generateVertexData( v, n, tex, el );

    // Create and populate the buffer objects
    m_positionBuffer.create();
    m_positionBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_positionBuffer.bind();
    m_positionBuffer.allocate( v, 3 * nVerts * sizeof( float ) );

    m_normalBuffer.create();
    m_normalBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_normalBuffer.bind();
    m_normalBuffer.allocate( n, 3 * nVerts * sizeof( float ) );

    m_textureCoordBuffer.create();
    m_textureCoordBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_textureCoordBuffer.bind();
    m_textureCoordBuffer.allocate( tex, 2 * nVerts * sizeof( float ) );

    m_indexBuffer.create();
    m_indexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_indexBuffer.bind();
    m_indexBuffer.allocate( el, 6 * faces * sizeof( unsigned int ) );

    // Delete our copy of the data as we no longer need it
    delete [] v;
    delete [] n;
    delete [] el;
    delete [] tex;

    updateVertexArrayObject();
}

void Torus::generateVertexData( float* vertices, float* normals, float* texCoords, unsigned int* indices )
{
    float ringFactor = twoPi / static_cast<float>( m_rings );
    float sideFactor = twoPi / static_cast<float>( m_sides );

    int index = 0, texCoordIndex = 0;
    for ( int ring = 0; ring <= m_rings; ring++ )
    {
        float u = ring * ringFactor;
        float cu = cos( u );
        float su = sin( u );

        for ( int side = 0; side < m_sides; side++ )
        {
            float v = side * sideFactor;
            float cv = cos( v );
            float sv = sin( v );
            float r = ( m_majorRadius + m_minorRadius * cv );

            vertices[index] = r * cu;
            vertices[index+1] = r * su;
            vertices[index+2] = m_minorRadius * sv;

            normals[index] = cv * cu * r;
            normals[index+1] = cv * su * r;
            normals[index+2] = sv * r;

            texCoords[texCoordIndex] = u / twoPi;
            texCoords[texCoordIndex+1] = v / twoPi;
            texCoordIndex += 2;

            // Normalize the normal vector
            float len = sqrt( normals[index] * normals[index] +
                              normals[index+1] * normals[index+1] +
                              normals[index+2] * normals[index+2] );
            normals[index] /= len;
            normals[index+1] /= len;
            normals[index+2] /= len;

            index += 3;
        }
    }

    index = 0;
    for ( int ring = 0; ring < m_rings; ring++ )
    {
        int ringStart = ring * m_sides;
        int nextRingStart = ( ring + 1 ) * m_sides;
        for ( int side = 0; side < m_sides; side++ )
        {
            int nextSide = ( side + 1 ) % m_sides;

            // The quad
            indices[index] = ( ringStart + side );
            indices[index+1] = ( nextRingStart + side );
            indices[index+2] = ( nextRingStart + nextSide );
            indices[index+3] = ringStart + side;
            indices[index+4] = nextRingStart + nextSide;
            indices[index+5] = ( ringStart + nextSide );
            index += 6;
        }
    }
}

void Torus::updateVertexArrayObject()
{
    // Ensure that we have a valid material and geometry
    if ( !m_material || !m_positionBuffer.isCreated() )
        return;

    // Create a vertex array object
    if ( !m_vao.isCreated() )
        m_vao.create();
    m_vao.bind();

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

    m_indexBuffer.bind();

    // End VAO setup
    m_vao.release();

    // Tidy up after ourselves
    m_textureCoordBuffer.release();
    m_indexBuffer.release();
}

void Torus::render()
{
    // Bind the vertex array oobject to set up our vertex buffers and index buffer
    m_vao.bind();

    // Draw it!
    glDrawElements( GL_TRIANGLES, indexCount(), GL_UNSIGNED_INT, 0 );

    m_vao.release();
}
