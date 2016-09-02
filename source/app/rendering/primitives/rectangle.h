#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <QObject>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
namespace Primitive
{
class Rectangle  : public QObject
{
    Q_OBJECT
public:
    explicit Rectangle(QObject* parent = nullptr);

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }

    int indexCount() const { return 6; }

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
}

#endif // RECTANGLE_H
