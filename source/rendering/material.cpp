#include "material.h"

#include <QByteArray>
#include <QFile>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_1_3>
#include <QOpenGLFunctions_3_1>

Material::Material()
    : _shader(new QOpenGLShaderProgram),
      _funcsType(Unknown)
{
}

Material::~Material()
{
    _shader->release();
}

void Material::bind()
{
    if(!isInitialized())
        initialize();

    _shader->bind();
    foreach(const GLuint unit, _unitConfigs.keys())
    {
        const TextureUnitConfiguration& config = _unitConfigs.value(unit);

        // Bind the texture
        switch(_funcsType)
        {
        case Core:
            _funcs.core->glActiveTexture(GL_TEXTURE0 + unit);
            break;

        case Compatibility:
            _funcs.compat->glActiveTexture(GL_TEXTURE0 + unit);
            break;

        default:
            Q_ASSERT(false);
        }

        config.texture()->bind();

        // Bind the sampler (if present)
        if(config.sampler())
            config.sampler()->bind(unit);

        // Associate with sampler uniform in shader (if we know the name or location)
        if(_samplerUniforms.contains(unit))
            _shader->setUniformValue(_samplerUniforms.value(unit).constData(), unit);
    }
}

void Material::setShaders(const QString& vertexShader,
                           const QString& fragmentShader)
{
    // Create a shader program
    if(!_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
        qCritical() << QObject::tr("Could not compile vertex shader. Log:") << _shader->log();

    if(!_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
        qCritical() << QObject::tr("Could not compile fragment shader. Log:") << _shader->log();

    if(!_shader->link())
        qCritical() << QObject::tr("Could not link shader program. Log:") << _shader->log();
}

void Material::setShader(const QOpenGLShaderProgramPtr& shader)
{
    _shader = shader;
}

void Material::setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler)
{
    TextureUnitConfiguration config(texture, sampler);
    _unitConfigs.insert(unit, config);
}

void Material::setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler, const QByteArray& uniformName)
{
    setTextureUnitConfiguration(unit, texture, sampler);
    _samplerUniforms.insert(unit, uniformName);
}

void Material::setTextureUnitConfiguration(GLuint unit, TexturePtr texture)
{
    SamplerPtr sampler;
    setTextureUnitConfiguration(unit, texture, sampler);
}

void Material::setTextureUnitConfiguration(GLuint unit, TexturePtr texture, const QByteArray& uniformName)
{
    SamplerPtr sampler;
    setTextureUnitConfiguration(unit, texture, sampler, uniformName);
}

TextureUnitConfiguration Material::textureUnitConfiguration(GLuint unit) const
{
    return _unitConfigs.value(unit, TextureUnitConfiguration());
}

void Material::initialize()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if(!context)
    {
        qWarning() << "Unable to resolve OpenGL functions with out a valid context";
        return;
    }

    // Get functions object depending upon profile. Needed for glActiveTexture()
    QSurfaceFormat format = context->format();
    if(format.profile() == QSurfaceFormat::CoreProfile)
    {
        _funcs.core = context->versionFunctions<QOpenGLFunctions_3_1>();
        if(!_funcs.core)
        {
            qWarning() << "Unable to obtain OpenGL functions object";
            return;
        }
        _funcs.core->initializeOpenGLFunctions();
        _funcsType = Core;
    }
    else
    {
        _funcs.compat = context->versionFunctions<QOpenGLFunctions_1_3>();
        if(!_funcs.compat)
        {
            qWarning() << "Unable to obtain OpenGL functions object";
            return;
        }
        _funcs.compat->initializeOpenGLFunctions();
        _funcsType = Compatibility;
    }
}
