#include "sphere.h"

#include <QOpenGLShaderProgram>

#include <math.h>

const float pi = 3.14159265358979323846f;
const float twoPi = 2.0f * pi;

Sphere::Sphere(QObject* parent)
    : QObject(parent),
      _radius(1.0f),
      _rings(30),
      _slices(30),
      _positionBuffer(QOpenGLBuffer::VertexBuffer),
      _normalBuffer(QOpenGLBuffer::VertexBuffer),
      _textureCoordBuffer(QOpenGLBuffer::VertexBuffer),
      _indexBuffer(QOpenGLBuffer::IndexBuffer),
      _vao(),
      _normalLines(QOpenGLBuffer::VertexBuffer),
      _tangentLines(QOpenGLBuffer::VertexBuffer)
{
}

void Sphere::setMaterial(const MaterialPtr& material)
{
    if(material == _material)
        return;
    _material = material;
    updateVertexArrayObject();
}

MaterialPtr Sphere::material() const
{
    return _material;
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

void Sphere::generateVertexData(QVector<float>& vertices, QVector<float>& normals,
                                QVector<float>& texCoords, QVector<float>& tangents,
                                QVector<unsigned int>& indices)
{
    int faces = (_slices - 2) * _rings +           // Number of "rectangular" faces
                (_rings * 2); // and one ring for the top and bottom caps
    int nVerts  = (_slices + 1) *(_rings + 1); // One extra line of latitude

    // Resize vector to hold our data
    vertices.resize(3 * nVerts);
    normals.resize(3 * nVerts);
    tangents.resize(4 * nVerts);
    texCoords.resize(2 * nVerts);
    indices.resize(6 * faces);

    const float dTheta = twoPi / static_cast<float>(_slices);
    const float dPhi = pi / static_cast<float>(_rings);
    const float du = 1.0f / static_cast<float>(_slices);
    const float dv = 1.0f / static_cast<float>(_rings);

    // Iterate over latitudes (rings)
    int index = 0, texCoordIndex = 0, tangentIndex = 0;
    for(int lat = 0; lat < _rings + 1; ++lat)
    {
        const float phi = pi / 2.0f - static_cast<float>(lat) * dPhi;
        const float cosPhi = cosf(phi);
        const float sinPhi = sinf(phi);
        const float v = 1.0f - static_cast<float>(lat) * dv;

        // Iterate over longitudes (slices)
        for(int lon = 0; lon < _slices + 1; ++lon)
        {
            const float theta = static_cast<float>(lon) * dTheta;
            const float cosTheta = cosf(theta);
            const float sinTheta = sinf(theta);
            const float u = static_cast<float>(lon) * du;

            vertices[index]   = _radius * cosTheta * cosPhi;
            vertices[index+1] = _radius * sinPhi;
            vertices[index+2] = _radius * sinTheta * cosPhi;

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
        const int nextRingStartIndex = _slices + 1;
        for(int j = 0; j < _slices; ++j)
        {
            indices[elIndex] = nextRingStartIndex + j;
            indices[elIndex+1] = 0;
            indices[elIndex+2] = nextRingStartIndex + j + 1;
            elIndex += 3;
        }
    }

    for(int i = 1; i < (_rings - 1); ++i)
    {
        const int ringStartIndex = i *(_slices + 1);
        const int nextRingStartIndex = (i + 1) *(_slices + 1);

        for(int j = 0; j < _slices; ++j)
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
        const int ringStartIndex = (_rings - 1) *(_slices + 1);
        const int nextRingStartIndex = (_rings) *(_slices + 1);
        for(int j = 0; j < _slices; ++j)
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

void Sphere::render()
{
    // Bind the vertex array oobject to set up our vertex buffers and index buffer
    _vao.bind();

    // Draw it!
    //glDrawElements(GL_TRIANGLES, indexCount(), GL_UNSIGNED_INT, 0);

    _vao.release();
}

void Sphere::computeNormalLinesBuffer(const MaterialPtr& mat, double scale)
{

    int nVerts  = (_slices + 1) *(_rings + 1); // One extra line of latitude
    float* v = new float[6 * nVerts];
    float* vPtr = v;

    _positionBuffer.bind();
    float* p = reinterpret_cast<float*>(_positionBuffer.map(QOpenGLBuffer::ReadOnly));

    _normalBuffer.bind();
    float* n = reinterpret_cast<float*>(_normalBuffer.map(QOpenGLBuffer::ReadOnly));

    Q_ASSERT(n);
    Q_ASSERT(p);
    for(int vIndex = 0; vIndex < nVerts; ++vIndex)
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

    _normalBuffer.unmap();
    _positionBuffer.bind();
    _positionBuffer.unmap();

    _normalLines.create();
    _normalLines.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _normalLines.bind();
    _normalLines.allocate(v, 6 * nVerts * sizeof(float));

    _vaoNormals.create();
    _vaoNormals.bind();
    mat->shader()->enableAttributeArray("vertexPosition");
    mat->shader()->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);
    _vaoNormals.release();
    _normalLines.release();

    delete[] v;
}

void Sphere::computeTangentLinesBuffer(const MaterialPtr& mat, double scale)
{
    int nVerts  = (_slices + 1) *(_rings + 1); // One extra line of latitude
    float* v = new float[6 * nVerts];
    float* vPtr = v;

    _tangentBuffer.bind();
    float* t = reinterpret_cast<float*>(_tangentBuffer.map(QOpenGLBuffer::ReadOnly));

    _positionBuffer.bind();
    float* p = reinterpret_cast<float*>(_positionBuffer.map(QOpenGLBuffer::ReadOnly));

    Q_ASSERT(t);
    Q_ASSERT(p);
    for(int vIndex = 0; vIndex < nVerts; ++vIndex)
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

    _positionBuffer.unmap();
    _tangentBuffer.bind();
    _tangentBuffer.unmap();

    _tangentLines.create();
    _tangentLines.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _tangentLines.bind();
    _tangentLines.allocate(v, 6 * nVerts * sizeof(float));

    _vaoTangents.create();
    _vaoTangents.bind();
    mat->shader()->enableAttributeArray("vertexPosition");
    mat->shader()->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);
    _vaoTangents.release();
    _tangentLines.release();

    delete[] v;
}


void Sphere::renderNormalLines()
{
    //int nVerts  = (_slices + 1) *(_rings + 1); // One extra line of latitude

    _vaoNormals.bind();
    //glDrawArrays(GL_LINES, 0, nVerts * 2);
    _vaoNormals.release();
}

void Sphere::renderTangentLines()
{
    //int nVerts  = (_slices + 1) *(_rings + 1); // One extra line of latitude

    _vaoTangents.bind();
    //glDrawArrays(GL_LINES, 0, nVerts * 2);
    _vaoTangents.release();
}

void Sphere::bindBuffers()
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
