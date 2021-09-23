/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "rectangle.h"

#include <array>

namespace Primitive
{
// NOLINTNEXTLINE modernize-use-equals-default
Rectangle::Rectangle() :
    _positionBuffer(QOpenGLBuffer::VertexBuffer),
    _normalBuffer(QOpenGLBuffer::VertexBuffer),
    _textureCoordBuffer(QOpenGLBuffer::VertexBuffer),
    _indexBuffer(QOpenGLBuffer::IndexBuffer)
{
}

static void generateVertexData(std::vector<float>& vertices, std::vector<float>& normals,
    std::vector<float>& texCoords, std::vector<float>& tangents, std::vector<unsigned int>& indices)
{
    vertices.resize(3ul * 4ul);
    normals.resize(3ul * 4ul);
    tangents.resize(4ul * 4ul);
    texCoords.resize(2ul * 4ul);
    indices.resize(3ul * 2ul);

    std::array<float, 12> verts{{
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
    }};

    std::array<float, 3> norm{{
        0.0f, 0.0f, 1.0f
    }};

    std::array<float, 4> tangs{{
        1.0f, 0.0f, 0.0f, 1.0f
    }};

    // Copy Verts
    vertices.assign(std::begin(verts), std::end(verts));
    for(size_t i = 0; i < 4; ++i)
    {
        auto index = i*3;
        vertices[index] = verts.at(index);
        vertices[index+1] = verts.at(index+1);
        vertices[index+2] = verts.at(index+2);

        normals[index] = norm.at(0);
        normals[index+1] = norm.at(1);
        normals[index+2] = norm.at(2);

        auto tindex = i*4;
        tangents[tindex] = tangs.at(0);
        tangents[tindex+1] = tangs.at(1);
        tangents[tindex+2] = tangs.at(2);
        tangents[tindex+3] = tangs.at(3);

        auto texIndex = i*2;
        texCoords[texIndex] = verts.at(i*3);
        texCoords[texIndex+1] = verts.at((i*3)+1);
    }

    // Build two triangles
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    indices[3] = 2;
    indices[4] = 3;
    indices[5] = 0;
}

void Rectangle::create(QOpenGLShaderProgram &shader)
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

} // namespace Primitive
