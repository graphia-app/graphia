#include "sphere.h"

#include <QOpenGLShaderProgram>

#include <math.h>

const float pi = 3.14159265358979323846f;
const float twoPi = 2.0f * pi;

Sphere::Sphere( QObject* parent )
    : QObject( parent ),
      m_radius( 1.0f ),
      m_rings( 30 ),
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

void Sphere::setMaterial( const MaterialPtr& material )
{
    if ( material == m_material )
        return;
    m_material = material;
    updateVertexArrayObject();
}

MaterialPtr Sphere::material() const
{
    return m_material;
}

void Sphere::create()
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

void Sphere::generateVertexData( QVector<float>& vertices, QVector<float>& normals,
                                QVector<float>& texCoords, QVector<float>& tangents,
                                QVector<unsigned int>& indices  )
{
    int faces = (m_slices - 2) * m_rings +           // Number of "rectangular" faces
            (m_rings * 2); // and one ring for the top and bottom caps
    int nVerts  = ( m_slices + 1 ) * ( m_rings + 1 ); // One extra line of latitude

    // Resize vector to hold our data
    vertices.resize( 3 * nVerts );
    normals.resize( 3 * nVerts );
    tangents.resize( 4 * nVerts );
    texCoords.resize( 2 * nVerts );
    indices.resize( 6 * faces );

    const float dTheta = twoPi / static_cast<float>( m_slices );
    const float dPhi = pi / static_cast<float>( m_rings );
    const float du = 1.0f / static_cast<float>( m_slices );
    const float dv = 1.0f / static_cast<float>( m_rings );

    // Iterate over latitudes (rings)
    int index = 0, texCoordIndex = 0, tangentIndex = 0;
    for ( int lat = 0; lat < m_rings + 1; ++lat )
    {
        const float phi = pi / 2.0f - static_cast<float>( lat ) * dPhi;
        const float cosPhi = cosf( phi );
        const float sinPhi = sinf( phi );
        const float v = 1.0f - static_cast<float>( lat ) * dv;

        // Iterate over longitudes (slices)
        for ( int lon = 0; lon < m_slices + 1; ++lon )
        {
            const float theta = static_cast<float>( lon ) * dTheta;
            const float cosTheta = cosf( theta );
            const float sinTheta = sinf( theta );
            const float u = static_cast<float>( lon ) * du;

            vertices[index]   = m_radius * cosTheta * cosPhi;
            vertices[index+1] = m_radius * sinPhi;
            vertices[index+2] = m_radius * sinTheta * cosPhi;

            normals[index]   = cosTheta * cosPhi;
            normals[index+1] = sinPhi;
            normals[index+2] = sinTheta * cosPhi;

            tangents[tangentIndex] = sinTheta;
            tangents[tangentIndex + 1] = 0.0;
            tangents[tangentIndex + 2] = -cosTheta;
            tangents[tangentIndex + 3] = 1.0;
            tangentIndex += 4;

            index += 3;

            texCoords[texCoordIndex] = u;
            texCoords[texCoordIndex+1] = v;

            texCoordIndex += 2;


        }
    }

    int elIndex = 0;

    // top cap
    {
        const int nextRingStartIndex = m_slices + 1;
        for ( int j = 0; j < m_slices; ++j )
        {
            indices[elIndex] = nextRingStartIndex + j;
            indices[elIndex+1] = 0;
            indices[elIndex+2] = nextRingStartIndex + j + 1;
            elIndex += 3;
        }
    }

    for ( int i = 1; i < (m_rings - 1); ++i )
    {
        const int ringStartIndex = i * ( m_slices + 1 );
        const int nextRingStartIndex = ( i + 1 ) * ( m_slices + 1 );

        for ( int j = 0; j < m_slices; ++j )
        {
            // Split the quad into two triangles
            indices[elIndex]   = ringStartIndex + j;
            indices[elIndex+1] = ringStartIndex + j + 1;
            indices[elIndex+2] = nextRingStartIndex + j;
            indices[elIndex+3] = nextRingStartIndex + j;
            indices[elIndex+4] = ringStartIndex + j + 1;
            indices[elIndex+5] = nextRingStartIndex + j + 1;

            elIndex += 6;
        }
    }

    // bottom cap
    {
        const int ringStartIndex = (m_rings - 1) * ( m_slices + 1);
        const int nextRingStartIndex = (m_rings) * ( m_slices + 1);
        for ( int j = 0; j < m_slices; ++j )
        {
            indices[elIndex] = ringStartIndex + j + 1;
            indices[elIndex+1] = nextRingStartIndex;
            indices[elIndex+2] = ringStartIndex + j;
            elIndex += 3;
        }
    }
}

void Sphere::updateVertexArrayObject()
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

void Sphere::render()
{
    // Bind the vertex array oobject to set up our vertex buffers and index buffer
    m_vao.bind();

    // Draw it!
    glDrawElements( GL_TRIANGLES, indexCount(), GL_UNSIGNED_INT, 0 );

    m_vao.release();
}

void Sphere::computeNormalLinesBuffer( const MaterialPtr& mat, double scale )
{

    int nVerts  = ( m_slices + 1 ) * ( m_rings + 1 ); // One extra line of latitude
    float* v = new float[6 * nVerts];
    float* vPtr = v;

    m_positionBuffer.bind();
    float* p = reinterpret_cast<float*>( m_positionBuffer.map( QOpenGLBuffer::ReadOnly ) );

    m_normalBuffer.bind();
    float* n = reinterpret_cast<float*>( m_normalBuffer.map( QOpenGLBuffer::ReadOnly ) );

    Q_ASSERT(n);
    Q_ASSERT(p);
    for ( int vIndex = 0; vIndex < nVerts; ++vIndex )
    {
        float x = *p++,
                y = *p++,
                z = *p++;

        *vPtr++ = x;
        *vPtr++ = y;
        *vPtr++ = z;
        *vPtr++ = x + (*n++ * scale);
        *vPtr++ = y + (*n++ * scale);
        *vPtr++ = z + (*n++ * scale);
    }

    m_normalBuffer.unmap();
    m_positionBuffer.bind();
    m_positionBuffer.unmap();

    m_normalLines.create();
    m_normalLines.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_normalLines.bind();
    m_normalLines.allocate( v, 6 * nVerts * sizeof( float ) );

    m_vaoNormals.create();
    m_vaoNormals.bind();
    mat->shader()->enableAttributeArray( "vertexPosition" );
    mat->shader()->setAttributeBuffer( "vertexPosition", GL_FLOAT, 0, 3 );
    m_vaoNormals.release();
    m_normalLines.release();

    delete[] v;
}

void Sphere::computeTangentLinesBuffer( const MaterialPtr& mat, double scale )
{
    int nVerts  = ( m_slices + 1 ) * ( m_rings + 1 ); // One extra line of latitude
    float* v = new float[6 * nVerts];
    float* vPtr = v;

    m_tangentBuffer.bind();
    float* t = reinterpret_cast<float*>( m_tangentBuffer.map( QOpenGLBuffer::ReadOnly ) );

    m_positionBuffer.bind();
    float* p = reinterpret_cast<float*>( m_positionBuffer.map( QOpenGLBuffer::ReadOnly ) );

    Q_ASSERT(t);
    Q_ASSERT(p);
    for ( int vIndex = 0; vIndex < nVerts; ++vIndex )
    {
        float x = *p++,
                y = *p++,
                z = *p++;

        *vPtr++ = x;
        *vPtr++ = y;
        *vPtr++ = z;
        *vPtr++ = x + (*t++ * scale);
        *vPtr++ = y + (*t++ * scale);
        *vPtr++ = z + (*t++ * scale);
        t++; // skip fourth tangent value
    }

    m_positionBuffer.unmap();
    m_tangentBuffer.bind();
    m_tangentBuffer.unmap();

    m_tangentLines.create();
    m_tangentLines.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_tangentLines.bind();
    m_tangentLines.allocate( v, 6 * nVerts * sizeof( float ) );

    m_vaoTangents.create();
    m_vaoTangents.bind();
    mat->shader()->enableAttributeArray( "vertexPosition" );
    mat->shader()->setAttributeBuffer( "vertexPosition", GL_FLOAT, 0, 3 );
    m_vaoTangents.release();
    m_tangentLines.release();

    delete[] v;
}


void Sphere::renderNormalLines()
{
    int nVerts  = ( m_slices + 1 ) * ( m_rings + 1 ); // One extra line of latitude

    m_vaoNormals.bind();
    glDrawArrays( GL_LINES, 0, nVerts * 2 );
    m_vaoNormals.release();
}

void Sphere::renderTangentLines()
{
    int nVerts  = ( m_slices + 1 ) * ( m_rings + 1 ); // One extra line of latitude

    m_vaoTangents.bind();
    glDrawArrays( GL_LINES, 0, nVerts * 2 );
    m_vaoTangents.release();
}

void Sphere::bindBuffers()
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
