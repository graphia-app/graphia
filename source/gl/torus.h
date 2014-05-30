#ifndef TORUS_H
#define TORUS_H

#include <QObject>

#include "material.h"

#include <QOpenGLBuffer>
#include "qopenglvertexarrayobject.h"

class QOpenGLShaderProgram;

class Torus : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float minorRadius READ minorRadius WRITE setMinorRadius)
    Q_PROPERTY(float majorRadius READ majorRadius WRITE setMajorRadius)
    Q_PROPERTY(int rings READ rings WRITE setRings)
    Q_PROPERTY(int sides READ sides WRITE setSides)

public:
    explicit Torus(QObject* parent = 0);

    void setMaterial(const MaterialPtr& material);
    MaterialPtr material() const;

    float minorRadius() const
    {
        return _minorRadius;
    }

    float majorRadius() const
    {
        return _majorRadius;
    }

    int rings() const
    {
        return _rings;
    }

    int sides() const
    {
        return _sides;
    }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }

    int indexCount() const { return 6 * _sides * _rings; }

public slots:
    void setMinorRadius(float arg)
    {
        _minorRadius = arg;
    }

    void setMajorRadius(float arg)
    {
        _majorRadius = arg;
    }

    void setRings(int arg)
    {
        _rings = arg;
    }

    void setSides(int arg)
    {
        _sides = arg;
    }

    void create();
    void render();

private:
    void generateVertexData(float* vertices, float* normals,float* texCoords, unsigned int* indices);
    void updateVertexArrayObject();

    MaterialPtr _material;

    float _minorRadius;
    float _majorRadius;
    int _rings;
    int _sides;

    // QVertexBuffers to hold the data
    QOpenGLBuffer _positionBuffer;
    QOpenGLBuffer _normalBuffer;
    QOpenGLBuffer _textureCoordBuffer;
    QOpenGLBuffer _indexBuffer;

    QOpenGLVertexArrayObject _vao;
};

#endif // TORUS_H
