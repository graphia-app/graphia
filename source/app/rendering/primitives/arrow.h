/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#ifndef ARROW_H
#define ARROW_H

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <vector>
namespace Primitive
{
// Consists of a cylinder and a cone. Edge shader hides the cone based on
// edge type.
class Arrow
{
public:
    Arrow();

    float radius() const { return _radius; }
    auto length() const { return _length; }
    auto slices() const { return _slices; }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }

    auto glIndexCount() const { return static_cast<GLsizei>(12 * _slices); }

public slots:
    void setRadius(float radius) { _radius = radius; }
    void setLength(float length) { _length = length; }
    void setSlices(size_t slices) { _slices = slices; }

    void create(QOpenGLShaderProgram& shader);

private:
    void generateVertexData(std::vector<float>& vertices, std::vector<float>& normals,
                            std::vector<float>& texCoords, std::vector<float>& tangents,
                            std::vector<unsigned int>& indices) const;

    float _radius = 1.0f;
    float _length = 1.0f;
    size_t _slices = 30;

    QOpenGLBuffer _positionBuffer;
    QOpenGLBuffer _normalBuffer;
    QOpenGLBuffer _textureCoordBuffer;
    QOpenGLBuffer _indexBuffer;
    QOpenGLBuffer _tangentBuffer;

    QOpenGLVertexArrayObject _vao;
};
} // namespace Primitive

#endif // ARROW_H
