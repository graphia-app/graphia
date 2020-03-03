#ifndef SDFCOMPUTEJOB_H
#define SDFCOMPUTEJOB_H

#include "gpucomputejob.h"

#include "rendering/glyphmap.h"
#include "rendering/doublebufferedtexture.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <functional>

class SDFComputeJob : public GPUComputeJob
{
private:
    DoubleBufferedTexture* _sdfTexture;
    GlyphMap* _glyphMap;

    std::function<void()> _onCompleteFn;

    void prepareGlyphMapTextureLayer(int layer, GLuint& texture);
    void generateSDF();

public:
    SDFComputeJob(DoubleBufferedTexture* sdfTexture, GlyphMap *glyphMap);

    void run() override;
    void executeWhenComplete(std::function<void()> onCompleteFn);
};

#endif // SDFCOMPUTEJOB_H
