#ifndef UNITQUAD_H
#define UNITQUAD_H

#include <QObject>

#include "material.h"
#include "qopenglvertexarrayobject.h"

#include <QOpenGLBuffer>

class Quad : public QObject
{
    Q_OBJECT

public:
    explicit Quad(QObject* parent = 0);

    void setMaterial(const MaterialPtr& material);
    MaterialPtr material() const;

    QOpenGLVertexArrayObject* vertexArrayObject() { return &_vao; }

    int indexCount() const { return 6; }

    void bindBuffers();

    float edgeLength()
    {
        return _edgeLength;
    }

public slots:
    void create();

    void setEdgeLength(float edgeLength)
    {
        _edgeLength = edgeLength;
    }

private:
    void generateVertexData(QVector<float>& vertices, QVector<float>& normals,
                             QVector<float>& texCoords, QVector<float>& tangents,
                             QVector<unsigned int>& indices);
    void updateVertexArrayObject();

    MaterialPtr _material;

    float _edgeLength;

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

#endif // UNITQUAD_H
