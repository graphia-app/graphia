#ifndef MATERIAL_H
#define MATERIAL_H

#include "sampler.h"
#include "texture.h"

#include <QMap>
#include <QOpenGLShaderProgram>
#include <QPair>
#include <QSharedPointer>

class QOpenGLFunctions_3_1;
class QOpenGLFunctions_1_3;

typedef QSharedPointer<QOpenGLShaderProgram> QOpenGLShaderProgramPtr;

class TextureUnitConfiguration : public QPair<TexturePtr, SamplerPtr>
{
public:
    TextureUnitConfiguration()
        : QPair<TexturePtr, SamplerPtr>(TexturePtr(), SamplerPtr())
    {
    }

    explicit TextureUnitConfiguration(const TexturePtr& texture, const SamplerPtr& sampler)
        : QPair<TexturePtr, SamplerPtr>(texture, sampler)
    {
    }

    void setTexture(const TexturePtr& texture) { first = texture; }
    TexturePtr texture() const { return first; }

    void setSampler(const SamplerPtr sampler) { second = sampler; }
    SamplerPtr sampler() const { return second; }
};

class Material
{
public:
    Material();
    ~Material();

    void bind();

    void setShaders(const QString& vertexShader,
                     const QString& fragmentShader);

    void setShader(const QOpenGLShaderProgramPtr& shader);

    QOpenGLShaderProgramPtr shader() const { return _shader; }

    void setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler);
    void setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler, const QByteArray& uniformName);

    void setTextureUnitConfiguration(GLuint unit, TexturePtr texture);
    void setTextureUnitConfiguration(GLuint unit, TexturePtr texture, const QByteArray& uniformName);

    TextureUnitConfiguration textureUnitConfiguration(GLuint unit) const;

private:
    void initialize();
    bool isInitialized() const { return _funcsType != Unknown; }

    // For now we assume that we own the shader
    /** \todo Allow this to use reference to non-owned shader */
    QOpenGLShaderProgramPtr _shader;

    // This map contains the configuration for the texture units
    QMap<GLuint, TextureUnitConfiguration> _unitConfigs;
    QMap<GLuint, QByteArray> _samplerUniforms;

    union
    {
        QOpenGLFunctions_3_1* core;
        QOpenGLFunctions_1_3* compat;
    } _funcs;

    enum
    {
        Unknown,
        Core,
        Compatibility
    } _funcsType;
};

typedef QSharedPointer<Material> MaterialPtr;

#endif // MATERIAL_H
