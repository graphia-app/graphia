#ifndef TEXTURE_H
#define TEXTURE_H

#include <qopengl.h>
#include <QSharedPointer>

class QImage;
class QOpenGLFunctions;

class TexturePrivate;

class Texture
{
public:
    enum TextureType
    {
        Texture1D      = GL_TEXTURE_1D,
        Texture2D      = GL_TEXTURE_2D,
        Texture3D      = GL_TEXTURE_3D,
        TextureCubeMap = GL_TEXTURE_CUBE_MAP
    };

    Texture( TextureType type = Texture2D );
    ~Texture();

    TextureType type() const;

    bool create();
    void destroy();
    GLuint textureId() const;
    void bind();
    void release();

    void initializeToEmpty( const QSize& size );

    void setImage( const QImage& image );
    void setCubeMapImage( GLenum face, const QImage& image );
    void setRawData2D( GLenum target, int mipmapLevel, GLenum internalFormat,
                       int width, int height, int borderWidth,
                       GLenum format, GLenum type, const void* data );

    void generateMipMaps();

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
    Q_DECLARE_PRIVATE( Texture )
    TexturePrivate* d_ptr;
};

typedef QSharedPointer<Texture> TexturePtr;

#endif // TEXTURE_H
