#ifndef CYLINDER_H
#define CYLINDER_H

#include <QObject>

#include "material.h"
#include "qopenglvertexarrayobject.h"

#include <QOpenGLBuffer>

class Cylinder : public QObject
{
    Q_OBJECT

    Q_PROPERTY( float radius READ radius WRITE setRadius )
    Q_PROPERTY( float length READ length WRITE setLength )
    Q_PROPERTY( int slices READ slices WRITE setSlices )

public:
    explicit Cylinder( QObject* parent = 0 );

    void setMaterial( const MaterialPtr& material );
    MaterialPtr material() const;

    float radius() const
    {
        return _radius;
    }

    float length() const
    {
        return _length;
    }

    int slices() const
    {
        return _slices;
    }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }

    int indexCount() const { return 6 * _slices; }

    void bindBuffers();

public slots:
    void setRadius( float arg )
    {
        _radius = arg;
    }

    void setLength( float arg )
    {
        _length = arg;
    }

    void setSlices( int arg )
    {
        _slices = arg;
    }

    void create();

private:
    void generateVertexData( QVector<float>& vertices, QVector<float>& normals,
                             QVector<float>& texCoords, QVector<float>& tangents,
                             QVector<unsigned int>& indices );
    void updateVertexArrayObject();

    MaterialPtr _material;

    float _radius;
    float _length;
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

#endif // CYLINDER_H
