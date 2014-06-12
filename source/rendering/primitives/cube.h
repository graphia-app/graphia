#ifndef CUBE_H
#define CUBE_H

#include <QObject>

#include "../material.h"

#include <QOpenGLBuffer>
#include "qopenglvertexarrayobject.h"

class QOpenGLShaderProgram;

class Cube : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float length READ length WRITE setLength)

public:
    explicit Cube(QObject* parent = 0);

    void setMaterial(const MaterialPtr& material);
    MaterialPtr material() const;

    float length() const
    {
        return _length;
    }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }

    void bindBuffers();

    int indexCount() const { return 36; }

public slots:
    void setLength(float arg)
    {
        _length = arg;
    }

    void create();

    void render();

private:
    void generateVertexData(float* vertices, float* normals, float* texCoords, float* tangents, unsigned int* indices);
    void updateVertexArrayObject();

    MaterialPtr _material;
    float _length;

    // QVertexBuffers to hold the data
    QOpenGLBuffer _positionBuffer;
    QOpenGLBuffer _normalBuffer;
    QOpenGLBuffer _textureCoordBuffer;
    QOpenGLBuffer _tangentBuffer;
    QOpenGLBuffer _indexBuffer;

    QOpenGLVertexArrayObject _vao;
};

#endif // CUBE_H
