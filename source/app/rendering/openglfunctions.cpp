#include "openglfunctions.h"

#include <QOpenGLFunctions_1_0>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QOffscreenSurface>

void OpenGLFunctions::resolveOpenGLFunctions()
{
    Q_ASSERT(QOpenGLContext::currentContext() != nullptr);

    if(!initializeOpenGLFunctions())
    {
        // This should never happen if hasOpenGLSupport has returned true
        qFatal("Could not obtain required OpenGL context version");
    }
}

void OpenGLFunctions::setDefaultFormat()
{
    defaultFormat();
}

QSurfaceFormat OpenGLFunctions::defaultFormat()
{
    QSurfaceFormat format;
    format.setMajorVersion(4);
    format.setMinorVersion(0);
    format.setProfile(QSurfaceFormat::CoreProfile);

    QSurfaceFormat::setDefaultFormat(format);
    return QSurfaceFormat::defaultFormat();
}

template<typename F>
class Functions
{
private:
    bool _valid = false;
    QOpenGLContext _context;
    QOffscreenSurface _surface;
    F* _f;

    QString GLubyteToQString(const GLubyte* bytes) const
    {
        QString string;

        while(*bytes != 0)
            string += *bytes++;

        return string;
    }

public:
    Functions(const QSurfaceFormat& surfaceFormat)
    {
        _context.setFormat(surfaceFormat);
        _context.create();

        _surface.setFormat(surfaceFormat);
        _surface.create();

        if(!_surface.isValid())
            return;

        _context.makeCurrent(&_surface);

        if(!_context.isValid())
            return;

        _f = _context.versionFunctions<F>();

        if(_f == nullptr)
            return;

        if(!_f->initializeOpenGLFunctions())
            return;

        _valid = true;
    }

    bool valid() const { return _valid; }

    QString getString(GLenum name) const
    {
        return GLubyteToQString(_f->glGetString(name));
    }

    QString getString(GLenum name, GLuint index) const
    {
        return GLubyteToQString(_f->glGetStringi(name, index));
    }

    F* operator->() { return _f; }
};

bool OpenGLFunctions::hasOpenGLSupport()
{
    auto format = OpenGLFunctions::defaultFormat();
    return Functions<OpenGLFunctions>(format).valid();
}

QString OpenGLFunctions::vendor()
{
    // Normally when we're calling this we'll not have good OpenGL drivers
    // installed so use the most primitive OpenGL version we can
    QSurfaceFormat format;
    format.setMajorVersion(1);
    format.setMajorVersion(0);

    return Functions<QOpenGLFunctions_1_0>(format).getString(GL_VENDOR);
}

QString OpenGLFunctions::info()
{
    auto format = OpenGLFunctions::defaultFormat();
    Functions<OpenGLFunctions> f(format);

    QString extensions;
    GLint numExtensions;
    f->glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(int i = 0; i < numExtensions; i++)
    {
        extensions.append(f.getString(GL_EXTENSIONS, i));
        extensions.append(" ");
    }

    return QStringLiteral("%1\n%2\n%3\n%4\n%5")
        .arg(f.getString(GL_VENDOR),
             f.getString(GL_RENDERER),
             f.getString(GL_VERSION),
             f.getString(GL_SHADING_LANGUAGE_VERSION),
             extensions);
}
