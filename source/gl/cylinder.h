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
        return m_radius;
    }

    float length() const
    {
        return m_length;
    }

    int slices() const
    {
        return m_slices;
    }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &m_vao; }

    int indexCount() const { return 6 * m_slices; }

    void bindBuffers();

public slots:
    void setRadius( float arg )
    {
        m_radius = arg;
    }

    void setLength( float arg )
    {
        m_length = arg;
    }

    void setSlices( int arg )
    {
        m_slices = arg;
    }

    void create();

private:
    void generateVertexData( QVector<float>& vertices, QVector<float>& normals,
                             QVector<float>& texCoords, QVector<float>& tangents,
                             QVector<unsigned int>& indices );
    void updateVertexArrayObject();

    MaterialPtr m_material;

    float m_radius;
    float m_length;
    int m_slices; // Longitude

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

#endif // CYLINDER_H
