#ifndef SDFCOMPUTEJOB_H
#define SDFCOMPUTEJOB_H

#include "gpucomputejob.h"

#include "rendering/glyphmap.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <functional>

class SDFComputeJob : public GPUComputeJob
{
private:
    GLuint _sdfTexture = 0;
    GlyphMap* _glyphMap;

    std::function<void()> _onCompleteFn;

    void prepareGlyphMapTextureLayer(int layer, GLuint& texture);
    void prepareScreenQuadDataBuffer(QOpenGLBuffer& buffer, int width, int height);
    void prepareQuad(QOpenGLVertexArrayObject& screenQuadVAO,
                     QOpenGLBuffer& screenQuadDataBuffer,
                     QOpenGLShaderProgram& sdfShader);
    void generateSDF();

public:
    SDFComputeJob(GLuint sdfTexture, GlyphMap *glyphMap);

    void run() override;
    void executeWhenComplete(std::function<void()> onCompleteFn);
};

#endif // SDFCOMPUTEJOB_H
