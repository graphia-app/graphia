#include "graphrenderer.h"

#include "glyphmap.h"
#include "graphcomponentscene.h"
#include "graphoverviewscene.h"
#include "compute/sdfcomputejob.h"
#include "shared/utils/preferences.h"
#include "graph/graphmodel.h"
#include "ui/graphcomponentinteractor.h"
#include "ui/graphoverviewinteractor.h"
#include "ui/document.h"
#include "ui/graphquickitem.h"
#include "ui/selectionmanager.h"
#include "utils/shadertools.h"

#include <QObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLDebugLogger>
#include <QColor>
#include <QQuickWindow>
#include <QEvent>
#include <QNativeGestureEvent>
#include <QTextLayout>

#include <cstddef>

template<typename T>
void setupTexture(T t, GLuint& texture, int width, int height, GLint format)
{
    if(texture == 0)
        t->glGenTextures(1, &texture);
    t->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    t->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, GraphRenderer::NUM_MULTISAMPLES, format, width, height, GL_FALSE);
    t->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    t->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void GraphInitialiser::initialiseFromGraph(const Graph *graph)
{
    for(auto componentId : graph->componentIds())
        onComponentAdded(graph, componentId, false);

    onGraphChanged(graph, true);
}

GPUGraphData::GPUGraphData()
{
    resolveOpenGLFunctions();
}

void GPUGraphData::initialise(QOpenGLShaderProgram& nodesShader,
                              QOpenGLShaderProgram& edgesShader,
                              QOpenGLShaderProgram& textShader)
{
    _sphere.setRadius(1.0f);
    _sphere.setRings(16);
    _sphere.setSlices(16);
    _sphere.create(nodesShader);

    _arrow.setRadius(1.0f);
    _arrow.setLength(1.0f);
    _arrow.setSlices(8);
    _arrow.create(edgesShader);

    _rectangle.create(textShader);

    prepareVertexBuffers();
    prepareNodeVAO(nodesShader);
    prepareEdgeVAO(edgesShader);
    prepareTextVAO(textShader);
}

GPUGraphData::~GPUGraphData()
{
    if(_fbo != 0)
    {
        glDeleteFramebuffers(1, &_fbo);
        _fbo = 0;
    }

    if(_colorTexture != 0)
    {
        glDeleteTextures(1, &_colorTexture);
        _colorTexture = 0;
    }

    if(_selectionTexture != 0)
    {
        glDeleteTextures(1, &_selectionTexture);
        _selectionTexture = 0;
    }
}

void GPUGraphData::prepareVertexBuffers()
{
    if(!_nodeVBO.isCreated())
    {
        _nodeVBO.create();
        _nodeVBO.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    }

    if(!_textVBO.isCreated())
    {
        _textVBO.create();
        _textVBO.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    }

    if(!_edgeVBO.isCreated())
    {
        _edgeVBO.create();
        _edgeVBO.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    }
}

void GPUGraphData::prepareTextVAO(QOpenGLShaderProgram& shader)
{
    _rectangle.vertexArrayObject()->bind();
    shader.bind();
    _textVBO.bind();

    shader.enableAttributeArray("component");
    shader.enableAttributeArray("textureCoord");
    shader.enableAttributeArray("textureLayer");
    shader.enableAttributeArray("basePosition");
    shader.enableAttributeArray("glyphOffset");
    shader.enableAttributeArray("glyphSize");
    shader.enableAttributeArray("color");

    glVertexAttribIPointer(shader.attributeLocation("component"),                                 1, GL_INT, sizeof(GlyphData),
                             reinterpret_cast<const void*>(offsetof(GlyphData, _component)));
    shader.setAttributeBuffer("textureCoord",    GL_FLOAT, offsetof(GlyphData, _textureCoord),    2, sizeof(GlyphData));
    glVertexAttribIPointer(shader.attributeLocation("textureLayer"),                              1, GL_INT, sizeof(GlyphData),
                             reinterpret_cast<const void*>(offsetof(GlyphData, _textureLayer)));
    shader.setAttributeBuffer("basePosition",    GL_FLOAT, offsetof(GlyphData, _basePosition),    3, sizeof(GlyphData));
    shader.setAttributeBuffer("glyphOffset",     GL_FLOAT, offsetof(GlyphData, _glyphOffset),     2, sizeof(GlyphData));
    shader.setAttributeBuffer("glyphSize",       GL_FLOAT, offsetof(GlyphData, _glyphSize),       2, sizeof(GlyphData));
    shader.setAttributeBuffer("color",           GL_FLOAT, offsetof(GlyphData, _color),           3, sizeof(GlyphData));

    glVertexAttribDivisor(shader.attributeLocation("component"), 1);
    glVertexAttribDivisor(shader.attributeLocation("textureCoord"), 1);
    glVertexAttribDivisor(shader.attributeLocation("textureLayer"), 1);
    glVertexAttribDivisor(shader.attributeLocation("basePosition"), 1);
    glVertexAttribDivisor(shader.attributeLocation("glyphOffset"), 1);
    glVertexAttribDivisor(shader.attributeLocation("glyphSize"), 1);
    glVertexAttribDivisor(shader.attributeLocation("color"), 1);

    _textVBO.release();
    shader.release();
    _rectangle.vertexArrayObject()->release();
}

void GPUGraphData::prepareNodeVAO(QOpenGLShaderProgram& shader)
{
    _sphere.vertexArrayObject()->bind();
    shader.bind();

    _nodeVBO.bind();
    shader.enableAttributeArray("nodePosition");
    shader.enableAttributeArray("component");
    shader.enableAttributeArray("size");
    shader.enableAttributeArray("color");
    shader.enableAttributeArray("outlineColor");
    shader.setAttributeBuffer("nodePosition", GL_FLOAT, offsetof(NodeData, _position),     3,         sizeof(NodeData));
    glVertexAttribIPointer(shader.attributeLocation("component"),                          1, GL_INT, sizeof(NodeData),
                          reinterpret_cast<const void*>(offsetof(NodeData, _component)));
    shader.setAttributeBuffer("size",         GL_FLOAT, offsetof(NodeData, _size),         1,         sizeof(NodeData));
    shader.setAttributeBuffer("color",        GL_FLOAT, offsetof(NodeData, _color),        3,         sizeof(NodeData));
    shader.setAttributeBuffer("outlineColor", GL_FLOAT, offsetof(NodeData, _outlineColor), 3,         sizeof(NodeData));
    glVertexAttribDivisor(shader.attributeLocation("nodePosition"), 1);
    glVertexAttribDivisor(shader.attributeLocation("component"), 1);
    glVertexAttribDivisor(shader.attributeLocation("size"), 1);
    glVertexAttribDivisor(shader.attributeLocation("color"), 1);
    glVertexAttribDivisor(shader.attributeLocation("outlineColor"), 1);
    _nodeVBO.release();

    shader.release();
    _sphere.vertexArrayObject()->release();
}

void GPUGraphData::prepareEdgeVAO(QOpenGLShaderProgram& shader)
{
    _arrow.vertexArrayObject()->bind();
    shader.bind();

    _edgeVBO.bind();
    shader.enableAttributeArray("sourcePosition");
    shader.enableAttributeArray("targetPosition");
    shader.enableAttributeArray("sourceSize");
    shader.enableAttributeArray("targetSize");
    shader.enableAttributeArray("edgeType");
    shader.enableAttributeArray("component");
    shader.enableAttributeArray("size");
    shader.enableAttributeArray("color");
    shader.enableAttributeArray("outlineColor");
    shader.setAttributeBuffer("sourcePosition", GL_FLOAT, offsetof(EdgeData, _sourcePosition),  3,         sizeof(EdgeData));
    shader.setAttributeBuffer("targetPosition", GL_FLOAT, offsetof(EdgeData, _targetPosition),  3,         sizeof(EdgeData));
    shader.setAttributeBuffer("sourceSize",     GL_FLOAT, offsetof(EdgeData, _sourceSize),      1,         sizeof(EdgeData));
    shader.setAttributeBuffer("targetSize",     GL_FLOAT, offsetof(EdgeData, _targetSize),      1,         sizeof(EdgeData));
    glVertexAttribIPointer(shader.attributeLocation("edgeType"),                                1, GL_INT, sizeof(EdgeData),
                            reinterpret_cast<const void*>(offsetof(EdgeData, _edgeType)));
    glVertexAttribIPointer(shader.attributeLocation("component"),                               1, GL_INT, sizeof(EdgeData),
                           reinterpret_cast<const void*>(offsetof(EdgeData, _component)));
    shader.setAttributeBuffer("size",           GL_FLOAT, offsetof(EdgeData, _size),            1,         sizeof(EdgeData));
    shader.setAttributeBuffer("color",          GL_FLOAT, offsetof(EdgeData, _color),           3,         sizeof(EdgeData));
    shader.setAttributeBuffer("outlineColor",   GL_FLOAT, offsetof(EdgeData, _outlineColor),    3,         sizeof(EdgeData));
    glVertexAttribDivisor(shader.attributeLocation("sourcePosition"),   1);
    glVertexAttribDivisor(shader.attributeLocation("targetPosition"),   1);
    glVertexAttribDivisor(shader.attributeLocation("sourceSize"),       1);
    glVertexAttribDivisor(shader.attributeLocation("targetSize"),       1);
    glVertexAttribDivisor(shader.attributeLocation("edgeType"),         1);
    glVertexAttribDivisor(shader.attributeLocation("component"),        1);
    glVertexAttribDivisor(shader.attributeLocation("size"),             1);
    glVertexAttribDivisor(shader.attributeLocation("color"),            1);
    glVertexAttribDivisor(shader.attributeLocation("outlineColor"),     1);
    _edgeVBO.release();

    shader.release();
    _arrow.vertexArrayObject()->release();
}

bool GPUGraphData::prepareRenderBuffers(int width, int height, GLuint depthTexture)
{
    setupTexture(this, _colorTexture,     width, height, GL_RGBA);
    setupTexture(this, _selectionTexture, width, height, GL_RGBA);

    if(_fbo == 0)
        glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D_MULTISAMPLE, depthTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    bool fboValid = (status == GL_FRAMEBUFFER_COMPLETE);
    Q_ASSERT(fboValid);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fboValid;
}

void GPUGraphData::reset()
{
    _alpha1 = 0.0f;
    _alpha2 = 0.0f;
    _nodeData.clear();
    _edgeData.clear();
    _glyphData.clear();
}

void GPUGraphData::clearFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, static_cast<GLenum*>(drawBuffers));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GPUGraphData::clearDepthbuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GPUGraphData::upload()
{
    _nodeVBO.bind();
    _nodeVBO.allocate(_nodeData.data(), static_cast<int>(_nodeData.size()) * sizeof(NodeData));
    _nodeVBO.release();

    _edgeVBO.bind();
    _edgeVBO.allocate(_edgeData.data(), static_cast<int>(_edgeData.size()) * sizeof(EdgeData));
    _edgeVBO.release();

    _textVBO.bind();
    _textVBO.allocate(_glyphData.data(), static_cast<int>(_glyphData.size()) * sizeof(GlyphData));
    _textVBO.release();
}

int GPUGraphData::numNodes() const
{
    return static_cast<int>(_nodeData.size());
}

int GPUGraphData::numEdges() const
{
    return static_cast<int>(_edgeData.size());
}

float GPUGraphData::alpha() const
{
    return _alpha1 * _alpha2;
}

bool GPUGraphData::unused() const
{
    return _alpha1 == 0.0f && _alpha2 == 0.0f;
}

GraphRenderer::GraphRenderer(GraphModel* graphModel,
                             CommandManager* commandManager,
                             SelectionManager* selectionManager,
                             GPUComputeThread* gpuComputeThread) :
    QObject(),
    OpenGLFunctions(),
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _gpuComputeThread(gpuComputeThread),
    _componentRenderers(_graphModel->graph()),
    _hiddenNodes(_graphModel->graph()),
    _hiddenEdges(_graphModel->graph()),
    _layoutChanged(true),
    _performanceCounter(std::chrono::seconds(1))
{
    resolveOpenGLFunctions();

    // Shade all samples in multi-sampling
    glMinSampleShading(1.0);

    ShaderTools::loadShaderProgram(_screenShader, ":/shaders/screen.vert", ":/shaders/screen.frag");
    ShaderTools::loadShaderProgram(_selectionShader, ":/shaders/screen.vert", ":/shaders/selection.frag");
    ShaderTools::loadShaderProgram(_sdfShader, ":/shaders/screen.vert", ":/shaders/sdf.frag");

    ShaderTools::loadShaderProgram(_nodesShader, ":/shaders/instancednodes.vert", ":/shaders/ads.frag");
    ShaderTools::loadShaderProgram(_edgesShader, ":/shaders/instancededges.vert", ":/shaders/ads.frag");

    ShaderTools::loadShaderProgram(_selectionMarkerShader, ":/shaders/2d.vert", ":/shaders/selectionMarker.frag");
    ShaderTools::loadShaderProgram(_debugLinesShader, ":/shaders/debuglines.vert", ":/shaders/debuglines.frag");

    ShaderTools::loadShaderProgram(_textShader, ":/shaders/textrender.vert", ":/shaders/textrender.frag");

    prepareSelectionMarkerVAO();
    prepareQuad();

    for(auto& gpuGraphData : _gpuGraphData)
        gpuGraphData.initialise(_nodesShader, _edgesShader, _textShader);

    prepareComponentDataTexture();

    _glyphMap = std::make_shared<GlyphMap>(u::pref("visuals/textFont").toString());
    prepareSDFTextures();

    auto graph = &_graphModel->graph();

    connect(graph, &Graph::nodeAdded, this, &GraphRenderer::onNodeAdded, Qt::DirectConnection);
    connect(graph, &Graph::edgeAdded, this, &GraphRenderer::onEdgeAdded, Qt::DirectConnection);
    connect(graph, &Graph::nodeAddedToComponent, this, &GraphRenderer::onNodeAddedToComponent, Qt::DirectConnection);
    connect(graph, &Graph::edgeAddedToComponent, this, &GraphRenderer::onEdgeAddedToComponent, Qt::DirectConnection);

    connect(graph, &Graph::graphWillChange, this, &GraphRenderer::onGraphWillChange, Qt::DirectConnection);
    connect(graph, &Graph::graphChanged, this, &GraphRenderer::onGraphChanged, Qt::DirectConnection);
    connect(graph, &Graph::componentAdded, this, &GraphRenderer::onComponentAdded, Qt::DirectConnection);

    connect(&_transition, &Transition::started, this, &GraphRenderer::rendererStartedTransition, Qt::DirectConnection);
    connect(&_transition, &Transition::finished, this, &GraphRenderer::rendererFinishedTransition, Qt::DirectConnection);

    _graphOverviewScene = new GraphOverviewScene(commandManager, this);
    _graphComponentScene = new GraphComponentScene(this);

    connect(graph, &Graph::componentWillBeRemoved, this, &GraphRenderer::onComponentWillBeRemoved, Qt::DirectConnection);

    _graphOverviewInteractor = new GraphOverviewInteractor(_graphModel, _graphOverviewScene, commandManager, _selectionManager, this);
    _graphComponentInteractor = new GraphComponentInteractor(_graphModel, _graphComponentScene, commandManager, _selectionManager, this);

    initialiseFromGraph(graph);
    _graphOverviewScene->initialiseFromGraph(graph);
    _graphComponentScene->initialiseFromGraph(graph);

    if(graph->componentIds().size() == 1)
        switchToComponentMode(false);
    else
        switchToOverviewMode(false);

    connect(S(Preferences), &Preferences::preferenceChanged, this, &GraphRenderer::onPreferenceChanged, Qt::DirectConnection);
    connect(_graphModel, &GraphModel::visualsWillChange, [this]
    {
        disableSceneUpdate();
    });

    connect(_graphModel, &GraphModel::visualsChanged, [this]
    {
        updateText();

        executeOnRendererThread([this]
        {
            updateGPUData(When::Later);
            update(); // QQuickFramebufferObject::Renderer::update
        }, "GraphModel::visualsChanged");

        enableSceneUpdate();
    });

    _performanceCounter.setReportFn([this](float ticksPerSecond)
    {
        emit fpsChanged(ticksPerSecond);
    });

    updateText(true);
    enableSceneUpdate();
}

GraphRenderer::~GraphRenderer()
{
    if(_componentDataTBO != 0)
    {
        glDeleteBuffers(1, &_componentDataTBO);
        _componentDataTBO = 0;
    }

    if(_componentDataTexture != 0)
    {
        glDeleteTextures(1, &_componentDataTexture);
        _componentDataTexture = 0;
    }

    _FBOcomplete = false;

    if(_depthTexture != 0)
    {
        glDeleteTextures(1, &_depthTexture);
        _depthTexture = 0;
    }

    if(_sdfTextures[0] != 0)
    {
        glDeleteTextures(2, &_sdfTextures[0]);
        _sdfTextures = {};
    }
}

void GraphRenderer::resize(int width, int height)
{
    _width = width;
    _height = height;
    _resized = true;

    if(width > 0 && height > 0)
    {
        setupTexture(this, _depthTexture, width, height, GL_DEPTH_COMPONENT);

        if(!_gpuGraphData.empty())
        {
            _FBOcomplete = true;
            for(auto& gpuGraphData : _gpuGraphData)
            {
                _FBOcomplete = _FBOcomplete &&
                        gpuGraphData.prepareRenderBuffers(width, height, _depthTexture);
            }
        }
        else
            _FBOcomplete = false;
    }

    auto w = static_cast<GLfloat>(_width);
    auto h = static_cast<GLfloat>(_height);
    GLfloat quadData[] =
    {
        0, 0,
        w, 0,
        w, h,

        w, h,
        0, h,
        0, 0,
    };

    _screenQuadDataBuffer.bind();
    _screenQuadDataBuffer.allocate(static_cast<void*>(quadData), static_cast<int>(sizeof(quadData)));
    _screenQuadDataBuffer.release();
}

GPUGraphData* GraphRenderer::gpuGraphDataForAlpha(float alpha1, float alpha2)
{
    for(auto& gpuGraphData : _gpuGraphData)
    {
        if(gpuGraphData.unused())
        {
            gpuGraphData._alpha1 = alpha1;
            gpuGraphData._alpha2 = alpha2;
            return &gpuGraphData;
        }
        else if(gpuGraphData._alpha1 == alpha1 && gpuGraphData._alpha2 == alpha2)
            return &gpuGraphData;
    }

    qWarning() << "Not enough gpuGraphData instances for" << alpha1 << alpha2;
    for(auto& gpuGraphData : _gpuGraphData)
        qWarning() << "  " << gpuGraphData._alpha1 << gpuGraphData._alpha2;

    return nullptr;
}

void GraphRenderer::createGPUGlyphData(const QString& text, const QColor& textColor, const TextAlignment& textAlignment,
                                    float textScale, float elementSize, const QVector3D& elementPosition,
                                    int componentIndex, GPUGraphData* gpuGraphData)
{
    auto& textLayout = _textLayoutResults._layouts[text];

    auto verticalCentre = -textLayout._xHeight * textScale * 0.5f;
    auto top = elementSize;
    auto bottom = (-elementSize) - (textLayout._xHeight * textScale);

    auto horizontalCentre = -textLayout._width * textScale * 0.5f;
    auto right = elementSize;
    auto left = (-elementSize) - (textLayout._width * textScale);

    for(const auto& glyph : textLayout._glyphs)
    {
        GPUGraphData::GlyphData glyphData;

        auto textureGlyph = _textLayoutResults._glyphs[glyph._index];

        glyphData._component = componentIndex;

        std::array<float, 2> baseOffset{0.0f, 0.0f};
        switch(textAlignment)
        {
        default:
        case TextAlignment::Right:  baseOffset = {{right,            verticalCentre}}; break;
        case TextAlignment::Left:   baseOffset = {{left,             verticalCentre}}; break;
        case TextAlignment::Centre: baseOffset = {{horizontalCentre, verticalCentre}}; break;
        case TextAlignment::Top:    baseOffset = {{horizontalCentre, top           }}; break;
        case TextAlignment::Bottom: baseOffset = {{horizontalCentre, bottom        }}; break;
        }

        glyphData._glyphOffset[0] = baseOffset[0] + (glyph._advance * textScale);
        glyphData._glyphOffset[1] = baseOffset[1] - ((textureGlyph._height + textureGlyph._ascent) * textScale);
        glyphData._glyphSize[0] = textureGlyph._width;
        glyphData._glyphSize[1] = textureGlyph._height;

        glyphData._textureCoord[0] = textureGlyph._u;
        glyphData._textureCoord[1] = textureGlyph._v;
        glyphData._textureLayer = textureGlyph._layer;

        glyphData._basePosition[0] = elementPosition.x();
        glyphData._basePosition[1] = elementPosition.y();
        glyphData._basePosition[2] = elementPosition.z();

        glyphData._color[0] = textColor.redF();
        glyphData._color[1] = textColor.greenF();
        glyphData._color[2] = textColor.blueF();

        gpuGraphData->_glyphData.push_back(glyphData);
    }
}

void GraphRenderer::updateGPUDataIfRequired()
{
    if(!_gpuDataRequiresUpdate)
        return;

    _gpuDataRequiresUpdate = false;

    std::unique_lock<std::recursive_mutex> nodePositionsLock(_graphModel->nodePositions().mutex());
    std::unique_lock<std::recursive_mutex> glyphMapLock(_glyphMap->mutex());

    int componentIndex = 0;

    auto& nodePositions = _graphModel->nodePositions();
    auto& nodeVisuals = _graphModel->nodeVisuals();
    auto& edgeVisuals = _graphModel->edgeVisuals();

    for(auto& gpuGraphData : _gpuGraphData)
        gpuGraphData.reset();

    NodeArray<QVector3D> scaledAndSmoothedNodePositions(_graphModel->graph());

    float textScale = u::pref("visuals/textSize").toFloat();
    auto textAlignment = static_cast<TextAlignment>(u::pref("visuals/textAlignment").toInt());
    auto textColor = Document::contrastingColorForBackground();
    auto showNodeText = static_cast<TextState>(u::pref("visuals/showNodeText").toInt());
    auto showEdgeText = static_cast<TextState>(u::pref("visuals/showEdgeText").toInt());
    auto edgeVisualType = static_cast<EdgeVisualType>(u::pref("visuals/edgeVisualType").toInt());

    for(auto& componentRendererRef : _componentRenderers)
    {
        GraphComponentRenderer* componentRenderer = componentRendererRef;
        if(!componentRenderer->visible())
            continue;

        const float NotFoundAlpha = 0.2f;

        for(auto nodeId : componentRenderer->nodeIds())
        {
            if(_hiddenNodes.get(nodeId))
                continue;

            const QVector3D nodePosition = nodePositions.getScaledAndSmoothed(nodeId);
            scaledAndSmoothedNodePositions[nodeId] = nodePosition;

            auto& nodeVisual = nodeVisuals[nodeId];

            // Create and Add NodeData
            GPUGraphData::NodeData nodeData;
            nodeData._position[0] = nodePosition.x();
            nodeData._position[1] = nodePosition.y();
            nodeData._position[2] = nodePosition.z();
            nodeData._component = componentIndex;
            nodeData._size = nodeVisual._size;
            nodeData._color[0] = nodeVisual._color.redF();
            nodeData._color[1] = nodeVisual._color.greenF();
            nodeData._color[2] = nodeVisual._color.blueF();

            QColor outlineColor = nodeVisual._state.testFlag(VisualFlags::Selected) ?
                Qt::white : Qt::black;

            nodeData._outlineColor[0] = outlineColor.redF();
            nodeData._outlineColor[1] = outlineColor.greenF();
            nodeData._outlineColor[2] = outlineColor.blueF();

            auto* gpuGraphData = gpuGraphDataForAlpha(componentRenderer->alpha(),
                nodeVisual._state.testFlag(VisualFlags::NotFound) ? NotFoundAlpha : 1.0f);

            if(gpuGraphData != nullptr)
                gpuGraphData->_nodeData.push_back(nodeData);

            if(showNodeText == TextState::Off || nodeVisual._state.testFlag(VisualFlags::NotFound))
                continue;

            if(showNodeText == TextState::Selected && !nodeVisual._state.testFlag(VisualFlags::Selected))
                continue;

            createGPUGlyphData(nodeVisual._text, textColor, textAlignment, textScale, nodeVisual._size, nodePosition, componentIndex, gpuGraphData);
        }

        for(auto& edge : componentRenderer->edges())
        {
            if(_hiddenEdges.get(edge->id()) || _hiddenNodes.get(edge->sourceId()) || _hiddenNodes.get(edge->targetId()))
                continue;

            const QVector3D& sourcePosition = scaledAndSmoothedNodePositions[edge->sourceId()];
            const QVector3D& targetPosition = scaledAndSmoothedNodePositions[edge->targetId()];

            auto& edgeVisual = edgeVisuals[edge->id()];

            GPUGraphData::EdgeData edgeData;
            edgeData._sourcePosition[0] = sourcePosition.x();
            edgeData._sourcePosition[1] = sourcePosition.y();
            edgeData._sourcePosition[2] = sourcePosition.z();
            edgeData._targetPosition[0] = targetPosition.x();
            edgeData._targetPosition[1] = targetPosition.y();
            edgeData._targetPosition[2] = targetPosition.z();
            edgeData._sourceSize = nodeVisuals[edge->sourceId()]._size;
            edgeData._targetSize = nodeVisuals[edge->targetId()]._size;
            edgeData._edgeType = static_cast<int>(edgeVisualType);
            edgeData._component = componentIndex;
            edgeData._size = edgeVisual._size;
            edgeData._color[0] = edgeVisual._color.redF();
            edgeData._color[1] = edgeVisual._color.greenF();
            edgeData._color[2] = edgeVisual._color.blueF();

            edgeData._outlineColor[0] = 0.0f;
            edgeData._outlineColor[1] = 0.0f;
            edgeData._outlineColor[2] = 0.0f;

            auto* gpuGraphData = gpuGraphDataForAlpha(componentRenderer->alpha(),
                edgeVisual._state.testFlag(VisualFlags::NotFound) ? NotFoundAlpha : 1.0f);

            if(gpuGraphData != nullptr)
                gpuGraphData->_edgeData.push_back(edgeData);

            if(showEdgeText == TextState::Off || edgeVisual._state.testFlag(VisualFlags::NotFound))
                continue;

            if(showEdgeText == TextState::Selected && !edgeVisual._state.testFlag(VisualFlags::Selected))
                continue;

            QVector3D midPoint = (sourcePosition + targetPosition) * 0.5f;
            createGPUGlyphData(edgeVisual._text, textColor, textAlignment, textScale, edgeVisual._size, midPoint, componentIndex, gpuGraphData);
        }

        componentIndex++;
    }

    for(auto& gpuGraphData : _gpuGraphData)
    {
        if(gpuGraphData.alpha() > 0.0f)
            gpuGraphData.upload();
    }
}

void GraphRenderer::updateGPUData(GraphRenderer::When when)
{
    _gpuDataRequiresUpdate = true;

    if(when == When::Now)
        updateGPUDataIfRequired();
}

void GraphRenderer::updateComponentGPUData()
{
    //FIXME this doesn't necessarily need to be entirely regenerated and rebuffered
    // every frame, so it makes sense to do partial updates as and when required.
    // OTOH, it's probably not ever going to be masses of data, so maybe we should
    // just suck it up; need to get a profiler on it and see how long we're spending
    // here transfering the buffer, when there are lots of components
    std::vector<GLfloat> componentData;

    for(auto& componentRendererRef : _componentRenderers)
    {
        GraphComponentRenderer* componentRenderer = componentRendererRef;
        if(componentRenderer == nullptr)
        {
            qWarning() << "null component renderer";
            continue;
        }

        if(!componentRenderer->visible())
            continue;

        for(int i = 0; i < 16; i++)
            componentData.push_back(componentRenderer->modelViewMatrix().data()[i]);

        for(int i = 0; i < 16; i++)
            componentData.push_back(componentRenderer->projectionMatrix().data()[i]);
    }

    glBindBuffer(GL_TEXTURE_BUFFER, _componentDataTBO);
    glBufferData(GL_TEXTURE_BUFFER, componentData.size() * sizeof(GLfloat), componentData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void GraphRenderer::setScene(Scene* scene)
{
    if(_scene != nullptr)
    {
        _scene->setVisible(false);
        _scene->onHide();
    }

    _scene = scene;

    _scene->setVisible(true);
    _scene->onShow();

    _scene->setViewportSize(_width, _height);
}

GraphRenderer::Mode GraphRenderer::mode() const
{
    return _mode;
}

void GraphRenderer::setMode(Mode mode)
{
    if(mode != _mode)
    {
        _mode = mode;
        emit modeChanged();
    }
}

void GraphRenderer::resetTime()
{
    _time.start();
    _lastTime = 0.0f;
}

float GraphRenderer::secondsElapsed()
{
    float time = _time.elapsed() / 1000.0f;
    float dTime = time - _lastTime;
    _lastTime = time;

    return dTime;
}

bool GraphRenderer::transitionActive() const
{
    return _transition.active() || _scene->transitionActive();
}

void GraphRenderer::moveFocusToNode(NodeId nodeId, float cameraDistance)
{
    if(mode() == Mode::Component)
        _graphComponentScene->moveFocusToNode(nodeId, cameraDistance);
    else if(mode() == Mode::Overview)
    {
        // To focus on a node, we need to be in component mode
        auto componentId = _graphModel->graph().componentIdOfNode(nodeId);
        auto* newComponentRenderer = componentRendererForId(componentId);
        newComponentRenderer->moveFocusToNode(nodeId, cameraDistance);
        newComponentRenderer->saveViewData();
        newComponentRenderer->resetView();

        switchToComponentMode(true, componentId);
    }
}

void GraphRenderer::moveFocusToComponent(ComponentId componentId)
{
    if(mode() == Mode::Component)
        _graphComponentScene->setComponentId(componentId, true);
}

void GraphRenderer::rendererStartedTransition()
{
    if(!_transitionPotentiallyInProgress)
    {
        _transitionPotentiallyInProgress = true;

        emit userInteractionStarted();
        resetTime();
    }
}

void GraphRenderer::rendererFinishedTransition()
{
    if(transitionActive())
        return;

    _transitionPotentiallyInProgress = false;
    emit userInteractionFinished();
}

void GraphRenderer::sceneFinishedTransition()
{
    clearHiddenElements();
    updateGPUData(When::Later);
}

void GraphRenderer::executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description)
{
    _preUpdateExecutor.enqueue(std::move(task), description);
    emit taskAddedToExecutor();
}

// Stop the renderer thread executing any tasks until...
void GraphRenderer::pauseRendererThreadExecution()
{
    _preUpdateExecutor.pause();
}

// ...this is called
void GraphRenderer::resumeRendererThreadExecution()
{
    _preUpdateExecutor.resume();
}

bool GraphRenderer::visible() const
{
    return _graphOverviewScene->visible() || _graphComponentScene->visible();
}

void GraphRenderer::onNodeAdded(const Graph*, NodeId nodeId)
{
    _hiddenNodes.set(nodeId, true);
}

void GraphRenderer::onEdgeAdded(const Graph*, EdgeId edgeId)
{
    _hiddenEdges.set(edgeId, true);
}

void GraphRenderer::onNodeAddedToComponent(const Graph*, NodeId nodeId, ComponentId)
{
    _hiddenNodes.set(nodeId, true);
}

void GraphRenderer::onEdgeAddedToComponent(const Graph*, EdgeId edgeId, ComponentId)
{
    _hiddenEdges.set(edgeId, true);
}

void GraphRenderer::finishTransitionToOverviewMode(bool doTransition)
{
    setMode(GraphRenderer::Mode::Overview);
    setScene(_graphOverviewScene);
    setInteractor(_graphOverviewInteractor);

    if(doTransition)
    {
        // When we first change to overview mode we want all
        // the renderers to be in their reset state
        for(auto componentId : _graphModel->graph().componentIds())
        {
            auto renderer = componentRendererForId(componentId);
            renderer->resetView();
        }

        _graphOverviewScene->resetView(false);
        _graphOverviewScene->startTransitionFromComponentMode(_graphComponentScene->componentId());
    }

    updateGPUData(When::Later);
}

void GraphRenderer::finishTransitionToOverviewModeOnRendererThread(bool doTransition)
{
    setMode(GraphRenderer::Mode::Overview);
    executeOnRendererThread([this, doTransition]
    {
        finishTransitionToOverviewMode(doTransition);
    }, "GraphRenderer::finishTransitionToOverviewMode");
}

void GraphRenderer::finishTransitionToComponentMode(bool doTransition)
{
    setMode(GraphRenderer::Mode::Component);
    setScene(_graphComponentScene);
    setInteractor(_graphComponentInteractor);

    if(doTransition)
    {
        // Go back to where we were before
        _graphComponentScene->startTransition();
        _graphComponentScene->restoreViewData();
    }

    updateGPUData(When::Later);
}

void GraphRenderer::finishTransitionToComponentModeOnRendererThread(bool doTransition)
{
    setMode(GraphRenderer::Mode::Component);
    executeOnRendererThread([this, doTransition]
    {
        finishTransitionToComponentMode(doTransition);
    }, "GraphRenderer::finishTransitionToComponentMode");
}

void GraphRenderer::switchToOverviewMode(bool doTransition)
{
    executeOnRendererThread([this, doTransition]
    {
        // Refuse to switch to overview mode if there is nothing to display
        if(_graphModel->graph().numComponents() <= 1)
            return;

        // So that we can return to the current view parameters later
        _graphComponentScene->saveViewData();

        if(mode() != GraphRenderer::Mode::Overview && doTransition &&
           _graphComponentScene->componentRenderer() != nullptr)
        {
            if(!_graphComponentScene->viewIsReset())
            {
                _graphComponentScene->startTransition([this]
                {
                    sceneFinishedTransition();
                    _transition.willBeImmediatelyReused();
                    finishTransitionToOverviewModeOnRendererThread(true);
                });

                _graphComponentScene->resetView(false);
            }
            else
                finishTransitionToOverviewModeOnRendererThread(true);
        }
        else
            finishTransitionToOverviewMode(false);

    }, "GraphRenderer::switchToOverviewMode");
}

void GraphRenderer::switchToComponentMode(bool doTransition, ComponentId componentId)
{
    executeOnRendererThread([this, componentId, doTransition]
    {
        _graphComponentScene->setComponentId(componentId);

        if(mode() != GraphRenderer::Mode::Component && doTransition)
        {
            _graphOverviewScene->startTransitionToComponentMode(_graphComponentScene->componentId(),
            [this]
            {
                if(!_graphComponentScene->savedViewIsReset())
                {
                    _transition.willBeImmediatelyReused();
                    finishTransitionToComponentModeOnRendererThread(true);
                }
                else
                    finishTransitionToComponentModeOnRendererThread(false);
            });
        }
        else
            finishTransitionToComponentMode(false);

    }, "GraphRenderer::switchToComponentMode");
}

void GraphRenderer::onGraphWillChange(const Graph* graph)
{
    pauseRendererThreadExecution();

    // Hide any graph elements that are merged; they aren't displayed normally,
    // but during scene transitions they may become unmerged, and we don't want
    // to show them until the scene transition is over
    for(NodeId nodeId : graph->nodeIds())
    {
        if(graph->typeOf(nodeId) == MultiElementType::Tail)
            _hiddenNodes.set(nodeId, true);
    }

    for(EdgeId edgeId : graph->edgeIds())
    {
        if(graph->typeOf(edgeId) == MultiElementType::Tail)
            _hiddenEdges.set(edgeId, true);
    }
}

void GraphRenderer::onGraphChanged(const Graph* graph, bool changed)
{
    if(!changed)
        return;

    _numComponents = graph->numComponents();

    // We may not, in fact, subsequently actually start a transition here, but we
    // speculatively pretend we do so that if a command is currently in progress
    // there is some overlap between it and the renderer transition. This ensures
    // that Document::idle() returns false for the duration of the transaction.
    if(visible())
        rendererStartedTransition();

    executeOnRendererThread([this]
    {
        for(ComponentId componentId : _graphModel->graph().componentIds())
        {
            componentRendererForId(componentId)->initialise(_graphModel, componentId,
                                                            _selectionManager, this);
        }
        updateGPUData(When::Later);
    }, "GraphRenderer::onGraphChanged update");
}

void GraphRenderer::onComponentAdded(const Graph*, ComponentId componentId, bool)
{
    // If the component is entirely new, we shouldn't be hiding any of it
    auto* component = _graphModel->graph().componentById(componentId);
    for(NodeId nodeId : component->nodeIds())
        _hiddenNodes.set(nodeId, false);
    for(EdgeId edgeId : component->edgeIds())
        _hiddenEdges.set(edgeId, false);

    executeOnRendererThread([this, componentId]
    {
        componentRendererForId(componentId)->initialise(_graphModel, componentId,
                                                        _selectionManager, this);
    }, "GraphRenderer::onComponentAdded");
}

void GraphRenderer::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool)
{
    executeOnRendererThread([this, componentId]
    {
        componentRendererForId(componentId)->cleanup();
    }, QString("GraphRenderer::onComponentWillBeRemoved (cleanup) component %1").arg(static_cast<int>(componentId)));
}

void GraphRenderer::onPreferenceChanged(const QString& key, const QVariant& value)
{
    if(key == "visuals/textFont")
    {
        _glyphMap->setFontName(value.toString());
        updateText();
    }
}

void GraphRenderer::onCommandWillExecute(const ICommand*)
{
    disableSceneUpdate();
}

void GraphRenderer::onCommandCompleted(const ICommand*, const QString&)
{
    enableSceneUpdate();
    update();
}

void GraphRenderer::onLayoutChanged()
{
    _layoutChanged = true;
}

void GraphRenderer::onComponentAlphaChanged(ComponentId)
{
    updateGPUData(When::Later);
}

void GraphRenderer::onComponentCleanup(ComponentId)
{
    updateGPUData(When::Later);
}

void GraphRenderer::onVisibilityChanged()
{
    updateGPUData(When::Later);
}

GLuint GraphRenderer::sdfTextureCurrent() const
{
    return _sdfTextures.at(_currentSDFTextureIndex);
}

GLuint GraphRenderer::sdfTextureOffscreen() const
{
    return _sdfTextures[1 - _currentSDFTextureIndex];
}

void GraphRenderer::swapSdfTexture()
{
    _currentSDFTextureIndex = 1 - _currentSDFTextureIndex;
}

void GraphRenderer::updateText(bool waitForCompletion)
{
    std::unique_lock<std::recursive_mutex> glyphMapLock(_glyphMap->mutex());

    for(auto nodeId : _graphModel->graph().nodeIds())
        _glyphMap->addText(_graphModel->nodeVisuals()[nodeId]._text);

    for(auto edgeId : _graphModel->graph().edgeIds())
        _glyphMap->addText(_graphModel->edgeVisuals()[edgeId]._text);

    if(_glyphMap->updateRequired())
    {
        auto job = std::make_unique<SDFComputeJob>(sdfTextureOffscreen(), _glyphMap);
        job->executeWhenComplete([this]
        {
            executeOnRendererThread([this]
            {
                swapSdfTexture();
                _textLayoutResults = _glyphMap->results();

                updateGPUData(When::Later);
                update(); // QQuickFramebufferObject::Renderer::update
            }, "GraphRenderer::updateText");
        });

        _gpuComputeThread->enqueue(job);

        glyphMapLock.unlock();
        if(waitForCompletion)
            _gpuComputeThread->wait();
    }
}

void GraphRenderer::resetView()
{
    if(_scene != nullptr)
        _scene->resetView();
}

bool GraphRenderer::viewIsReset() const
{
    if(_scene != nullptr)
        return _scene->viewIsReset();

    return true;
}

static void setShaderADSParameters(QOpenGLShaderProgram& program)
{
    struct Light
    {
        Light() = default;
        Light(const QVector4D& _position, const QVector3D& _intensity) :
            position(_position), intensity(_intensity)
        {}

        QVector4D position;
        QVector3D intensity;
    };

    std::vector<Light> lights;
    lights.emplace_back(QVector4D(-20.0f, 0.0f, 3.0f, 1.0f), QVector3D(0.6f, 0.6f, 0.6f));
    lights.emplace_back(QVector4D(0.0f, 0.0f, 0.0f, 1.0f), QVector3D(0.2f, 0.2f, 0.2f));
    lights.emplace_back(QVector4D(10.0f, -10.0f, -10.0f, 1.0f), QVector3D(0.4f, 0.4f, 0.4f));

    auto numberOfLights = static_cast<int>(lights.size());

    program.setUniformValue("numberOfLights", numberOfLights);

    for(int i = 0; i < numberOfLights; i++)
    {
        QByteArray positionId = QString("lights[%1].position").arg(i).toLatin1();
        program.setUniformValue(positionId.data(), lights[i].position);

        QByteArray intensityId = QString("lights[%1].intensity").arg(i).toLatin1();
        program.setUniformValue(intensityId.data(), lights[i].intensity);
    }

    program.setUniformValue("material.ks", QVector3D(1.0f, 1.0f, 1.0f));
    program.setUniformValue("material.ka", QVector3D(0.02f, 0.02f, 0.02f));
    program.setUniformValue("material.shininess", 50.0f);
}

std::vector<int> GraphRenderer::gpuGraphDataRenderOrder() const
{
    std::vector<int> renderOrder;

    for(int i = 0; i < static_cast<int>(_gpuGraphData.size()); i++)
        renderOrder.push_back(i);

    std::sort(renderOrder.begin(), renderOrder.end(), [this](auto a, auto b)
    {
        if(_gpuGraphData.at(a)._alpha1 == _gpuGraphData.at(b)._alpha1)
            return _gpuGraphData.at(a)._alpha2 > _gpuGraphData.at(b)._alpha2;

        return _gpuGraphData.at(a)._alpha1 > _gpuGraphData.at(b)._alpha1;
    });

    while(!renderOrder.empty() && _gpuGraphData.at(renderOrder.back()).alpha() <= 0.0f)
        renderOrder.pop_back();

    return renderOrder;
}

void GraphRenderer::renderNodes(GPUGraphData& gpuGraphData)
{
    _nodesShader.bind();
    setShaderADSParameters(_nodesShader);

    gpuGraphData._nodeVBO.bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, _componentDataTexture);
    _nodesShader.setUniformValue("componentData", 0);

    gpuGraphData._sphere.vertexArrayObject()->bind();
    glDrawElementsInstanced(GL_TRIANGLES, gpuGraphData._sphere.indexCount(),
                            GL_UNSIGNED_INT, nullptr, gpuGraphData.numNodes());
    gpuGraphData._sphere.vertexArrayObject()->release();

    glBindTexture(GL_TEXTURE_BUFFER, 0);
    gpuGraphData._nodeVBO.release();
    _nodesShader.release();
}

void GraphRenderer::renderEdges(GPUGraphData& gpuGraphData)
{
    _edgesShader.bind();
    setShaderADSParameters(_edgesShader);

    gpuGraphData._edgeVBO.bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, _componentDataTexture);
    _edgesShader.setUniformValue("componentData", 0);

    gpuGraphData._arrow.vertexArrayObject()->bind();
    glDrawElementsInstanced(GL_TRIANGLES, gpuGraphData._arrow.indexCount(),
                            GL_UNSIGNED_INT, nullptr, gpuGraphData.numEdges());
    gpuGraphData._arrow.vertexArrayObject()->release();

    glBindTexture(GL_TEXTURE_BUFFER, 0);
    gpuGraphData._edgeVBO.release();
    _edgesShader.release();
}

void GraphRenderer::renderText(GPUGraphData& gpuGraphData)
{
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    // Enable per-sample shading, this makes small text look nice
    glEnable(GL_SAMPLE_SHADING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    _textShader.bind();
    gpuGraphData._textVBO.bind();

     // Bind SDF textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sdfTextureCurrent());

    // Set to linear filtering for SDF text
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    _textShader.setUniformValue("tex", 0);
    _textShader.setUniformValue("textScale", u::pref("visuals/textSize").toFloat());

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_BUFFER, _componentDataTexture);
    _textShader.setUniformValue("componentData", 1);

    gpuGraphData._rectangle.vertexArrayObject()->bind();
    glDrawElementsInstanced(GL_TRIANGLES, gpuGraphData._rectangle.indexCount(),
                            GL_UNSIGNED_INT, nullptr, static_cast<int>(gpuGraphData._glyphData.size())) ;
    gpuGraphData._rectangle.vertexArrayObject()->release();

    glBindTexture(GL_TEXTURE_BUFFER, 0);

    gpuGraphData._textVBO.release();
    _textShader.release();
    glDisable(GL_BLEND);
    glDisable(GL_SAMPLE_SHADING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void GraphRenderer::renderGraph(GPUGraphData& gpuGraphData)
{
    glBindFramebuffer(GL_FRAMEBUFFER, gpuGraphData._fbo);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, static_cast<GLenum*>(drawBuffers));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderNodes(gpuGraphData);
    renderEdges(gpuGraphData);
    renderText(gpuGraphData);
}

void GraphRenderer::renderScene()
{
    ifSceneUpdateEnabled([this]
    {
        _preUpdateExecutor.execute();
    });

    if(_scene == nullptr)
        return;

    if(_resized)
    {
        _scene->setViewportSize(_width, _height);
        _resized = false;
    }

    ifSceneUpdateEnabled([this]
    {
        // _synchronousLayoutChanged can only ever be (atomically) true in this scope
        _synchronousLayoutChanged = _layoutChanged.exchange(false);

        // If there is a transition active then we'll need another
        // frame once we're finished with this one
        if(transitionActive())
            update(); // QQuickFramebufferObject::Renderer::update

        float dTime = secondsElapsed();
        _transition.update(dTime);
        _scene->update(dTime);

        if(layoutChanged())
            updateGPUData(When::Later);

        updateGPUDataIfRequired();
        updateComponentGPUData();

        _synchronousLayoutChanged = false;
    });

    if(!_FBOcomplete)
    {
        qWarning() << "Attempting to render without a complete FBO";
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    for(auto& gpuGraphData : _gpuGraphData)
        gpuGraphData.clearFramebuffer();

    for(auto i : gpuGraphDataRenderOrder())
    {
        auto& gpuGraphData = _gpuGraphData.at(i);

        // Clear the depth buffer, but only when we're about to render graph elements
        // that are found, so that subsequent render passes of not found elements
        // use the existing depth information
        if(gpuGraphData._alpha2 >= 1.0f)
            gpuGraphData.clearDepthbuffer();

        renderGraph(gpuGraphData);
    }
}

void GraphRenderer::render2D()
{
    auto& gpuGraphData = _gpuGraphData[0];

    if(gpuGraphData.unused())
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, gpuGraphData._fbo);

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, _width, _height);

    QMatrix4x4 m;
    m.ortho(0.0f, _width, 0.0f, _height, -1.0f, 1.0f);

    if(!_selectionRect.isNull())
    {
        const QColor color(Qt::white);

        QRect r;
        r.setLeft(_selectionRect.left());
        r.setRight(_selectionRect.right());
        r.setTop(_height - _selectionRect.top());
        r.setBottom(_height - _selectionRect.bottom());

        std::vector<GLfloat> quadData;

        quadData.push_back(r.left()); quadData.push_back(r.bottom());
        quadData.push_back(color.redF()); quadData.push_back(color.blueF()); quadData.push_back(color.greenF());
        quadData.push_back(r.right()); quadData.push_back(r.bottom());
        quadData.push_back(color.redF()); quadData.push_back(color.blueF()); quadData.push_back(color.greenF());
        quadData.push_back(r.right()); quadData.push_back(r.top());
        quadData.push_back(color.redF()); quadData.push_back(color.blueF()); quadData.push_back(color.greenF());

        quadData.push_back(r.right()); quadData.push_back(r.top());
        quadData.push_back(color.redF()); quadData.push_back(color.blueF()); quadData.push_back(color.greenF());
        quadData.push_back(r.left());  quadData.push_back(r.top());
        quadData.push_back(color.redF()); quadData.push_back(color.blueF()); quadData.push_back(color.greenF());
        quadData.push_back(r.left());  quadData.push_back(r.bottom());
        quadData.push_back(color.redF()); quadData.push_back(color.blueF()); quadData.push_back(color.greenF());

        glDrawBuffer(GL_COLOR_ATTACHMENT1);

        _selectionMarkerDataBuffer.bind();
        _selectionMarkerDataBuffer.allocate(quadData.data(), static_cast<int>(quadData.size()) * sizeof(GLfloat));

        _selectionMarkerShader.bind();
        _selectionMarkerShader.setUniformValue("projectionMatrix", m);

        _selectionMarkerDataVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        _selectionMarkerDataVAO.release();

        _selectionMarkerShader.release();
        _selectionMarkerDataBuffer.release();
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
}

QOpenGLFramebufferObject* GraphRenderer::createFramebufferObject(const QSize& size)
{
    // Piggy back our FBO resize on to Qt's
    resize(size.width(), size.height());

    return new QOpenGLFramebufferObject(size);
}

void GraphRenderer::render()
{
    if(!_FBOcomplete)
    {
        qWarning() << "Attempting to render incomplete FBO";
        return;
    }

    glViewport(0, 0, _width, _height);

    renderScene();
    render2D();
    finishRender();

    std::unique_lock<std::mutex> lock(_resetOpenGLStateMutex);
    resetOpenGLState();

    _performanceCounter.tick();
}

void GraphRenderer::synchronize(QQuickFramebufferObject* item)
{
    if(!resetOpenGLState)
    {
        resetOpenGLState = [item]
        {
            if(item->window() != nullptr)
                item->window()->resetOpenGLState();
        };

        connect(item, &QObject::destroyed, this, [this]
        {
            std::unique_lock<std::mutex> lock(_resetOpenGLStateMutex);
            resetOpenGLState = []{};
        }, Qt::DirectConnection);
    }

    auto graphQuickItem = qobject_cast<GraphQuickItem*>(item);

    if(graphQuickItem->viewResetPending())
        resetView();

    if(graphQuickItem->overviewModeSwitchPending())
        switchToOverviewMode();

    NodeId focusNodeId = graphQuickItem->desiredFocusNodeId();
    ComponentId focusComponentId = graphQuickItem->desiredFocusComponentId();

    if(!focusNodeId.isNull())
        moveFocusToNode(focusNodeId, GraphComponentRenderer::COMFORTABLE_ZOOM_DISTANCE);
    else if(!focusComponentId.isNull())
    {
        if(mode() == Mode::Overview)
            switchToComponentMode(true, focusComponentId);
        else
            moveFocusToComponent(focusComponentId);
    }

    ifSceneUpdateEnabled([this, &graphQuickItem]
    {
        if(_scene != nullptr)
        {
            //FIXME try delivering these events by queued connection instead
            while(graphQuickItem->eventsPending())
            {
                auto e = graphQuickItem->nextEvent();
                auto mouseEvent = dynamic_cast<QMouseEvent*>(e.get());
                auto wheelEvent = dynamic_cast<QWheelEvent*>(e.get());
                auto nativeGestureEvent = dynamic_cast<QNativeGestureEvent*>(e.get());

                switch(e->type())
                {
                case QEvent::Type::MouseButtonPress:    _interactor->mousePressEvent(mouseEvent);               break;
                case QEvent::Type::MouseButtonRelease:  _interactor->mouseReleaseEvent(mouseEvent);             break;
                case QEvent::Type::MouseMove:           _interactor->mouseMoveEvent(mouseEvent);                break;
                case QEvent::Type::MouseButtonDblClick: _interactor->mouseDoubleClickEvent(mouseEvent);         break;
                case QEvent::Type::Wheel:               _interactor->wheelEvent(wheelEvent);                    break;
                case QEvent::Type::NativeGesture:       _interactor->nativeGestureEvent(nativeGestureEvent);    break;
                default: break;
                }
            }
        }
    });

    // Tell the QuickItem what we're doing
    graphQuickItem->setViewIsReset(viewIsReset());
    graphQuickItem->setCanEnterOverviewMode(mode() != Mode::Overview && _numComponents > 1);
    graphQuickItem->setInOverviewMode(mode() == Mode::Overview);

    ComponentId focusedComponentId = mode() != Mode::Overview ? _graphComponentScene->componentId() : ComponentId();
    graphQuickItem->setFocusedComponentId(focusedComponentId);
}

void GraphRenderer::render2DComposite(QOpenGLShaderProgram& shader, GLuint texture, float alpha)
{
    shader.bind();
    shader.setUniformValue("alpha", alpha);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    shader.release();
}

void GraphRenderer::finishRender()
{
    if(!framebufferObject()->bind())
        qWarning() << "QQuickFrameBufferobject::Renderer FBO not bound";

    glViewport(0, 0, framebufferObject()->width(), framebufferObject()->height());

    auto backgroundColor = u::pref("visuals/backgroundColor").value<QColor>();

    glClearColor(backgroundColor.redF(),
                 backgroundColor.greenF(),
                 backgroundColor.blueF(), 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    QMatrix4x4 m;
    m.ortho(0, _width, 0, _height, -1.0f, 1.0f);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    glEnable(GL_BLEND);

    _screenShader.bind();
    _screenShader.setUniformValue("projectionMatrix", m);
     _screenShader.release();

    _selectionShader.bind();
    _selectionShader.setUniformValue("projectionMatrix", m);
    _selectionShader.setUniformValue("highlightColor",
                                     u::pref("visuals/highlightColor").value<QColor>());
    _selectionShader.release();

    _screenQuadDataBuffer.bind();
    _screenQuadVAO.bind();

    for(auto i : gpuGraphDataRenderOrder())
    {
        render2DComposite(_screenShader,    _gpuGraphData[i]._colorTexture,     _gpuGraphData[i].alpha());
        render2DComposite(_selectionShader, _gpuGraphData[i]._selectionTexture, _gpuGraphData[i].alpha());
    }

    _screenQuadDataBuffer.release();
    _screenQuadVAO.release();
}

GraphComponentRenderer* GraphRenderer::componentRendererForId(ComponentId componentId) const
{
    if(componentId.isNull())
        return nullptr;

    GraphComponentRenderer* renderer = _componentRenderers.at(componentId);
    Q_ASSERT(renderer != nullptr);
    return renderer;
}

void GraphRenderer::prepareSDFTextures()
{
    // Generate SDF textures
    if(_sdfTextures[0] == 0)
        glGenTextures(2, &_sdfTextures[0]);
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
    _screenShader.setUniformValue("multisamples", NUM_MULTISAMPLES);
    _screenShader.release();

    _sdfShader.bind();
    _sdfShader.enableAttributeArray("position");
    _sdfShader.setAttributeBuffer("position", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    _sdfShader.release();

    _selectionShader.bind();
    _selectionShader.enableAttributeArray("position");
    _selectionShader.setAttributeBuffer("position", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    _selectionShader.setUniformValue("frameBufferTexture", 0);
    _selectionShader.setUniformValue("multisamples", NUM_MULTISAMPLES);
    _selectionShader.release();

    _screenQuadDataBuffer.release();
    _screenQuadVAO.release();
}

void GraphRenderer::prepareComponentDataTexture()
{
    if(_componentDataTexture == 0)
        glGenTextures(1, &_componentDataTexture);

    if(_componentDataTBO == 0)
        glGenBuffers(1, &_componentDataTBO);

    glBindTexture(GL_TEXTURE_BUFFER, _componentDataTexture);
    glBindBuffer(GL_TEXTURE_BUFFER, _componentDataTBO);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _componentDataTBO);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void GraphRenderer::enableSceneUpdate()
{
    std::unique_lock<std::mutex> lock(_sceneUpdateMutex);
    Q_ASSERT(_sceneUpdateDisabled > 0);
    _sceneUpdateDisabled--;
    resetTime();
}

void GraphRenderer::disableSceneUpdate()
{
    std::unique_lock<std::mutex> lock(_sceneUpdateMutex);
    _sceneUpdateDisabled++;
}

void GraphRenderer::ifSceneUpdateEnabled(const std::function<void()>& f)
{
    std::unique_lock<std::mutex> lock(_sceneUpdateMutex);
    if(!_sceneUpdateDisabled)
        f();
}

void GraphRenderer::clearHiddenElements()
{
    _hiddenNodes.resetElements();
    _hiddenEdges.resetElements();
}
