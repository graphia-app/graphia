#include "cube.h"

#include <QOpenGLShaderProgram>

// Vertices
//
// 3 floats per vertex
// 4 vertices per face
// 6 faces
static const int vertexDataCount = 6 * 4 * 3;

static const float vertexData[vertexDataCount] = {
    // Left face
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,

    // Top face
    -0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f,  0.5f,
     0.5f, 0.5f,  0.5f,
     0.5f, 0.5f, -0.5f,

    // Right face
    0.5f,  0.5f, -0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f, -0.5f, -0.5f,

    // Bottom face
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    // Front face
     0.5f, -0.5f, 0.5f,
     0.5f,  0.5f, 0.5f,
    -0.5f,  0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,

    // Back face
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f
};


// Normal vectors
static const int normalDataCount = 6 * 4 * 3;

static const float normalData[normalDataCount] = {
    // Left face
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,

    // Top face
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,

    // Right face
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,

    // Bottom face
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,

    // Front face
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,

    // Back face
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f
};


// Texure coords
static const int textureCoordDataCount = 6 * 4 * 2;

static const float textureCoordData[textureCoordDataCount] = {
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f
};


static const int tangentDataCount = 6 * 4 * 3;

static const float tangentData[tangentDataCount] = {
    // Left face
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,

    // Top face
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,

    // Right face
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,

    // Bottom face
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,

    // Front face
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,

    // Back face
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f
};


// Indices
//
// 3 indices per triangle
// 2 triangles per face
// 6 faces
static const int indexDataCount = 6 * 3 * 2;

static const unsigned int indexData[indexDataCount] = {
    0,  1,  2,  0,  2,  3,  // Left face
    4,  5,  6,  4,  6,  7,  // Top face
    8,  9,  10, 8,  10, 11, // Right face
    12, 14, 15, 12, 13, 14, // Bottom face
    16, 17, 18, 16, 18, 19, // Front face
    20, 22, 23, 20, 21, 22  // Back face
};

Cube::Cube(QObject* parent)
    : QObject(parent),
      _length(1.0),
      _positionBuffer(QOpenGLBuffer::VertexBuffer),
      _normalBuffer(QOpenGLBuffer::VertexBuffer),
      _textureCoordBuffer(QOpenGLBuffer::VertexBuffer),
      _tangentBuffer(QOpenGLBuffer::VertexBuffer),
      _indexBuffer(QOpenGLBuffer::IndexBuffer),
      _vao()
{
}

void Cube::setMaterial(const MaterialPtr& material)
{
    if(material == _material)
        return;
    _material = material;
    updateVertexArrayObject();
}

MaterialPtr Cube::material() const
{
    return _material;
}

void Cube::create()
{
    // Allocate some storage to hold per-vertex data
    float* v = new float[vertexDataCount];               // Vertices
    float* n = new float[normalDataCount];               // Normals
    float* tex = new float[textureCoordDataCount];       // Tex coords
    float* t = new float[tangentDataCount];              // Tangents
    unsigned int* el = new unsigned int[indexDataCount]; // Elements

    // Generate the vertex data
    generateVertexData(v, n, tex, t, el);

    // Create and populate the buffer objects
    _positionBuffer.create();
    _positionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _positionBuffer.bind();
    _positionBuffer.allocate(v, vertexDataCount * sizeof(float));

    _normalBuffer.create();
    _normalBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _normalBuffer.bind();
    _normalBuffer.allocate(n, normalDataCount * sizeof(float));

    _textureCoordBuffer.create();
    _textureCoordBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _textureCoordBuffer.bind();
    _textureCoordBuffer.allocate(tex, textureCoordDataCount * sizeof(float));

    _tangentBuffer.create();
    _tangentBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _tangentBuffer.bind();
    _tangentBuffer.allocate(t, tangentDataCount * sizeof(float));

    _indexBuffer.create();
    _indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _indexBuffer.bind();
    _indexBuffer.allocate(el, indexDataCount * sizeof(unsigned int));

    // Delete our copy of the data as we no longer need it
    delete [] v;
    delete [] n;
    delete [] el;
    delete [] tex;
    delete [] t;

    updateVertexArrayObject();
}

void Cube::generateVertexData(float* vertices, float* normals, float* texCoords, float* tangents, unsigned int* indices)
{
    /** \todo Scale the vertex data by _length */
    for(int i = 0; i < vertexDataCount; i++)
        vertices[i] = (vertexData[i] * _length);
    //memcpy(vertices, vertexData, vertexDataCount * sizeof(float));
    memcpy(normals, normalData, normalDataCount * sizeof(float));
    memcpy(texCoords, textureCoordData, textureCoordDataCount * sizeof(float));
    memcpy(tangents, tangentData, tangentDataCount * sizeof(float));
    memcpy(indices, indexData, indexDataCount * sizeof(unsigned int));
}

void Cube::updateVertexArrayObject()
{
    // Ensure that we have a valid material and geometry
    if(!_material || !_positionBuffer.isCreated())
        return;

    // Create a vertex array object
    if(!_vao.isCreated())
        _vao.create();
    _vao.bind();

    bindBuffers();

    // End VAO setup
    _vao.release();
}

void Cube::render()
{
    // Bind the vertex array oobject to set up our vertex buffers and index buffer
    _vao.bind();

    // Draw it!
    glDrawElements(GL_TRIANGLES, indexCount(), GL_UNSIGNED_INT, 0);

    _vao.release();
}

void Cube::bindBuffers()
{
    QOpenGLShaderProgramPtr shader = _material->shader();
    shader->bind();

    _positionBuffer.bind();
    shader->enableAttributeArray("vertexPosition");
    shader->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    _normalBuffer.bind();
    shader->enableAttributeArray("vertexNormal");
    shader->setAttributeBuffer("vertexNormal", GL_FLOAT, 0, 3);

    _textureCoordBuffer.bind();
    shader->enableAttributeArray("vertexTexCoord");
    shader->setAttributeBuffer("vertexTexCoord", GL_FLOAT, 0, 2);

    _tangentBuffer.bind();
    shader->enableAttributeArray("vertexTangent");
    shader->setAttributeBuffer("vertexTangent", GL_FLOAT, 0, 3);

    _indexBuffer.bind();
}
