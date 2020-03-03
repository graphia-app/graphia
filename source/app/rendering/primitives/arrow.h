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
    void setSlices(int slices) { _slices = slices; }

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
