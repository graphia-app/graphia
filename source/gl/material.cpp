#include "material.h"

#include <QByteArray>
#include <QFile>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_1_3>
#include <QOpenGLFunctions_3_1>

Material::Material()
    : m_shader( new QOpenGLShaderProgram ),
      m_funcsType( Unknown )
{
}

Material::~Material()
{
    m_shader->release();
}

void Material::bind()
{
    if ( !isInitialized() )
        initialize();

    m_shader->bind();
    foreach ( const GLuint unit, m_unitConfigs.keys() )
    {
        const TextureUnitConfiguration& config = m_unitConfigs.value( unit );

        // Bind the texture
        switch ( m_funcsType )
        {
        case Core:
            m_funcs.core->glActiveTexture( GL_TEXTURE0 + unit );
            break;

        case Compatibility:
            m_funcs.compat->glActiveTexture( GL_TEXTURE0 + unit );
            break;

        default:
            Q_ASSERT( false );
        }

        config.texture()->bind();

        // Bind the sampler (if present)
        if ( config.sampler() )
            config.sampler()->bind( unit );

        // Associate with sampler uniform in shader (if we know the name or location)
        if ( m_samplerUniforms.contains( unit ) )
            m_shader->setUniformValue( m_samplerUniforms.value( unit ).constData(), unit );
    }
}

void Material::setShaders( const QString& vertexShader,
                           const QString& fragmentShader )
{
    // Create a shader program
    if ( !m_shader->addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShader ) )
        qCritical() << QObject::tr( "Could not compile vertex shader. Log:" ) << m_shader->log();

    if ( !m_shader->addShaderFromSourceFile( QOpenGLShader::Fragment, fragmentShader ) )
        qCritical() << QObject::tr( "Could not compile fragment shader. Log:" ) << m_shader->log();

    if ( !m_shader->link() )
        qCritical() << QObject::tr( "Could not link shader program. Log:" ) << m_shader->log();
}

void Material::setShader( const QOpenGLShaderProgramPtr& shader )
{
    m_shader = shader;
}

void Material::setTextureUnitConfiguration( GLuint unit, TexturePtr texture, SamplerPtr sampler )
{
    TextureUnitConfiguration config( texture, sampler );
    m_unitConfigs.insert( unit, config );
}

void Material::setTextureUnitConfiguration( GLuint unit, TexturePtr texture, SamplerPtr sampler, const QByteArray& uniformName )
{
    setTextureUnitConfiguration( unit, texture, sampler );
    m_samplerUniforms.insert( unit, uniformName );
}

void Material::setTextureUnitConfiguration( GLuint unit, TexturePtr texture )
{
    SamplerPtr sampler;
    setTextureUnitConfiguration( unit, texture, sampler );
}

void Material::setTextureUnitConfiguration( GLuint unit, TexturePtr texture, const QByteArray& uniformName )
{
    SamplerPtr sampler;
    setTextureUnitConfiguration( unit, texture, sampler, uniformName );
}

TextureUnitConfiguration Material::textureUnitConfiguration( GLuint unit ) const
{
    return m_unitConfigs.value( unit, TextureUnitConfiguration() );
}

void Material::initialize()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if ( !context )
    {
        qWarning() << "Unable to resolve OpenGL functions with out a valid context";
        return;
    }

    // Get functions object depending upon profile. Needed for glActiveTexture()
    QSurfaceFormat format = context->format();
    if ( format.profile() == QSurfaceFormat::CoreProfile )
    {
        m_funcs.core = context->versionFunctions<QOpenGLFunctions_3_1>();
        if ( !m_funcs.core )
        {
            qWarning() << "Unable to obtain OpenGL functions object";
            return;
        }
        m_funcs.core->initializeOpenGLFunctions();
        m_funcsType = Core;
    }
    else
    {
        m_funcs.compat = context->versionFunctions<QOpenGLFunctions_1_3>();
        if ( !m_funcs.compat )
        {
            qWarning() << "Unable to obtain OpenGL functions object";
            return;
        }
        m_funcs.compat->initializeOpenGLFunctions();
        m_funcsType = Compatibility;
    }
}
