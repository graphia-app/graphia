/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "openglfunctions.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QOffscreenSurface>

using namespace Qt::Literals::StringLiterals;

void OpenGLFunctions::resolveOpenGLFunctions()
{
    const auto* context = QOpenGLContext::currentContext();
    Q_ASSERT(context != nullptr);

    if(!initializeOpenGLFunctions())
    {
        // This should never happen if hasOpenGLSupport has returned true
        qFatal("Could not obtain required OpenGL context version");
    }

    if(context->hasExtension(QByteArrayLiteral("GL_ARB_sample_shading")))
    {
        _glMinSampleShadingARBFnPtr = reinterpret_cast<PFNGLMINSAMPLESHADINGARBPROC>(
            context->getProcAddress("glMinSampleShadingARB"));

        if(_glMinSampleShadingARBFnPtr == nullptr)
            qDebug() << "Failed to resolve glMinSampleShadingARB";
    }
}

QSurfaceFormat OpenGLFunctions::minimumFormat()
{
    QSurfaceFormat format;

#ifdef OPENGL_ES
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setMajorVersion(3);
    format.setMinorVersion(0);
#else
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
#endif

    return format;
}

void OpenGLFunctions::requestMinimumFormat()
{
#ifndef Q_OS_WIN // For reasons not understood, this causes problems on Windows
    const auto defaultFormat = QSurfaceFormat::defaultFormat();
    const std::pair<int, int> defaultVersion = {defaultFormat.majorVersion(), defaultFormat.minorVersion()};
    const auto requestFormat = minimumFormat();
    const std::pair<int, int> requestVersion = {requestFormat.majorVersion(), requestFormat.minorVersion()};

    if(defaultVersion < requestVersion)
        QSurfaceFormat::setDefaultFormat(requestFormat);
#endif
}

class QueryFunctions
{
private:
    bool _valid = false;
    QOpenGLContext _context;
    QOffscreenSurface _surface;
    QOpenGLExtraFunctions* _f = nullptr;

    QString GLubyteToQString(const GLubyte* bytes) const
    {
        QString text;

        while(*bytes != 0)
            text += static_cast<char>(*bytes++);

        return text;
    }

public:
    explicit QueryFunctions(const QSurfaceFormat& surfaceFormat)
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

        _f = _context.extraFunctions();

        if(_f == nullptr)
            return;

        _valid = true;
    }

    bool valid() const { return _valid; }

    QString getString(GLenum name) const { return GLubyteToQString(_f->glGetString(name)); }
    QString getString(GLenum name, GLuint index) const { return GLubyteToQString(_f->glGetStringi(name, index)); }

    QOpenGLExtraFunctions* operator->() { return _f; }
};

bool OpenGLFunctions::hasOpenGLSupport() { return QueryFunctions(OpenGLFunctions::minimumFormat()).valid(); }

QString OpenGLFunctions::vendor()
{
    // Normally when we're calling this we'll not have good OpenGL drivers
    // installed so use the most primitive OpenGL version we can
    QSurfaceFormat format;
    format.setMajorVersion(1);
    format.setMajorVersion(0);

    auto f = QueryFunctions(format);

    if(!f.valid())
        return {};

    return f.getString(GL_VENDOR);
}

QString OpenGLFunctions::info()
{
    auto format = QSurfaceFormat::defaultFormat();
    QueryFunctions f(format);

    if(!f.valid())
        return {};

    QString extensions;
    GLint numExtensions = 0;
    f->glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(GLint i = 0; i < numExtensions; i++)
    {
        extensions.append(f.getString(GL_EXTENSIONS, static_cast<GLuint>(i)));
        extensions.append(" ");
    }

    return u"%1\n%2\n%3\n%4\n%5"_s
        .arg(f.getString(GL_VENDOR),
             f.getString(GL_RENDERER),
             f.getString(GL_VERSION),
             f.getString(GL_SHADING_LANGUAGE_VERSION),
             extensions);
}
