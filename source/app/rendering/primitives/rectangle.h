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
