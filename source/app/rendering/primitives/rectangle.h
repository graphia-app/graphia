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

    GLsizei glIndexCount() const { return 6; }

public slots:
    void create(QOpenGLShaderProgram& shader);
private:
    void generateVertexData(std::vector<float>& vertices, std::vector<float>& normals,
                            std::vector<float>& texCoords, std::vector<float>& tangents,
                            std::vector<unsigned int>& indices);

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
