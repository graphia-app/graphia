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

#include "shadertools.h"

#include "shared/utils/scope_exit.h"

#include <QOpenGLShaderProgram>
#include <QString>
#include <QObject>
#include <QtGlobal>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

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
    if(!program.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
    {
        if(!allowFailure) qFatal("Vertex shader compilation failure");
        return false;
    }

    shaderName = fragmentShader;
    if(!program.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
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
