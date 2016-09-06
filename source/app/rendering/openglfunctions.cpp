#include "openglfunctions.h"

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
    QSurfaceFormat format;
    format.setMajorVersion(4);
    format.setMinorVersion(0);
    format.setProfile(QSurfaceFormat::CoreProfile);

    QSurfaceFormat::setDefaultFormat(format);
}

class Functions
{
private:
    bool _valid = false;
    QOpenGLContext _context;
    QOffscreenSurface _surface;
    OpenGLFunctions* _f;

public:
    Functions()
    {
        _context.setFormat(QSurfaceFormat::defaultFormat());
        _context.create();

        QOffscreenSurface surface;
        _surface.setFormat(QSurfaceFormat::defaultFormat());
        _surface.create();

        if(!_surface.isValid())
            return;

        _context.makeCurrent(&_surface);

        if(!_context.isValid())
            return;

        _f = _context.versionFunctions<OpenGLFunctions>();

        if(_f == nullptr)
            return;

        if(!_f->initializeOpenGLFunctions())
            return;

        _valid = true;
    }

    bool valid() const { return _valid; }

    OpenGLFunctions* operator()() { return _f; }
};

bool OpenGLFunctions::hasOpenGLSupport()
{
    return Functions().valid();
}

QString OpenGLFunctions::info()
{
    Functions f;

    QString extensions;
    GLint numExtensions;
    f()->glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(int i = 0; i < numExtensions; i++)
    {
        extensions.append(reinterpret_cast<const char*>(f()->glGetStringi(GL_EXTENSIONS, i)));
        extensions.append(" ");
    }

    return QString("%1\n%2\n%3\n%4\n%5")
        .arg(reinterpret_cast<const char*>(f()->glGetString(GL_VENDOR)))
        .arg(reinterpret_cast<const char*>(f()->glGetString(GL_RENDERER)))
        .arg(reinterpret_cast<const char*>(f()->glGetString(GL_VERSION)))
        .arg(reinterpret_cast<const char*>(f()->glGetString(GL_SHADING_LANGUAGE_VERSION)))
        .arg(extensions);
}
