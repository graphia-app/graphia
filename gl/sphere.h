#ifndef SPHERE_H
#define SPHERE_H

#include <QObject>

#include "material.h"
#include "qopenglvertexarrayobject.h"

#include <QOpenGLBuffer>

class Sphere : public QObject
{
    Q_OBJECT

    Q_PROPERTY( float radius READ radius WRITE setRadius )
    Q_PROPERTY( int rings READ rings WRITE setRings )
    Q_PROPERTY( int slices READ slices WRITE setSlices )

public:
    explicit Sphere( QObject* parent = 0 );

    void setMaterial( const MaterialPtr& material );
    MaterialPtr material() const;

    float radius() const
    {
        return m_radius;
    }

    int rings() const
    {
        return m_rings;
    }

    int slices() const
    {
        return m_slices;
    }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &m_vao; }

    int indexCount() const { return 6 * m_slices * m_rings; }

    /**
     * @brief computeNormalLinesBuffer - compute a vertex buffer suitable for
     * rendering with GL_LINES, showing each vertex normal
     */
    void computeNormalLinesBuffer( const MaterialPtr& mat, double scale = 1.0 );
    void computeTangentLinesBuffer( const MaterialPtr& mat, double scale = 1.0 );

    void renderNormalLines();
    void renderTangentLines();

    void bindBuffers();

public slots:
    void setRadius( float arg )
    {
        m_radius = arg;
    }

    void setRings( int arg )
    {
        m_rings = arg;
    }

    void setSlices( int arg )
    {
        m_slices = arg;
    }

    void create();
    void render();

private:
    void generateVertexData( QVector<float>& vertices, QVector<float>& normals,
                             QVector<float>& texCoords, QVector<float>& tangents,
                             QVector<unsigned int>& indices );
    void updateVertexArrayObject();

    MaterialPtr m_material;

    float m_radius;
    int m_rings;  // Rings of latitude
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

#endif // SPHERE_H
