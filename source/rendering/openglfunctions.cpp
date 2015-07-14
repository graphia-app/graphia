#include "openglfunctions.h"

#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QOffscreenSurface>

void OpenGLFunctions::resolveOpenGLFunctions()
{
    Q_ASSERT(QOpenGLContext::currentContext() != nullptr);

    if(!initializeOpenGLFunctions())
    {
        // This should never happen if checkOpenGLSupport has returned true
        qFatal("Could not obtain required OpenGL context version");
    }
}

bool OpenGLFunctions::checkOpenGLSupport()
{
    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setProfile(QSurfaceFormat::CoreProfile);

    QSurfaceFormat::setDefaultFormat(format);

    QOpenGLContext context;
    context.setFormat(format);
    context.create();

    QOffscreenSurface surface;
    surface.setFormat(format);
    surface.create();

    if(!surface.isValid())
        return false;

    context.makeCurrent(&surface);

    if(!context.isValid())
        return false;

    auto f = context.versionFunctions<OpenGLFunctions>();

    if(f == nullptr)
        return false;

    if(!f->initializeOpenGLFunctions())
        return false;

    return true;
}
