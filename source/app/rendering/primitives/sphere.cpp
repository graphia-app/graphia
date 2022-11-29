/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sphere.h"

#include <cmath>
#include <numbers>

#include <QOpenGLShaderProgram>

namespace Primitive
{
// NOLINTNEXTLINE modernize-use-equals-default
Sphere::Sphere() :
    _positionBuffer(QOpenGLBuffer::VertexBuffer),
    _normalBuffer(QOpenGLBuffer::VertexBuffer),
    _textureCoordBuffer(QOpenGLBuffer::VertexBuffer),
    _indexBuffer(QOpenGLBuffer::IndexBuffer)
{
}

void Sphere::create(QOpenGLShaderProgram& shader)
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
    _positionBuffer.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    _normalBuffer.create();
    _normalBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _normalBuffer.bind();
    _normalBuffer.allocate(normals.data(), static_cast<int>(normals.size() * sizeof(float)));

    _textureCoordBuffer.create();
    _textureCoordBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _textureCoordBuffer.bind();
    _textureCoordBuffer.allocate(texCoords.data(), static_cast<int>(texCoords.size() * sizeof(float)));

    _tangentBuffer.create();
    _tangentBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _tangentBuffer.bind();
    _tangentBuffer.allocate(tangents.data(), static_cast<int>(tangents.size() * sizeof(float)));

    _indexBuffer.create();
    _indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _indexBuffer.bind();
    _indexBuffer.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(unsigned int)));

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

void Sphere::generateVertexData(std::vector<float>& vertices, std::vector<float>& normals,
                                std::vector<float>& texCoords, std::vector<float>& tangents,
                                std::vector<unsigned int>& indices) const
{
    auto faces = (_slices - 2) * _rings + // Number of "rectangular" faces
            (_rings * 2); // and one ring for the top and bottom caps
    auto numVerts  = (_slices + 1) *(_rings + 1); // One extra line of latitude

    // Resize vector to hold our data
    vertices.resize(3 * numVerts);
    normals.resize(3 * numVerts);
    tangents.resize(4 * numVerts);
    texCoords.resize(2 * numVerts);
    indices.resize(6 * faces);

    const float dTheta = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(_slices);
    const float dPhi = std::numbers::pi_v<float> / static_cast<float>(_rings);
    const float du = 1.0f / static_cast<float>(_slices);
    const float dv = 1.0f / static_cast<float>(_rings);

    // Iterate over latitudes (rings)
    size_t index = 0, texCoordIndex = 0, tangentIndex = 0;
    for(size_t lat = 0U; lat < _rings + 1; ++lat)
    {
        const float phi = std::numbers::pi_v<float> / 2.0f - static_cast<float>(lat) * dPhi;
        const float cosPhi = std::cos(phi);
        const float sinPhi = std::sin(phi);
        const float v = 1.0f - static_cast<float>(lat) * dv;

        // Iterate over longitudes (slices)
        for(size_t lon = 0U; lon < _slices + 1; ++lon)
        {
            const float theta = static_cast<float>(lon) * dTheta;
            const float cosTheta = std::cos(theta);
            const float sinTheta = std::sin(theta);
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

    index = 0;

    // top cap
    {
        const auto nextRingStartIndex = _slices + 1;
        for(size_t j = 0U; j < _slices; ++j)
        {
            indices[index] = static_cast<unsigned int>(nextRingStartIndex + j);
            indices[index+1] = 0;
            indices[index+2] = static_cast<unsigned int>(nextRingStartIndex + j + 1);
            index += 3;
        }
    }

    for(size_t i = 1U; i < (_rings - 1); ++i)
    {
        const auto ringStartIndex = i *(_slices + 1);
        const auto nextRingStartIndex = (i + 1) *(_slices + 1);

        for(size_t j = 0U; j < _slices; ++j)
        {
            // Split the quad into two triangles
            indices[index]   = static_cast<unsigned int>(ringStartIndex + j);
            indices[index+1] = static_cast<unsigned int>(ringStartIndex + j + 1);
            indices[index+2] = static_cast<unsigned int>(nextRingStartIndex + j);
            indices[index+3] = static_cast<unsigned int>(nextRingStartIndex + j);
            indices[index+4] = static_cast<unsigned int>(ringStartIndex + j + 1);
            indices[index+5] = static_cast<unsigned int>(nextRingStartIndex + j + 1);

            index += 6;
        }
    }

    // bottom cap
    {
        const auto ringStartIndex = (_rings - 1) *(_slices + 1);
        const auto nextRingStartIndex = (_rings) *(_slices + 1);
        for(size_t j = 0U; j < _slices; ++j)
        {
            indices[index] = static_cast<unsigned int>(ringStartIndex + j + 1);
            indices[index+1] = static_cast<unsigned int>(nextRingStartIndex);
            indices[index+2] = static_cast<unsigned int>(ringStartIndex + j);
            index += 3;
        }
    }
}

} // namespace Primitive
