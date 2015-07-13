#include "texture.h"

#include <QImage>
#include <QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QDebug>

class TexturePrivate
{
public:
    TexturePrivate(Texture::TextureType type, Texture* qq)
        : q_ptr(qq),
          _type(type),
          _textureId(0),
          _funcs(0)
    {
    }

    bool create(QOpenGLContext* ctx);
    void destroy();
    void bind();
    void release();
    void setParameter(GLenum param, GLenum value);
    void setParameter(GLenum param, float value);

    Q_DECLARE_PUBLIC(Texture)

    Texture* q_ptr;

    Texture::TextureType _type;
    GLuint _textureId;
    QOpenGLFunctions* _funcs;
};

bool TexturePrivate::create(QOpenGLContext* ctx)
{
    _funcs = ctx->functions();
    _funcs->initializeOpenGLFunctions();
    //glGenTextures(1, &_textureId);
    return (_textureId != 0);
}

void TexturePrivate::destroy()
{
    if(_textureId)
    {
        //glDeleteTextures(1, &_textureId);
        _textureId = 0;
    }
}

void TexturePrivate::bind()
{
    //glBindTexture(_type, _textureId);
}

void TexturePrivate::release()
{
    //glBindTexture(_type, 0);
}

void TexturePrivate::setParameter(GLenum, GLenum)
{
    //glTexParameteri(_type, param, value);
}

void TexturePrivate::setParameter(GLenum, float)
{
    //glTexParameterf(_type, param, value);
}

Texture::Texture(TextureType type)
    : d_ptr(new TexturePrivate(type, this))
{
}

Texture::~Texture()
{
    delete d_ptr;
}

Texture::TextureType Texture::type() const
{
    Q_D(const Texture);
    return d->_type;
}

bool Texture::create()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    Q_ASSERT(context);
    Q_D(Texture);
    return d->create(context);
}

void Texture::destroy()
{
    Q_D(Texture);
    d->destroy();
}

GLuint Texture::textureId() const
{
    Q_D(const Texture);
    return d->_textureId;
}

void Texture::bind()
{
    Q_D(Texture);
    d->bind();
}

void Texture::release()
{
    Q_D(Texture);
    d->release();
}

void Texture::initializeToEmpty(const QSize& size)
{
    Q_D(Texture);
    Q_ASSERT(size.isValid());
    Q_ASSERT(d->_type == Texture2D);
    setRawData2D(d->_type, 0, GL_RGBA, size.width(), size.height(), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, 0);
}

void Texture::setImage(const QImage& image)
{
    Q_D(Texture);
    Q_ASSERT(d->_type == Texture2D);
    QImage glImage = QGLWidget::convertToGLFormat(image);
    setRawData2D(d->_type, 0, GL_RGBA, glImage.width(), glImage.height(), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());
}

void Texture::setCubeMapImage(GLenum face, const QImage& image)
{
    Q_D(Texture);
    Q_ASSERT(d->_type == TextureCubeMap);
    Q_UNUSED(d);
    QImage glImage = QGLWidget::convertToGLFormat(image);
    setRawData2D(face, 0, GL_RGBA8, glImage.width(), glImage.height(), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());
}

void Texture::setRawData2D(GLenum, int, GLenum,
                            int, int, int,
                            GLenum, GLenum, const void*)
{
    /*glTexImage2D(target, mipmapLevel, internalFormat, width, height,
                  borderWidth, format, type, data);*/
}

void Texture::generateMipMaps()
{
    Q_D(Texture);
    d->_funcs->glGenerateMipmap(d->_type);
}

void Texture::setWrapMode(CoordinateDirection direction, GLenum wrapMode)
{
    Q_D(Texture);
    d->setParameter(direction, wrapMode);
}

void Texture::setMinificationFilter(GLenum filter)
{
    Q_D(Texture);
    d->setParameter(GL_TEXTURE_MIN_FILTER, filter);
}

void Texture::setMagnificationFilter(GLenum filter)
{
    Q_D(Texture);
    d->setParameter(GL_TEXTURE_MAG_FILTER, filter);
}

void Texture::setMaximumAnisotropy(float anisotropy)
{
    Q_D(Texture);
    d->setParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
}
