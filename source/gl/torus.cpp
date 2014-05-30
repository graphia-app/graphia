#include "torus.h"

#include <QOpenGLShaderProgram>

#include <math.h>

const float pi = 3.14159265358979323846f;
const float twoPi = 2.0f * pi;

Torus::Torus(QObject* parent)
    : QObject(parent),
      _minorRadius(0.75f),
      _majorRadius(1.0f),
      _rings(50),
      _sides(50),
      _positionBuffer(QOpenGLBuffer::VertexBuffer),
      _normalBuffer(QOpenGLBuffer::VertexBuffer),
      _textureCoordBuffer(QOpenGLBuffer::VertexBuffer),
      _indexBuffer(QOpenGLBuffer::IndexBuffer),
      _vao()
{
}

void Torus::setMaterial(const MaterialPtr& material)
{
    if(material == _material)
        return;
    _material = material;
    updateVertexArrayObject();
}

MaterialPtr Torus::material() const
{
    return _material;
}

void Torus::create()
{
    int faces = _sides * _rings;
    int nVerts  = _sides *(_rings + 1);   // One extra ring to duplicate first ring

    // Allocate some storage to hold per-vertex data
    float* v = new float[3*nVerts];               // Vertices
    float* n = new float[3*nVerts];               // Normals
    float* tex = new float[2*nVerts];             // Tex coords
    unsigned int* el = new unsigned int[6*faces]; // Elements

    // Generate the vertex data
    generateVertexData(v, n, tex, el);

    // Create and populate the buffer objects
    _positionBuffer.create();
    _positionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _positionBuffer.bind();
    _positionBuffer.allocate(v, 3 * nVerts * sizeof(float));

    _normalBuffer.create();
    _normalBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _normalBuffer.bind();
    _normalBuffer.allocate(n, 3 * nVerts * sizeof(float));

    _textureCoordBuffer.create();
    _textureCoordBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _textureCoordBuffer.bind();
    _textureCoordBuffer.allocate(tex, 2 * nVerts * sizeof(float));

    _indexBuffer.create();
    _indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _indexBuffer.bind();
    _indexBuffer.allocate(el, 6 * faces * sizeof(unsigned int));

    // Delete our copy of the data as we no longer need it
    delete [] v;
    delete [] n;
    delete [] el;
    delete [] tex;

    updateVertexArrayObject();
}

void Torus::generateVertexData(float* vertices, float* normals, float* texCoords, unsigned int* indices)
{
    float ringFactor = twoPi / static_cast<float>(_rings);
    float sideFactor = twoPi / static_cast<float>(_sides);

    int index = 0, texCoordIndex = 0;
    for(int ring = 0; ring <= _rings; ring++)
    {
        float u = ring * ringFactor;
        float cu = cos(u);
        float su = sin(u);

        for(int side = 0; side < _sides; side++)
        {
            float v = side * sideFactor;
            float cv = cos(v);
            float sv = sin(v);
            float r = (_majorRadius + _minorRadius * cv);

            vertices[index] = r * cu;
            vertices[index+1] = r * su;
            vertices[index+2] = _minorRadius * sv;

            normals[index] = cv * cu * r;
            normals[index+1] = cv * su * r;
            normals[index+2] = sv * r;

            texCoords[texCoordIndex] = u / twoPi;
            texCoords[texCoordIndex+1] = v / twoPi;
            texCoordIndex += 2;

            // Normalize the normal vector
            float len = sqrt(normals[index] * normals[index] +
                              normals[index+1] * normals[index+1] +
                              normals[index+2] * normals[index+2]);
            normals[index] /= len;
            normals[index+1] /= len;
            normals[index+2] /= len;

            index += 3;
        }
    }

    index = 0;
    for(int ring = 0; ring < _rings; ring++)
    {
        int ringStart = ring * _sides;
        int nextRingStart = (ring + 1) * _sides;
        for(int side = 0; side < _sides; side++)
        {
            int nextSide = (side + 1) % _sides;

            // The quad
            indices[index] = (ringStart + side);
            indices[index+1] = (nextRingStart + side);
            indices[index+2] = (nextRingStart + nextSide);
            indices[index+3] = ringStart + side;
            indices[index+4] = nextRingStart + nextSide;
            indices[index+5] = (ringStart + nextSide);
            index += 6;
        }
    }
}

void Torus::updateVertexArrayObject()
{
    // Ensure that we have a valid material and geometry
    if(!_material || !_positionBuffer.isCreated())
        return;

    // Create a vertex array object
    if(!_vao.isCreated())
        _vao.create();
    _vao.bind();

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

    _indexBuffer.bind();

    // End VAO setup
    _vao.release();

    // Tidy up after ourselves
    _textureCoordBuffer.release();
    _indexBuffer.release();
}

void Torus::render()
{
    // Bind the vertex array oobject to set up our vertex buffers and index buffer
    _vao.bind();

    // Draw it!
    glDrawElements(GL_TRIANGLES, indexCount(), GL_UNSIGNED_INT, 0);

    _vao.release();
}
