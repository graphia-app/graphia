#ifndef CYLINDER_H
#define CYLINDER_H

#include <QObject>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <vector>

class Cylinder : public QObject
{
    Q_OBJECT

    Q_PROPERTY(float radius READ radius WRITE setRadius)
    Q_PROPERTY(float length READ length WRITE setLength)
    Q_PROPERTY(int slices READ slices WRITE setSlices)

public:
    explicit Cylinder(QObject* parent = 0);

    float radius() const { return _radius; }
    int length() const { return _length; }
    int slices() const { return _slices; }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }

    int indexCount() const { return 6 * _slices; }

public slots:
    void setRadius(float radius) { _radius = radius; }
    void setLength(int length) { _length = length; }
    void setSlices(int slices) { _slices = slices; }

    void create(QOpenGLShaderProgram& shader);

private:
    void generateVertexData(std::vector<float>& vertices, std::vector<float>& normals,
                            std::vector<float>& texCoords, std::vector<float>&tangents,
                            std::vector<unsigned int>& indices);

    float _radius;
    float _length;
    int _slices;

    QOpenGLBuffer _positionBuffer;
    QOpenGLBuffer _normalBuffer;
    QOpenGLBuffer _textureCoordBuffer;
    QOpenGLBuffer _indexBuffer;
    QOpenGLBuffer _tangentBuffer;

    QOpenGLVertexArrayObject _vao;
};

#endif // CYLINDER_H
