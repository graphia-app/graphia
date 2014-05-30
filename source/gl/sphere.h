#ifndef SPHERE_H
#define SPHERE_H

#include <QObject>

#include "material.h"
#include "qopenglvertexarrayobject.h"

#include <QOpenGLBuffer>

class Sphere : public QObject
{
    Q_OBJECT

    Q_PROPERTY(float radius READ radius WRITE setRadius)
    Q_PROPERTY(int rings READ rings WRITE setRings)
    Q_PROPERTY(int slices READ slices WRITE setSlices)

public:
    explicit Sphere(QObject* parent = 0);

    void setMaterial(const MaterialPtr& material);
    MaterialPtr material() const;

    float radius() const
    {
        return _radius;
    }

    int rings() const
    {
        return _rings;
    }

    int slices() const
    {
        return _slices;
    }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }

    int indexCount() const { return 6 * _slices * _rings; }

    /**
     * @brief computeNormalLinesBuffer - compute a vertex buffer suitable for
     * rendering with GL_LINES, showing each vertex normal
     */
    void computeNormalLinesBuffer(const MaterialPtr& mat, double scale = 1.0);
    void computeTangentLinesBuffer(const MaterialPtr& mat, double scale = 1.0);

    void renderNormalLines();
    void renderTangentLines();

    void bindBuffers();

public slots:
    void setRadius(float arg)
    {
        _radius = arg;
    }

    void setRings(int arg)
    {
        _rings = arg;
    }

    void setSlices(int arg)
    {
        _slices = arg;
    }

    void create();
    void render();

private:
    void generateVertexData(QVector<float>& vertices, QVector<float>& normals,
                             QVector<float>& texCoords, QVector<float>& tangents,
                             QVector<unsigned int>& indices);
    void updateVertexArrayObject();

    MaterialPtr _material;

    float _radius;
    int _rings;  // Rings of latitude
    int _slices; // Longitude

    // QVertexBuffers to hold the data
    QOpenGLBuffer _positionBuffer;
    QOpenGLBuffer _normalBuffer;
    QOpenGLBuffer _textureCoordBuffer;
    QOpenGLBuffer _indexBuffer;
    QOpenGLBuffer _tangentBuffer;

    QOpenGLVertexArrayObject _vao;

    QOpenGLBuffer _normalLines, _tangentLines;
    QOpenGLVertexArrayObject _vaoNormals, _vaoTangents;
};

#endif // SPHERE_H
