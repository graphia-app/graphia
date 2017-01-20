#include "cylinder.h"

#include "maths/constants.h"

#include <cmath>
#include <QOpenGLShaderProgram>

namespace Primitive
{

Cylinder::Cylinder(QObject* parent) :
    QObject(parent),
    _positionBuffer(QOpenGLBuffer::VertexBuffer),
    _normalBuffer(QOpenGLBuffer::VertexBuffer),
    _textureCoordBuffer(QOpenGLBuffer::VertexBuffer),
    _indexBuffer(QOpenGLBuffer::IndexBuffer),
    _vao()
{
}

void Cylinder::create(QOpenGLShaderProgram& shader)
{
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<float> tangents;
    std::vector<unsigned int> indices;

    generateVertexData(vertices, normals, texCoords, tangents, indices);

    _positionBuffer.create();
    _positionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _positionBuffer.bind();
    _positionBuffer.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(float));

    _normalBuffer.create();
    _normalBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _normalBuffer.bind();
    _normalBuffer.allocate(normals.data(), static_cast<int>(normals.size()) * sizeof(float));

    _textureCoordBuffer.create();
    _textureCoordBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _textureCoordBuffer.bind();
    _textureCoordBuffer.allocate(texCoords.data(), static_cast<int>(texCoords.size()) * sizeof(float));

    _tangentBuffer.create();
    _tangentBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _tangentBuffer.bind();
    _tangentBuffer.allocate(tangents.data(), static_cast<int>(tangents.size()) * sizeof(float));

    _indexBuffer.create();
    _indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _indexBuffer.bind();
    _indexBuffer.allocate(indices.data(), static_cast<int>(indices.size()) * sizeof(unsigned int));

    if(!_vao.isCreated())
        _vao.create();

    _vao.bind();

    shader.bind();

    _positionBuffer.bind();
    shader.enableAttributeArray("vertexPosition");
    shader.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    _normalBuffer.bind();
    shader.enableAttributeArray("vertexNormal");
    shader.setAttributeBuffer("vertexNormal", GL_FLOAT, 0, 3);

    _textureCoordBuffer.bind();
    shader.enableAttributeArray("vertexTexCoord");
    shader.setAttributeBuffer("vertexTexCoord", GL_FLOAT, 0, 2);

    _tangentBuffer.bind();
    shader.enableAttributeArray("vertexTangent");
    shader.setAttributeBuffer("vertexTangent", GL_FLOAT, 0, 4);

    _indexBuffer.bind();

    _vao.release();

    _indexBuffer.release();
    _tangentBuffer.release();
    _textureCoordBuffer.release();
    _normalBuffer.release();
    _positionBuffer.release();

    shader.release();
}

void Cylinder::generateVertexData(std::vector<float>& vertices, std::vector<float>& normals,
                                  std::vector<float>& texCoords, std::vector<float>& tangents,
                                  std::vector<unsigned int>& indices)
{
    int faces = (_slices);
    int numVerts  = ((_slices + 1) * 4) + 2;

    vertices.resize(3 * numVerts);
    normals.resize(3 * numVerts);
    tangents.resize(4 * numVerts);
    texCoords.resize(2 * numVerts);
    indices.resize(12 * faces);

    const float dTheta = Constants::TwoPi() / static_cast<float>(_slices);
    const float du = 1.0f / static_cast<float>(_slices);

    int index = 0, texCoordIndex = 0, tangentIndex = 0;

    QVector3D coneTip(0.0, _length * 0.5f, 0.0);

    // Iterate over longitudes (slices)
    for(int slice = 0; slice < _slices + 1; slice++)
    {
        const float theta = static_cast<float>(slice) * dTheta;
        const float cosTheta = std::cos(theta);
        const float sinTheta = std::sin(theta);
        const float u = static_cast<float>(slice) * du;

        vertices[index+0] = _radius * cosTheta;
        vertices[index+1] = _length * 0.25f;
        vertices[index+2] = _radius * sinTheta;

        normals[index+0] = cosTheta;
        normals[index+1] = 0.0f;
        normals[index+2] = sinTheta;

        index += 3;

        tangents[tangentIndex] = sinTheta;
        tangents[tangentIndex + 1] = 0.0;
        tangents[tangentIndex + 2] = -cosTheta;
        tangents[tangentIndex + 3] = 1.0;
        tangentIndex += 4;

        texCoords[texCoordIndex] = u;
        texCoords[texCoordIndex+1] = 0.0f;

        texCoordIndex += 2;

        vertices[index+0] = _radius * cosTheta;
        vertices[index+1] = _length * -0.5f;
        vertices[index+2] = _radius * sinTheta;

        normals[index+0] = cosTheta;
        normals[index+1] = 0.0f;
        normals[index+2] = sinTheta;

        index += 3;

        tangents[tangentIndex] = sinTheta;
        tangents[tangentIndex + 1] = 0.0;
        tangents[tangentIndex + 2] = -cosTheta;
        tangents[tangentIndex + 3] = 1.0;
        tangentIndex += 4;

        texCoords[texCoordIndex] = u;
        texCoords[texCoordIndex+1] = 1.0f;

        texCoordIndex += 2;

        // Make a cone for "arrow" mesh
        // This is closely coupled to the instancedEdges Shader
        // Bottom rim of cone (circle)
        vertices[index+0] = (_radius * 2) * cosTheta;
        vertices[index+1] = _length * 0.25f;
        vertices[index+2] = (_radius * 2) * sinTheta;
        QVector3D bottom(vertices[index+0],
                         vertices[index+1],
                         vertices[index+2]);

        // Base to tip
        QVector3D baseToTipTangent = coneTip - bottom;
        bottom.setY(0);

        QVector3D baseTangent = QVector3D::crossProduct(baseToTipTangent, bottom);
        baseTangent.normalize();
        QVector3D baseNormal = QVector3D::crossProduct(baseTangent, baseToTipTangent);
        baseNormal.normalize();

        // Cone surface normals
        normals[index+0] = baseNormal.x();
        normals[index+1] = baseNormal.y();
        normals[index+2] = baseNormal.z();

        index += 3;

        tangents[tangentIndex] = baseTangent.x();
        tangents[tangentIndex + 1] = baseTangent.y();
        tangents[tangentIndex + 2] = baseTangent.z();
        tangents[tangentIndex + 3] = 1.0;
        tangentIndex += 4;

        texCoords[texCoordIndex] = u;
        texCoords[texCoordIndex+1] = 0.0f;

        texCoordIndex += 2;

        // Bottom rim of cone (for cap only)
        vertices[index+0] = (_radius * 2) * cosTheta;
        vertices[index+1] = _length * 0.25f;
        vertices[index+2] = (_radius * 2) * sinTheta;

        normals[index+0] = 0;
        normals[index+1] = -1.0f;
        normals[index+2] = 0;

        index += 3;

        tangents[tangentIndex] = sinTheta;
        tangents[tangentIndex + 1] = 0.0;
        tangents[tangentIndex + 2] = -cosTheta;
        tangents[tangentIndex + 3] = 1.0;
        tangentIndex += 4;

        texCoords[texCoordIndex] = u;
        texCoords[texCoordIndex+1] = 1.0f;

        texCoordIndex += 2;
    }

    // Bottom of cone - top of edge (tail)
    int capIndex = index / 3;
    vertices[index+0] = 0.0;
    vertices[index+1] = _length * 0.25f;
    vertices[index+2] = 0.0;

    normals[index+0] = 0.0;
    normals[index+1] = -1.0;
    normals[index+2] = 0.0;

    tangents[tangentIndex] = 1.0;
    tangents[tangentIndex + 1] = 0.0;
    tangents[tangentIndex + 2] = 0.0;
    tangents[tangentIndex + 3] = 1.0;

    texCoords[texCoordIndex] = 0.0f;
    texCoords[texCoordIndex+1] = 0.0f;

    index += 3;

    int tipIndex = index / 3;
    vertices[index+0] = coneTip.x();
    vertices[index+1] = coneTip.y();
    vertices[index+2] = coneTip.z();

    // Tip normals (Clever hack. All 0's, allows blending with cone base normals
    // which is then normalised to create a smoothly blended normal)
    normals[index+0] = 0.0;
    normals[index+1] = 0.0;
    normals[index+2] = 0.0;

    tangents[tangentIndex] = 0.0;
    tangents[tangentIndex + 1] = 0.0;
    tangents[tangentIndex + 2] = 0.0;
    tangents[tangentIndex + 3] = 1.0;

    texCoords[texCoordIndex] = 0.0f;
    texCoords[texCoordIndex+1] = 0.0f;

    index = 0;

    for(int slice = 0; slice < _slices; slice++)
    {
        int baseIndex = slice * 4;

        // Tail (edge) indices
        indices[index+0] = baseIndex + 5;
        indices[index+1] = baseIndex + 1;
        indices[index+2] = baseIndex + 0;

        indices[index+3] = baseIndex + 0;
        indices[index+4] = baseIndex + 4;
        indices[index+5] = baseIndex + 5;

        // Cone Indices
        indices[index+6] = baseIndex + 2;
        indices[index+7] = tipIndex;
        indices[index+8] = baseIndex + 6;

        // Cap
        indices[index+9] = baseIndex + 7;
        indices[index+10] = capIndex;
        indices[index+11] = baseIndex + 3;

        index += 12;
    }
}
} // namespace Primitive
