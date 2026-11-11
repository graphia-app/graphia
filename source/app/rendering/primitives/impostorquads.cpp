/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "impostorquads.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <vector>

namespace Primitive
{
ImpostorQuads::ImpostorQuads(int numQuads) :
    _numQuads(numQuads),
    _positionBuffer(QOpenGLBuffer::VertexBuffer),
    _indexBuffer(QOpenGLBuffer::IndexBuffer)
{}

void ImpostorQuads::create(QOpenGLShaderProgram& shader)
{
    const std::array<float, 8> corners{{-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f}};
    const std::array<unsigned int, 6> cornerIndices{{0, 1, 2, 2, 3, 0}};

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for(int quad = 0; quad < _numQuads; quad++)
    {
        const auto firstVertex = static_cast<unsigned int>(vertices.size() / 3);

        for(size_t corner = 0; corner < corners.size(); corner += 2)
            vertices.insert(vertices.end(), {corners.at(corner), corners.at(corner + 1), static_cast<float>(quad)});

        std::ranges::transform(cornerIndices, std::back_inserter(indices),
            [firstVertex](auto index) { return firstVertex + index; });
    }

    _positionBuffer.create();
    _positionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _positionBuffer.bind();
    _positionBuffer.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

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

    _indexBuffer.bind();

    _vao.release();

    _indexBuffer.release();
    _positionBuffer.release();

    shader.release();
}
} // namespace Primitive
