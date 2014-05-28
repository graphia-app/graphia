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
    explicit Quad( QObject* parent = 0 );

    void setMaterial( const MaterialPtr& material );
    MaterialPtr material() const;

    QOpenGLVertexArrayObject* vertexArrayObject() { return &m_vao; }

    int indexCount() const { return 6; }

    void bindBuffers();

    float edgeLength()
    {
        return m_edgeLength;
    }

public slots:
    void create();

    void setEdgeLength(float edgeLength)
    {
        m_edgeLength = edgeLength;
    }

private:
    void generateVertexData( QVector<float>& vertices, QVector<float>& normals,
                             QVector<float>& texCoords, QVector<float>& tangents,
                             QVector<unsigned int>& indices );
    void updateVertexArrayObject();

    MaterialPtr m_material;

    float m_edgeLength;

    // QVertexBuffers to hold the data
    QOpenGLBuffer m_positionBuffer;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer m_textureCoordBuffer;
    QOpenGLBuffer m_indexBuffer;
    QOpenGLBuffer m_tangentBuffer;

    QOpenGLVertexArrayObject m_vao;

    QOpenGLBuffer m_normalLines, m_tangentLines;
    QOpenGLVertexArrayObject m_vaoNormals, m_vaoTangents;
};

#endif // UNITQUAD_H
