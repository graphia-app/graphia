#ifndef CUBE_H
#define CUBE_H

#include <QObject>

#include "material.h"

#include <QOpenGLBuffer>
#include "qopenglvertexarrayobject.h"

class QOpenGLShaderProgram;

class Cube : public QObject
{
    Q_OBJECT
    Q_PROPERTY( float length READ length WRITE setLength )

public:
    explicit Cube( QObject* parent = 0 );

    void setMaterial( const MaterialPtr& material );
    MaterialPtr material() const;

    float length() const
    {
        return m_length;
    }

    QOpenGLVertexArrayObject* vertexArrayObject() { return &m_vao; }

    void bindBuffers();

    int indexCount() const { return 36; }

public slots:
    void setLength( float arg )
    {
        m_length = arg;
    }

    void create();

    void render();

private:
    void generateVertexData(float* vertices, float* normals, float* texCoords, float* tangents, unsigned int* indices );
    void updateVertexArrayObject();

    MaterialPtr m_material;
    float m_length;

    // QVertexBuffers to hold the data
    QOpenGLBuffer m_positionBuffer;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer m_textureCoordBuffer;
    QOpenGLBuffer m_tangentBuffer;
    QOpenGLBuffer m_indexBuffer;

    QOpenGLVertexArrayObject m_vao;
};

#endif // CUBE_H
