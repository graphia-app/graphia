#include "sampler.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLExtensions>

class SamplerPrivate
{
public:
    SamplerPrivate( Sampler* qq )
        : q_ptr( qq ),
#if !defined(QT_OPENGL_ES_2)
          m_samplerId( 0 )
#endif
    {
    }

    bool create( QOpenGLContext* ctx );
    void destroy();
    void bind( GLuint unit );
    void release( GLuint unit );
    void setParameter( GLenum param, GLenum value );
    void setParameter( GLenum param, float value );

    Q_DECLARE_PUBLIC( Sampler )

    Sampler* q_ptr;

#if !defined(QT_OPENGL_ES_2)
    GLuint m_samplerId;

    union {
        QOpenGLFunctions_3_3_Core* core;
        QOpenGLExtension_ARB_sampler_objects *arb;
    } m_funcs;

    enum {
        NotSupported,
        Core,
        ARB
    } m_funcsType;
#endif
};

bool SamplerPrivate::create( QOpenGLContext* ctx )
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
    return false;
#else
    m_funcs.core = 0;
    m_funcsType = NotSupported;
    QSurfaceFormat format = ctx->format();
    const int version = ( format.majorVersion() << 8 ) + format.minorVersion();

    if ( version >= 0x0303 )
    {
        m_funcs.core = ctx->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if ( !m_funcs.core )
            return false;
        m_funcsType = Core;
        m_funcs.core->initializeOpenGLFunctions();
        m_funcs.core->glGenSamplers( 1, &m_samplerId );
    }
    else if ( ctx->hasExtension( "GL_ARB_sampler_objects" ) )
    {
        m_funcs.arb = new QOpenGLExtension_ARB_sampler_objects;
        if (!m_funcs.arb->initializeOpenGLFunctions())
            return false;
        m_funcsType = ARB;
        m_funcs.arb->glGenSamplers( 1, &m_samplerId );
    }

    return ( m_samplerId != 0 );
#endif
}

void SamplerPrivate::destroy()
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    if ( m_samplerId )
    {
        switch ( m_funcsType )
        {
            case Core:
                m_funcs.core->glDeleteSamplers( 1, &m_samplerId );
                break;

            case ARB:
                m_funcs.arb->glDeleteSamplers( 1, &m_samplerId );
                break;

            default:
            case NotSupported:
                // Shut the compiler up
                qt_noop();
        }
        m_samplerId = 0;
    }
#endif
}

void SamplerPrivate::bind( GLuint unit )
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    switch ( m_funcsType )
    {
        case Core:
            m_funcs.core->glBindSampler( unit, m_samplerId );
            break;

        case ARB:
            m_funcs.arb->glBindSampler( unit, m_samplerId );
            break;

        default:
        case NotSupported:
            // Shut the compiler up
            qt_noop();
    }
#endif
}

void SamplerPrivate::release( GLuint unit )
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    switch ( m_funcsType )
    {
        case Core:
            m_funcs.core->glBindSampler( unit, 0 );
            break;

        case ARB:
            m_funcs.arb->glBindSampler( unit, 0 );
            break;

        default:
        case NotSupported:
            // Shut the compiler up
            qt_noop();
    }
#endif
}

void SamplerPrivate::setParameter( GLenum param, GLenum value )
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    switch ( m_funcsType )
    {
        case Core:
            m_funcs.core->glSamplerParameteri( m_samplerId, param, value );
            break;

        case ARB:
            m_funcs.arb->glSamplerParameteri( m_samplerId, param, value );
            break;

        default:
        case NotSupported:
            // Shut the compiler up
            qt_noop();
    }
#endif
}

void SamplerPrivate::setParameter( GLenum param, float value )
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    switch ( m_funcsType )
    {
        case Core:
            m_funcs.core->glSamplerParameterf( m_samplerId, param, value );
            break;

        case ARB:
            m_funcs.arb->glSamplerParameterf( m_samplerId, param, value );
            break;

        default:
        case NotSupported:
            // Shut the compiler up
            qt_noop();
    }
#endif
}

Sampler::Sampler()
    : d_ptr( new SamplerPrivate( this ) )
{
}

Sampler::~Sampler()
{
}

bool Sampler::create()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    Q_ASSERT( context );
    Q_D( Sampler );
    return d->create( context );
}

void Sampler::destroy()
{
    Q_D( Sampler );
    d->destroy();
}

bool Sampler::isCreated() const
{
    Q_D( const Sampler );
    return ( d->m_samplerId != 0 );
}

GLuint Sampler::samplerId() const
{
    Q_D( const Sampler );
    return d->m_samplerId;
}

void Sampler::bind( GLuint unit )
{
    Q_D( Sampler );
    d->bind( unit );
}

void Sampler::release( GLuint unit )
{
    Q_D( Sampler );
    d->release( unit );
}

void Sampler::setWrapMode( CoordinateDirection direction, GLenum wrapMode )
{
    Q_D( Sampler );
    d->setParameter( direction, wrapMode );
}

void Sampler::setMinificationFilter( GLenum filter )
{
    Q_D( Sampler );
    d->setParameter( GL_TEXTURE_MIN_FILTER, filter );
}

void Sampler::setMagnificationFilter( GLenum filter )
{
    Q_D( Sampler );
    d->setParameter( GL_TEXTURE_MAG_FILTER, filter );
}

void Sampler::setMaximumAnisotropy( float anisotropy )
{
    Q_D( Sampler );
    d->setParameter( GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy );
}
