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
