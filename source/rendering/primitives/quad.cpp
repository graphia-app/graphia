#include "quad.h"

#include <QOpenGLShaderProgram>

#include <cmath>

Quad::Quad(QObject* parent)
    : QObject(parent),
      _edgeLength(1.0f),
      _positionBuffer(QOpenGLBuffer::VertexBuffer),
      _normalBuffer(QOpenGLBuffer::VertexBuffer),
      _textureCoordBuffer(QOpenGLBuffer::VertexBuffer),
      _indexBuffer(QOpenGLBuffer::IndexBuffer),
      _vao(),
      _normalLines(QOpenGLBuffer::VertexBuffer),
      _tangentLines(QOpenGLBuffer::VertexBuffer)
{
}

void Quad::setMaterial(const MaterialPtr& material)
{
    if(material == _material)
        return;
    _material = material;
    updateVertexArrayObject();
}

MaterialPtr Quad::material() const
{
    return _material;
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
    _positionBuffer.create();
    _positionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _positionBuffer.bind();
    _positionBuffer.allocate(v.constData(), v.size() * sizeof(float));

    _normalBuffer.create();
    _normalBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _normalBuffer.bind();
    _normalBuffer.allocate(n.constData(), n.size() * sizeof(float));

    _textureCoordBuffer.create();
    _textureCoordBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _textureCoordBuffer.bind();
    _textureCoordBuffer.allocate(tex.constData(), tex.size() * sizeof(float));

    _tangentBuffer.create();
    _tangentBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _tangentBuffer.bind();
    _tangentBuffer.allocate(tang.constData(), tang.size() * sizeof(float));

    _indexBuffer.create();
    _indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _indexBuffer.bind();
    _indexBuffer.allocate(el.constData(), el.size() * sizeof(unsigned int));

    updateVertexArrayObject();
}

void Quad::generateVertexData(QVector<float>& vertices, QVector<float>& normals,
                                QVector<float>& /*texCoords*/, QVector<float>& /*tangents*/,
                                QVector<unsigned int>& indices)
{
    // Resize vector to hold our data
    vertices.resize(12);
    normals.resize(12);
    indices.resize(6);

    int index = 0; //, texCoordIndex = 0, tangentIndex = 0;
    float halfEdgeLength = _edgeLength * 0.5f;

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
    if(!_material || !_positionBuffer.isCreated())
        return;

    // Create a vertex array object
    if(!_vao.isCreated())
        _vao.create();
    _vao.bind();

    bindBuffers();

    // End VAO setup
    _vao.release();

    // Tidy up after ourselves
    _tangentBuffer.release();
    _indexBuffer.release();
}

void Quad::bindBuffers()
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
    shader->setAttributeBuffer("vertexTangent", GL_FLOAT, 0, 4);

    _indexBuffer.bind();
}
