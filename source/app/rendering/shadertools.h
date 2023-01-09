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

#ifndef SHADERTOOLS_H
#define SHADERTOOLS_H

#include <QOpenGLShaderProgram>

class ShaderTools
{
public:
    static bool loadShaderProgram(QOpenGLShaderProgram& program, const QString& vertexShader, const QString& fragmentShader)
    {
        if(!program.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
        {
            qCritical() << QObject::tr("Could not compile vertex shader. Log:") << program.log();
            return false;
        }

        if(!program.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
        {
            qCritical() << QObject::tr("Could not compile fragment shader. Log:") << program.log();
            return false;
        }

        if(!program.link())
        {
            qCritical() << QObject::tr("Could not link shader program. Log:") << program.log();
            return false;
        }

        return true;
    }
};

#endif // SHADERTOOLS_H
