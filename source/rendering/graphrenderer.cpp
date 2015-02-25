#include "graphrenderer.h"

#include <QColor>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>

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

GraphRenderer::GraphRenderer(GraphWidget* graphWidget, const QOpenGLContext& context) :
    _graphWidget(graphWidget),
    _context(&context),
    _width(0), _height(0),
    _colorTexture(0),
    _selectionTexture(0),
    _depthTexture(0),
    _visualFBO(0),
    _FBOcomplete(false)
{
    _funcs = _context->versionFunctions<QOpenGLFunctions_3_3_Core>();
    if(!_funcs)
        qFatal("Could not obtain required OpenGL context version");
    _funcs->initializeOpenGLFunctions();

    loadShaderProgram(_screenShader, ":/rendering/shaders/screen.vert", ":/rendering/shaders/screen.frag");
    loadShaderProgram(_selectionShader, ":/rendering/shaders/screen.vert", ":/rendering/shaders/selection.frag");

    loadShaderProgram(_nodesShader, ":/rendering/shaders/instancednodes.vert", ":/rendering/shaders/ads.frag");
    loadShaderProgram(_edgesShader, ":/rendering/shaders/instancededges.vert", ":/rendering/shaders/ads.frag");

    loadShaderProgram(_selectionMarkerShader, ":/rendering/shaders/2d.vert", ":/rendering/shaders/selectionMarker.frag");
    loadShaderProgram(_debugLinesShader, ":/rendering/shaders/debuglines.vert", ":/rendering/shaders/debuglines.frag");

    prepareSelectionMarkerVAO();
    prepareQuad();
}

GraphRenderer::~GraphRenderer()
{
    if(_visualFBO != 0)
    {
        _funcs->glDeleteFramebuffers(1, &_visualFBO);
        _visualFBO = 0;
    }

    _FBOcomplete = false;

    if(_colorTexture != 0)
    {
        _funcs->glDeleteTextures(1, &_colorTexture);
        _colorTexture = 0;
    }

    if(_selectionTexture != 0)
    {
        _funcs->glDeleteTextures(1, &_selectionTexture);
        _selectionTexture = 0;
    }

    if(_depthTexture != 0)
    {
        _funcs->glDeleteTextures(1, &_depthTexture);
        _depthTexture = 0;
    }
}

void GraphRenderer::resize(int width, int height)
{
    _width = width;
    _height = height;

    if(width > 0 && height > 0)
        _FBOcomplete = prepareRenderBuffers(width, height);

    GLfloat w = static_cast<GLfloat>(_width);
    GLfloat h = static_cast<GLfloat>(_height);
    GLfloat data[] =
    {
        0, 0,
        w, 0,
        w, h,

        w, h,
        0, h,
        0, 0,
    };

    _screenQuadDataBuffer.bind();
    _screenQuadDataBuffer.allocate(data, static_cast<int>(sizeof(data)));
    _screenQuadDataBuffer.release();
}

void GraphRenderer::clear()
{
    if(!_FBOcomplete)
    {
        qWarning() << "Attempting to clear incomplete FBO";
        return;
    }

    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, _visualFBO);

    // Color buffer
    _funcs->glDrawBuffer(GL_COLOR_ATTACHMENT0);
    _funcs->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    _funcs->glClear(GL_COLOR_BUFFER_BIT);

    // Selection buffer
    _funcs->glDrawBuffer(GL_COLOR_ATTACHMENT1);
    _funcs->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    _funcs->glClear(GL_COLOR_BUFFER_BIT);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    _funcs->glDrawBuffers(2, drawBuffers);
    _funcs->glClear(GL_DEPTH_BUFFER_BIT);
}

void GraphRenderer::render2D()
{
    _funcs->glDisable(GL_DEPTH_TEST);

    QMatrix4x4 m;
    m.ortho(0.0f, _width, 0.0f, _height, -1.0f, 1.0f);

    if(!_selectionRect.isNull())
    {
        const QColor color(Qt::GlobalColor::white);

        QRect r;
        r.setLeft(_selectionRect.left());
        r.setRight(_selectionRect.right());
        r.setTop(_height - _selectionRect.top());
        r.setBottom(_height - _selectionRect.bottom());

        std::vector<GLfloat> data;

        data.push_back(r.left()); data.push_back(r.bottom());
        data.push_back(color.redF()); data.push_back(color.blueF()); data.push_back(color.greenF());
        data.push_back(r.right()); data.push_back(r.bottom());
        data.push_back(color.redF()); data.push_back(color.blueF()); data.push_back(color.greenF());
        data.push_back(r.right()); data.push_back(r.top());
        data.push_back(color.redF()); data.push_back(color.blueF()); data.push_back(color.greenF());

        data.push_back(r.right()); data.push_back(r.top());
        data.push_back(color.redF()); data.push_back(color.blueF()); data.push_back(color.greenF());
        data.push_back(r.left());  data.push_back(r.top());
        data.push_back(color.redF()); data.push_back(color.blueF()); data.push_back(color.greenF());
        data.push_back(r.left());  data.push_back(r.bottom());
        data.push_back(color.redF()); data.push_back(color.blueF()); data.push_back(color.greenF());

        _funcs->glDrawBuffer(GL_COLOR_ATTACHMENT1);

        _selectionMarkerDataBuffer.bind();
        _selectionMarkerDataBuffer.allocate(data.data(), static_cast<int>(data.size()) * sizeof(GLfloat));

        _selectionMarkerShader.bind();
        _selectionMarkerShader.setUniformValue("projectionMatrix", m);

        _selectionMarkerDataVAO.bind();
        _funcs->glDrawArrays(GL_TRIANGLES, 0, 6);
        _selectionMarkerDataVAO.release();

        _selectionMarkerShader.release();
        _selectionMarkerDataBuffer.release();
    }

    _funcs->glEnable(GL_DEPTH_TEST);
}

void GraphRenderer::render()
{
    if(!_FBOcomplete)
    {
        qWarning() << "Attempting to render incomplete FBO";
        return;
    }

    render2D();

    _funcs->glViewport(0, 0, _width, _height);
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    _funcs->glDisable(GL_DEPTH_TEST);

    QMatrix4x4 m;
    m.ortho(0, _width, 0, _height, -1.0f, 1.0f);

    _screenQuadDataBuffer.bind();

    _screenQuadVAO.bind();
    _funcs->glActiveTexture(GL_TEXTURE0);
    _funcs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _funcs->glEnable(GL_BLEND);

    // Color texture
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _colorTexture);

    _screenShader.bind();
    _screenShader.setUniformValue("projectionMatrix", m);
    _funcs->glDrawArrays(GL_TRIANGLES, 0, 6);
    _screenShader.release();

    // Selection texture
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture);

    _selectionShader.bind();
    _selectionShader.setUniformValue("projectionMatrix", m);
    _funcs->glDrawArrays(GL_TRIANGLES, 0, 6);
    _selectionShader.release();

    _screenQuadVAO.release();
    _screenQuadDataBuffer.release();
}

bool GraphRenderer::prepareRenderBuffers(int width, int height)
{
    bool valid;

    // Color texture
    if(_colorTexture == 0)
        _funcs->glGenTextures(1, &_colorTexture);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _colorTexture);
    _funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_MULTISAMPLES, GL_RGBA, width, height, GL_FALSE);
    _funcs->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Selection texture
    if(_selectionTexture == 0)
        _funcs->glGenTextures(1, &_selectionTexture);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture);
    _funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_MULTISAMPLES, GL_RGBA, width, height, GL_FALSE);
    _funcs->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Depth texture
    if(_depthTexture == 0)
        _funcs->glGenTextures(1, &_depthTexture);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _depthTexture);
    _funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_MULTISAMPLES, GL_DEPTH_COMPONENT, width, height, GL_FALSE);
    _funcs->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Visual FBO
    if(_visualFBO == 0)
        _funcs->glGenFramebuffers(1, &_visualFBO);
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, _visualFBO);
    _funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _colorTexture, 0);
    _funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture, 0);
    _funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, _depthTexture, 0);

    GLenum status = _funcs->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    valid = (status == GL_FRAMEBUFFER_COMPLETE);

    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Q_ASSERT(valid);
    return valid;
}

void GraphRenderer::prepareSelectionMarkerVAO()
{
    _selectionMarkerDataVAO.create();

    _selectionMarkerDataVAO.bind();
    _selectionMarkerShader.bind();

    _selectionMarkerDataBuffer.create();
    _selectionMarkerDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _selectionMarkerDataBuffer.bind();

    _selectionMarkerShader.enableAttributeArray("position");
    _selectionMarkerShader.enableAttributeArray("color");
    _selectionMarkerShader.disableAttributeArray("texCoord");
    _selectionMarkerShader.setAttributeBuffer("position", GL_FLOAT, 0, 2, 5 * sizeof(GLfloat));
    _selectionMarkerShader.setAttributeBuffer("color", GL_FLOAT, 2 * sizeof(GLfloat), 3, 5 * sizeof(GLfloat));

    _selectionMarkerDataBuffer.release();
    _selectionMarkerDataVAO.release();
    _selectionMarkerShader.release();
}

void GraphRenderer::prepareQuad()
{
    if(!_screenQuadVAO.isCreated())
        _screenQuadVAO.create();

    _screenQuadVAO.bind();

    _screenQuadDataBuffer.create();
    _screenQuadDataBuffer.bind();
    _screenQuadDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    _screenShader.bind();
    _screenShader.enableAttributeArray("position");
    _screenShader.setAttributeBuffer("position", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    _screenShader.setUniformValue("frameBufferTexture", 0);
    _screenShader.release();

    _selectionShader.bind();
    _selectionShader.enableAttributeArray("position");
    _selectionShader.setAttributeBuffer("position", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    _selectionShader.setUniformValue("frameBufferTexture", 0);
    _selectionShader.release();

    _screenQuadDataBuffer.release();
    _screenQuadVAO.release();
}
