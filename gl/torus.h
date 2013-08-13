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
    Q_PROPERTY( float minorRadius READ minorRadius WRITE setMinorRadius )
    Q_PROPERTY( float majorRadius READ majorRadius WRITE setMajorRadius )
    Q_PROPERTY( int rings READ rings WRITE setRings )
    Q_PROPERTY( int sides READ sides WRITE setSides )

public:
    explicit Torus( QObject* parent = 0 );

    void setMaterial( const MaterialPtr& material );
    MaterialPtr material() const;

    float minorRadius() const
    {
        return m_minorRadius;
    }

    float majorRadius() const
    {
        return m_majorRadius;
    }

    int rings() const
    {
        return m_rings;
    }

    int sides() const
    {
        return m_sides;
    }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &m_vao; }

    int indexCount() const { return 6 * m_sides * m_rings; }

public slots:
    void setMinorRadius( float arg )
    {
        m_minorRadius = arg;
    }

    void setMajorRadius( float arg )
    {
        m_majorRadius = arg;
    }

    void setRings( int arg )
    {
        m_rings = arg;
    }

    void setSides( int arg )
    {
        m_sides = arg;
    }

    void create();
    void render();

private:
    void generateVertexData( float* vertices, float* normals,float* texCoords, unsigned int* indices );
    void updateVertexArrayObject();

    MaterialPtr m_material;

    float m_minorRadius;
    float m_majorRadius;
    int m_rings;
    int m_sides;

    // QVertexBuffers to hold the data
    QOpenGLBuffer m_positionBuffer;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer m_textureCoordBuffer;
    QOpenGLBuffer m_indexBuffer;

    QOpenGLVertexArrayObject m_vao;
};

#endif // TORUS_H
