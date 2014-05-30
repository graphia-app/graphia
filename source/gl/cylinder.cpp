#include "cylinder.h"

#include <QOpenGLShaderProgram>

#include <math.h>

const float pi = 3.14159265358979323846f;
const float twoPi = 2.0f * pi;

Cylinder::Cylinder( QObject* parent )
    : QObject( parent ),
      _radius( 1.0f ),
      _length( 2.0f ),
      _slices( 30 ),
      _positionBuffer( QOpenGLBuffer::VertexBuffer ),
      _normalBuffer( QOpenGLBuffer::VertexBuffer ),
      _textureCoordBuffer( QOpenGLBuffer::VertexBuffer ),
      _indexBuffer( QOpenGLBuffer::IndexBuffer ),
      _vao(),
      _normalLines( QOpenGLBuffer::VertexBuffer ),
      _tangentLines( QOpenGLBuffer::VertexBuffer )
{
}

void Cylinder::setMaterial( const MaterialPtr& material )
{
    if ( material == _material )
        return;
    _material = material;
    updateVertexArrayObject();
}

MaterialPtr Cylinder::material() const
{
    return _material;
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
    _positionBuffer.create();
    _positionBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    _positionBuffer.bind();
    _positionBuffer.allocate( v.constData(), v.size() * sizeof( float ) );

    _normalBuffer.create();
    _normalBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    _normalBuffer.bind();
    _normalBuffer.allocate( n.constData(), n.size() * sizeof( float ) );

    _textureCoordBuffer.create();
    _textureCoordBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    _textureCoordBuffer.bind();
    _textureCoordBuffer.allocate( tex.constData(), tex.size() * sizeof( float ) );

    _tangentBuffer.create();
    _tangentBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    _tangentBuffer.bind();
    _tangentBuffer.allocate( tang.constData(), tang.size() * sizeof( float )  );

    _indexBuffer.create();
    _indexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    _indexBuffer.bind();
    _indexBuffer.allocate( el.constData(), el.size() * sizeof( unsigned int ) );

    updateVertexArrayObject();
}

void Cylinder::generateVertexData( QVector<float>& vertices, QVector<float>& normals,
                                QVector<float>& texCoords, QVector<float>& tangents,
                                QVector<unsigned int>& indices  )
{
    int faces = (_slices);
    int nVerts  = ((_slices + 1) * 2);

    // Resize vector to hold our data
    vertices.resize( 3 * nVerts );
    normals.resize( 3 * nVerts );
    //tangents.resize( 4 * nVerts );
    //texCoords.resize( 2 * nVerts );
    indices.resize( 6 * faces );

    const float dTheta = twoPi / static_cast<float>( _slices );
    //const float du = 1.0f / static_cast<float>( _slices );
    //const float dv = 1.0f / static_cast<float>( _rings );

    int index = 0; //, texCoordIndex = 0, tangentIndex = 0;

    // Iterate over longitudes (slices)
    for( int slice = 0; slice < _slices + 1; slice++ )
    {
        const float theta = static_cast<float>( slice ) * dTheta;
        const float cosTheta = cosf( theta );
        const float sinTheta = sinf( theta );
        //const float u = static_cast<float>( slice ) * du;

        vertices[index+0] = _radius * cosTheta;
        vertices[index+1] = _length * 0.5f;
        vertices[index+2] = _radius * sinTheta;

        normals[index+0] = cosTheta;
        normals[index+1] = 0.0f;
        normals[index+2] = sinTheta;

        index += 3;

        vertices[index+0] = _radius * cosTheta;
        vertices[index+1] = _length * -0.5f;
        vertices[index+2] = _radius * sinTheta;

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

    for(int slice = 0; slice < _slices; slice++)
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
    if ( !_material || !_positionBuffer.isCreated() )
        return;

    // Create a vertex array object
    if ( !_vao.isCreated() )
        _vao.create();
    _vao.bind();

    bindBuffers();

    // End VAO setup
    _vao.release();

    // Tidy up after ourselves
    _tangentBuffer.release();
    _indexBuffer.release();
}

void Cylinder::bindBuffers()
{
    QOpenGLShaderProgramPtr shader = _material->shader();
    shader->bind();

    _positionBuffer.bind();
    shader->enableAttributeArray( "vertexPosition" );
    shader->setAttributeBuffer( "vertexPosition", GL_FLOAT, 0, 3 );

    _normalBuffer.bind();
    shader->enableAttributeArray( "vertexNormal" );
    shader->setAttributeBuffer( "vertexNormal", GL_FLOAT, 0, 3 );

    _textureCoordBuffer.bind();
    shader->enableAttributeArray( "vertexTexCoord" );
    shader->setAttributeBuffer( "vertexTexCoord", GL_FLOAT, 0, 2 );

    _tangentBuffer.bind();
    shader->enableAttributeArray( "vertexTangent" );
    shader->setAttributeBuffer( "vertexTangent", GL_FLOAT, 0, 4 );

    _indexBuffer.bind();
}
