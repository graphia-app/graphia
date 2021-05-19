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

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

namespace Primitive
{
class Rectangle
{
public:
    Rectangle();

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }
    static GLsizei glIndexCount() { return 6; }
    void create(QOpenGLShaderProgram& shader);

private:
    QOpenGLBuffer _positionBuffer;
    QOpenGLBuffer _normalBuffer;
    QOpenGLBuffer _textureCoordBuffer;
    QOpenGLBuffer _indexBuffer;
    QOpenGLBuffer _tangentBuffer;

    QOpenGLVertexArrayObject _vao;
};
} // namespace Primitive

#endif // RECTANGLE_H
