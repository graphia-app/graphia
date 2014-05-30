#include "sampler.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLExtensions>

class SamplerPrivate
{
public:
    SamplerPrivate(Sampler* qq)
        : q_ptr(qq),
#if !defined(QT_OPENGL_ES_2)
          _samplerId(0)
#endif
    {
    }

    bool create(QOpenGLContext* ctx);
    void destroy();
    void bind(GLuint unit);
    void release(GLuint unit);
    void setParameter(GLenum param, GLenum value);
    void setParameter(GLenum param, float value);

    Q_DECLARE_PUBLIC(Sampler)

    Sampler* q_ptr;

#if !defined(QT_OPENGL_ES_2)
    GLuint _samplerId;

    union {
        QOpenGLFunctions_3_3_Core* core;
        QOpenGLExtension_ARB_sampler_objects *arb;
    } _funcs;

    enum {
        NotSupported,
        Core,
        ARB
    } _funcsType;
#endif
};

bool SamplerPrivate::create(QOpenGLContext* ctx)
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
    return false;
#else
    _funcs.core = 0;
    _funcsType = NotSupported;
    QSurfaceFormat format = ctx->format();
    const int version = (format.majorVersion() << 8) + format.minorVersion();

    if(version >= 0x0303)
    {
        _funcs.core = ctx->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if(!_funcs.core)
            return false;
        _funcsType = Core;
        _funcs.core->initializeOpenGLFunctions();
        _funcs.core->glGenSamplers(1, &_samplerId);
    }
    else if(ctx->hasExtension("GL_ARB_sampler_objects"))
    {
        _funcs.arb = new QOpenGLExtension_ARB_sampler_objects;
        if (!_funcs.arb->initializeOpenGLFunctions())
            return false;
        _funcsType = ARB;
        _funcs.arb->glGenSamplers(1, &_samplerId);
    }

    return(_samplerId != 0);
#endif
}

void SamplerPrivate::destroy()
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    if(_samplerId)
    {
        switch(_funcsType)
        {
            case Core:
                _funcs.core->glDeleteSamplers(1, &_samplerId);
                break;

            case ARB:
                _funcs.arb->glDeleteSamplers(1, &_samplerId);
                break;

            default:
            case NotSupported:
                // Shut the compiler up
                qt_noop();
        }
        _samplerId = 0;
    }
#endif
}

void SamplerPrivate::bind(GLuint unit)
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    switch(_funcsType)
    {
        case Core:
            _funcs.core->glBindSampler(unit, _samplerId);
            break;

        case ARB:
            _funcs.arb->glBindSampler(unit, _samplerId);
            break;

        default:
        case NotSupported:
            // Shut the compiler up
            qt_noop();
    }
#endif
}

void SamplerPrivate::release(GLuint unit)
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    switch(_funcsType)
    {
        case Core:
            _funcs.core->glBindSampler(unit, 0);
            break;

        case ARB:
            _funcs.arb->glBindSampler(unit, 0);
            break;

        default:
        case NotSupported:
            // Shut the compiler up
            qt_noop();
    }
#endif
}

void SamplerPrivate::setParameter(GLenum param, GLenum value)
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    switch(_funcsType)
    {
        case Core:
            _funcs.core->glSamplerParameteri(_samplerId, param, value);
            break;

        case ARB:
            _funcs.arb->glSamplerParameteri(_samplerId, param, value);
            break;

        default:
        case NotSupported:
            // Shut the compiler up
            qt_noop();
    }
#endif
}

void SamplerPrivate::setParameter(GLenum param, float value)
{
#if defined(QT_OPENGL_ES_2)
    qFatal("Sampler objects are not supported on OpenGL ES 2");
#else
    switch(_funcsType)
    {
        case Core:
            _funcs.core->glSamplerParameterf(_samplerId, param, value);
            break;

        case ARB:
            _funcs.arb->glSamplerParameterf(_samplerId, param, value);
            break;

        default:
        case NotSupported:
            // Shut the compiler up
            qt_noop();
    }
#endif
}

Sampler::Sampler()
    : d_ptr(new SamplerPrivate(this))
{
}

Sampler::~Sampler()
{
}

bool Sampler::create()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    Q_ASSERT(context);
    Q_D(Sampler);
    return d->create(context);
}

void Sampler::destroy()
{
    Q_D(Sampler);
    d->destroy();
}

bool Sampler::isCreated() const
{
    Q_D(const Sampler);
    return(d->_samplerId != 0);
}

GLuint Sampler::samplerId() const
{
    Q_D(const Sampler);
    return d->_samplerId;
}

void Sampler::bind(GLuint unit)
{
    Q_D(Sampler);
    d->bind(unit);
}

void Sampler::release(GLuint unit)
{
    Q_D(Sampler);
    d->release(unit);
}

void Sampler::setWrapMode(CoordinateDirection direction, GLenum wrapMode)
{
    Q_D(Sampler);
    d->setParameter(direction, wrapMode);
}

void Sampler::setMinificationFilter(GLenum filter)
{
    Q_D(Sampler);
    d->setParameter(GL_TEXTURE_MIN_FILTER, filter);
}

void Sampler::setMagnificationFilter(GLenum filter)
{
    Q_D(Sampler);
    d->setParameter(GL_TEXTURE_MAG_FILTER, filter);
}

void Sampler::setMaximumAnisotropy(float anisotropy)
{
    Q_D(Sampler);
    d->setParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
}
