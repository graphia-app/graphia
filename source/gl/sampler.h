#ifndef SAMPLER_H
#define SAMPLER_H

#include <qopengl.h>
#include <QSharedPointer>

class QOpenGLFunctions_3_3_Core;

class SamplerPrivate;

class Sampler
{
public:
    Sampler();
    ~Sampler();

    bool create();
    void destroy();
    bool isCreated() const;
    GLuint samplerId() const;
    void bind( GLuint unit );
    void release( GLuint unit );

    enum CoordinateDirection
    {
        DirectionS = GL_TEXTURE_WRAP_S,
        DirectionT = GL_TEXTURE_WRAP_T,
        DirectionR = GL_TEXTURE_WRAP_R
    };

    void setWrapMode( CoordinateDirection direction, GLenum wrapMode );
    void setMinificationFilter( GLenum filter );
    void setMagnificationFilter( GLenum filter );
    void setMaximumAnisotropy( float anisotropy );

private:
    Q_DECLARE_PRIVATE( Sampler )
    SamplerPrivate* d_ptr;
};

typedef QSharedPointer<Sampler> SamplerPtr;

#endif // SAMPLER_H
