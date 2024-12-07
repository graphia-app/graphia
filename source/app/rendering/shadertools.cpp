/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "shadertools.h"
#include "openglfunctions.h"

#include "shared/utils/scope_exit.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QString>
#include <QObject>
#include <QFile>
#include <QtGlobal>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

static QString shaderStringFromFile(const QString& filename)
{
    const QSurfaceFormat minimumFormat = OpenGLFunctions::minimumFormat();
    const int shaderVersionNumber = (minimumFormat.majorVersion() * 100) +
        (minimumFormat.minorVersion() * 10);


    QFile file(filename);
    if(!file.open(QFile::ReadOnly))
    {
        qFatal("Failed to open shader file");
        return {};
    }

    const QString contents = file.readAll();

    const QSurfaceFormat currentSurfaceFormat = QOpenGLContext::currentContext()->format();
    const QString version = currentSurfaceFormat.renderableType() == QSurfaceFormat::OpenGLES ?
        u"#version %1 es\nprecision mediump float;\n"_s.arg(shaderVersionNumber) :
        u"#version %1 core\n#define mediump\n"_s.arg(shaderVersionNumber);

    return version + contents;
}

bool ShaderTools::loadShaderProgram(QOpenGLShaderProgram& program,
    const QString& vertexShader, const QString& fragmentShader, bool allowFailure)
{
    // Temporary message handler that prepends the shader name to any error output
    // that the Qt shader compilation code produces
    static QString shaderName;
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg)
    {
        const std::string lineEnding = !msg.endsWith('\n') ? "\n" : "";
        std::cerr << shaderName.toStdString() << ": " << msg.toStdString() << lineEnding;
    });

    auto atExit = std::experimental::make_scope_exit([] { qInstallMessageHandler(nullptr); });

    shaderName = vertexShader;
    if(!program.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderStringFromFile(vertexShader)))
    {
        if(!allowFailure) qFatal("Vertex shader compilation failure");
        return false;
    }

    shaderName = fragmentShader;
    if(!program.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderStringFromFile(fragmentShader)))
    {
        if(!allowFailure) qFatal("Fragment shader compilation failure");
        return false;
    }

    shaderName = u"%1, %2"_s.arg(vertexShader, fragmentShader);
    if(!program.link())
    {
        if(!allowFailure) qFatal("Shader link failure");
        return false;
    }

    return true;
}
