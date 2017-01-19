#include "sdfcomputejob.h"

#include "utils/shadertools.h"
#include "shared/utils/preferences.h"

#include <QImage>
#include <QGLWidget>
#include <QTime>
#include <QDir>

SDFComputeJob::SDFComputeJob(GLuint sdfTexture, std::shared_ptr<GlyphMap> glyphMap) :
    GPUComputeJob(),
    _sdfTexture(sdfTexture),
    _glyphMap(glyphMap)
{}

void SDFComputeJob::run()
{
    _glyphMap->update();
    generateSDF();
}

void SDFComputeJob::prepareGlyphMapTextureLayer(int layer, GLuint& texture)
{
    const auto& images = _glyphMap->images();

    glActiveTexture(GL_TEXTURE0);

    if(texture == 0)
        glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    QImage openGLImage = QGLWidget::convertToGLFormat(images.at(layer));

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 images.at(layer).width(), images.at(layer).height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 openGLImage.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void SDFComputeJob::prepareQuad(QOpenGLVertexArrayObject& screenQuadVAO,
                                QOpenGLBuffer& screenQuadDataBuffer,
                                QOpenGLShaderProgram& sdfShader)
{
    screenQuadVAO.create();
    screenQuadVAO.bind();

    screenQuadDataBuffer.create();
    screenQuadDataBuffer.bind();
    screenQuadDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    sdfShader.bind();
    sdfShader.enableAttributeArray("position");
    sdfShader.setAttributeBuffer("position", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    sdfShader.release();

    screenQuadDataBuffer.release();
    screenQuadVAO.release();
}

void SDFComputeJob::prepareScreenQuadDataBuffer(QOpenGLBuffer& buffer, int width, int height)
{
    GLfloat w = static_cast<GLfloat>(width);
    GLfloat h = static_cast<GLfloat>(height);
    GLfloat quadData[] =
    {
        0, 0,
        w, 0,
        w, h,

        w, h,
        0, h,
        0, 0,
    };

    buffer.bind();
    buffer.allocate(static_cast<void*>(quadData), static_cast<int>(sizeof(quadData)));
    buffer.release();
}

void SDFComputeJob::generateSDF()
{
    if(_glyphMap->images().empty())
        return;

    QOpenGLVertexArrayObject screenQuadVAO;
    QOpenGLBuffer screenQuadDataBuffer;
    QOpenGLShaderProgram sdfShader;

    ShaderTools::loadShaderProgram(sdfShader, ":/shaders/screen.vert", ":/shaders/sdf.frag");
    prepareQuad(screenQuadVAO, screenQuadDataBuffer, sdfShader);

    const int scaleFactor = 4;
    // TEXTURE_2D_ARRAY has a fixed height and width
    const int sourceWidth = _glyphMap->images().at(0).width();
    const int sourceHeight = _glyphMap->images().at(0).height();
    const int renderWidth = _glyphMap->images().at(0).width() / scaleFactor;
    const int renderHeight = _glyphMap->images().at(0).height() / scaleFactor;
    const int numImages = static_cast<int>(_glyphMap->images().size());

    prepareScreenQuadDataBuffer(screenQuadDataBuffer, renderWidth, renderHeight);

    sdfShader.bind();
    screenQuadDataBuffer.bind();
    screenQuadVAO.bind();

    glEnable(GL_BLEND);
    glDepthMask(false);

    // Set render size to texture size only
    glViewport(0, 0, renderWidth, renderHeight);
    QMatrix4x4 m;
    m.ortho(0, renderWidth, 0, renderHeight, -1.0f, 1.0f);

    // SDF FBO
    GLuint sdfFBO;
    glGenFramebuffers(1, &sdfFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sdfFBO);

    // SDF texture
    glBindTexture(GL_TEXTURE_2D_ARRAY, _sdfTexture);

    // Generate FBO texture
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
                 renderWidth, renderHeight, numImages,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set initial filtering and wrapping properties (filteiring will be changed to linear later)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint glyphTexture = 0;

    // Draw SDF texture for each layer of atlas layer
    for(int layer = 0; layer < numImages; ++layer)
    {
        // Can only render to one texture layer per draw call :(
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _sdfTexture, 0, layer);

        // Set Frame draw buffers
        GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, static_cast<GLenum*>(DrawBuffers));

        // Check FrameBuffer is OK
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            qWarning("Unable to complete framebuffer in SDFComputeJob");
            return;
        }

        // Create and load the glyph texture
        prepareGlyphMapTextureLayer(layer, glyphTexture);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glyphTexture);

        sdfShader.setUniformValue("tex", 0);
        sdfShader.setUniformValue("projectionMatrix", m);
        sdfShader.setUniformValue("scaleFactor", static_cast<GLfloat>(scaleFactor));
        sdfShader.setUniformValue("texSize", QPoint(sourceWidth, sourceHeight));

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glFlush();

    // Debug code to pull out the SDF texture
    if(u::pref("debug/saveGlyphMaps").toBool())
    {
        int pixelCount = static_cast<int>((_glyphMap->images().at(0).byteCount() / (scaleFactor * scaleFactor)) *
                                          _glyphMap->images().size());
        std::vector<uchar> pixels(pixelCount);
        glBindTexture(GL_TEXTURE_2D_ARRAY, _sdfTexture);
        glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        // Save each layer as its own image for debug
        for(int layer = 0; layer < numImages; ++layer)
        {
            int offset = (_glyphMap->images().at(0).byteCount() / (scaleFactor * scaleFactor)) * layer;
            QImage sdfImage(pixels.data() + offset, renderWidth, renderHeight, QImage::Format_RGBA8888);
            sdfImage.save(QDir::currentPath() + "/SDF" + QString::number(layer) + ".png");
        }

        // Print Memory consumption
        int memoryConsumption = (renderWidth * renderHeight * 4) * numImages;
        qDebug() << "SDF texture memory consumption MB:" << memoryConsumption / (1000.0f * 1000.0f);
    }

    glDeleteTextures(1, &glyphTexture);

    screenQuadVAO.release();
    screenQuadDataBuffer.release();
    sdfShader.release();

    glDeleteFramebuffers(1, &sdfFBO);

    // Reset back to Screen size
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(_onCompleteFn != nullptr)
        _onCompleteFn();
}

void SDFComputeJob::executeWhenComplete(std::function<void()> onCompleteFn)
{
    _onCompleteFn = onCompleteFn;
}

