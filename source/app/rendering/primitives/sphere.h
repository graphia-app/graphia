#ifndef SPHERE_H
#define SPHERE_H

#include <QObject>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <vector>
namespace Primitive
{
class Sphere : public QObject
{
    Q_OBJECT

    Q_PROPERTY(float radius READ radius WRITE setRadius)
    Q_PROPERTY(int rings READ rings WRITE setRings)
    Q_PROPERTY(int slices READ slices WRITE setSlices)

public:
    explicit Sphere(QObject* parent = nullptr);

    float radius() const { return _radius; }
    int rings() const { return _rings; }
    int slices() const { return _slices; }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }

    int indexCount() const { return 6 * _slices * _rings; }

public slots:
    void setRadius(float radius) { _radius = radius; }
    void setRings(int rings) { _rings = rings; }
    void setSlices(int slices) { _slices = slices; }

    void create(QOpenGLShaderProgram& shader);

private:
    void generateVertexData(std::vector<float>& vertices, std::vector<float>& normals,
                            std::vector<float>& texCoords, std::vector<float>& tangents,
                            std::vector<unsigned int>& indices);

    float _radius = 1.0f;
    size_t _rings = 30;
    size_t _slices = 30;

    QOpenGLBuffer _positionBuffer;
    QOpenGLBuffer _normalBuffer;
    QOpenGLBuffer _textureCoordBuffer;
    QOpenGLBuffer _indexBuffer;
    QOpenGLBuffer _tangentBuffer;

    QOpenGLVertexArrayObject _vao;
};
}

#endif // SPHERE_H
