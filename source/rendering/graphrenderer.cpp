#include "graphrenderer.h"

#include "graphcomponentscene.h"
#include "graphoverviewscene.h"

#include "../ui/graphcomponentinteractor.h"
#include "../ui/graphoverviewinteractor.h"

#include "../graph/graphmodel.h"

#include "../ui/graphquickitem.h"
#include "../ui/selectionmanager.h"

#include "../commands/command.h"

#include "../utils/cpp1x_hacks.h"

#include <QObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLDebugLogger>
#include <QColor>
#include <QQuickWindow>
#include <QEvent>
#include <QNativeGestureEvent>

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

void GraphInitialiser::initialiseFromGraph(const Graph *graph)
{
    for(auto componentId : graph->componentIds())
        onComponentAdded(graph, componentId, false);

    onGraphChanged(graph);
}

GraphRenderer::GraphRenderer(std::shared_ptr<GraphModel> graphModel,
                             CommandManager& commandManager,
                             std::shared_ptr<SelectionManager> selectionManager) :
    QObject(),
    OpenGLFunctions(),
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _componentRenderers(_graphModel->graph()),
    _sceneUpdateEnabled(true)
{
    resolveOpenGLFunctions();

    loadShaderProgram(_screenShader, ":/shaders/screen.vert", ":/shaders/screen.frag");
    loadShaderProgram(_selectionShader, ":/shaders/screen.vert", ":/shaders/selection.frag");

    loadShaderProgram(_nodesShader, ":/shaders/instancednodes.vert", ":/shaders/ads.frag");
    loadShaderProgram(_edgesShader, ":/shaders/instancededges.vert", ":/shaders/ads.frag");

    loadShaderProgram(_selectionMarkerShader, ":/shaders/2d.vert", ":/shaders/selectionMarker.frag");
    loadShaderProgram(_debugLinesShader, ":/shaders/debuglines.vert", ":/shaders/debuglines.frag");

    prepareSelectionMarkerVAO();
    prepareQuad();

    auto graph = &_graphModel->graph();

    connect(graph, &Graph::graphChanged, this, &GraphRenderer::onGraphChanged, Qt::DirectConnection);
    connect(graph, &Graph::componentAdded, this, &GraphRenderer::onComponentAdded, Qt::DirectConnection);
    connect(graph, &Graph::componentWillBeRemoved, this, &GraphRenderer::onComponentWillBeRemoved, Qt::DirectConnection);

    _graphOverviewScene = new GraphOverviewScene(this);
    _graphComponentScene = new GraphComponentScene(this);

    _graphOverviewInteractor = new GraphOverviewInteractor(_graphModel, _graphOverviewScene, commandManager, _selectionManager, this);
    _graphComponentInteractor = new GraphComponentInteractor(_graphModel, _graphComponentScene, commandManager, _selectionManager, this);

    initialiseFromGraph(graph);
    _graphOverviewScene->initialiseFromGraph(graph);
    _graphComponentScene->initialiseFromGraph(graph);

    if(graph->componentIds().size() == 1)
        switchToComponentMode(false);
    else
        switchToOverviewMode(false);

    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &GraphRenderer::onSelectionChanged, Qt::DirectConnection);

    _time.start();
}

GraphRenderer::~GraphRenderer()
{
    if(_visualFBO != 0)
    {
        glDeleteFramebuffers(1, &_visualFBO);
        _visualFBO = 0;
    }

    _FBOcomplete = false;

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

    if(_depthTexture != 0)
    {
        glDeleteTextures(1, &_depthTexture);
        _depthTexture = 0;
    }
}

void GraphRenderer::resize(int width, int height)
{
    _width = width;
    _height = height;
    _resized = true;

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
    glBindFramebuffer(GL_FRAMEBUFFER, _visualFBO);

    // Color buffer
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Selection buffer
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GraphRenderer::setScene(Scene* scene)
{
    if(_scene != nullptr)
    {
        _scene->setVisible(false);
        _scene->onHide();
    }

    _scene = scene;

    if(!_scene->initialised())
    {
        _scene->initialise();
        _scene->setInitialised();
    }

    _scene->setVisible(true);
    _scene->onShow();

    _scene->setSize(_width, _height);
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

//FIXME this reference counting thing is rubbish, and gives rise to hacks
void GraphRenderer::rendererStartedTransition()
{
    Q_ASSERT(_numTransitioningRenderers >= 0);

    if(_numTransitioningRenderers == 0)
        emit userInteractionStarted();

    _numTransitioningRenderers++;
}

void GraphRenderer::rendererFinishedTransition()
{
    _numTransitioningRenderers--;

    Q_ASSERT(_numTransitioningRenderers >= 0);

    if(_numTransitioningRenderers == 0)
        emit userInteractionFinished();
}

void GraphRenderer::executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description)
{
    _preUpdateExecutor.enqueue(task, description);
    emit taskAddedToExecutor();
}

void GraphRenderer::finishTransitionToOverviewMode()
{
    setScene(_graphOverviewScene);
    setInteractor(_graphOverviewInteractor);

    if(_modeTransitionInProgress)
    {
        // When we first change to overview mode we want all
        // the renderers to be in their reset state
        for(auto componentId : _graphModel->graph().componentIds())
        {
            auto renderer = componentRendererForId(componentId);
            renderer->resetView();
        }

        _graphOverviewScene->startTransitionFromComponentMode(_graphComponentScene->componentId(),
                                                              0.3f, Transition::Type::EaseInEaseOut);
    }
}

void GraphRenderer::finishTransitionToComponentMode()
{
    setScene(_graphComponentScene);
    setInteractor(_graphComponentInteractor);

    if(!_graphComponentScene->savedViewIsReset())
    {
        // Go back to where we were before
        if(_modeTransitionInProgress)
            _graphComponentScene->startTransition(0.3f, Transition::Type::EaseInEaseOut);

        _graphComponentScene->restoreViewData();
    }
}

void GraphRenderer::switchToOverviewMode(bool doTransition)
{
    executeOnRendererThread([this, doTransition]
    {
        // So that we can return to the current view parameters later
        _graphComponentScene->saveViewData();

        if(mode() != GraphRenderer::Mode::Overview && doTransition)
        {
            if(!_graphComponentScene->viewIsReset())
            {
                if(!_transition.active())
                    rendererStartedTransition(); // Partner to * below

                _graphComponentScene->startTransition(0.3f, Transition::Type::EaseInEaseOut,
                [this]
                {
                    _modeTransitionInProgress = true;
                    setMode(GraphRenderer::Mode::Overview);
                    rendererFinishedTransition(); // *
                });

                _graphComponentScene->resetView();
            }
            else
            {
                _modeTransitionInProgress = true;
                setMode(GraphRenderer::Mode::Overview);
            }
        }
        else
        {
            setMode(GraphRenderer::Mode::Overview);
            finishTransitionToOverviewMode();
        }

    }, "GraphRenderer::switchToOverviewMode");
}

void GraphRenderer::switchToComponentMode(bool doTransition, ComponentId componentId)
{
    executeOnRendererThread([this, componentId, doTransition]
    {
        _graphComponentScene->setComponentId(componentId);

        if(mode() != GraphRenderer::Mode::Component && doTransition)
        {
            if(!_transition.active())
                rendererStartedTransition();

            _graphOverviewScene->startTransitionToComponentMode(_graphComponentScene->componentId(),
                                                                0.3f, Transition::Type::EaseInEaseOut,
            [this]
            {
                _modeTransitionInProgress = true;
                setMode(GraphRenderer::Mode::Component);
                rendererFinishedTransition();
            });
        }
        else
        {
            setMode(GraphRenderer::Mode::Component);
            finishTransitionToComponentMode();
        }

    }, "GraphRenderer::switchToComponentMode");
}

void GraphRenderer::onGraphChanged(const Graph* graph)
{
    _numComponents = graph->numComponents();

    for(auto componentId : graph->componentIds())
    {
        //FIXME: this makes me feel dirty
        // This is a slight hack to prevent there being a gap in which
        // layout can occur, inbetween the graph change and user
        // interaction phases
        rendererStartedTransition();

        executeOnRendererThread([this, componentId]
        {
            auto graphComponentRenderer = componentRendererForId(componentId);

            if(!graphComponentRenderer->initialised())
            {
                graphComponentRenderer->initialise(_graphModel, componentId,
                                                   _selectionManager,
                                                   this);
            }
            else
            {
                graphComponentRenderer->updateVisualData();
                graphComponentRenderer->updatePositionalData();
            }

            // Partner to the hack described above
            rendererFinishedTransition();
        }, QString("GraphRenderer::onGraphChanged (initialise/update) component %1").arg((int)componentId));
    }
}

void GraphRenderer::onComponentAdded(const Graph*, ComponentId componentId, bool)
{
    auto graphComponentRenderer = componentRendererForId(componentId);
    executeOnRendererThread([this, graphComponentRenderer, componentId]
    {
        graphComponentRenderer->initialise(_graphModel, componentId,
                                           _selectionManager,
                                           this);
    }, "GraphRenderer::onComponentAdded");
}

void GraphRenderer::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool)
{
    auto graphComponentRenderer = componentRendererForId(componentId);
    executeOnRendererThread([this, graphComponentRenderer]
    {
        graphComponentRenderer->cleanup();
    }, QString("GraphRenderer::onComponentWillBeRemoved (cleanup) component %1").arg((int)componentId));
}

void GraphRenderer::onSelectionChanged(const SelectionManager*)
{
    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto graphComponentRenderer = componentRendererForId(componentId);
        executeOnRendererThread([graphComponentRenderer]
        {
            graphComponentRenderer->updateVisualData();
        }, QString("GraphRenderer::onSelectionChanged component %1").arg((int)componentId));
    }
}

void GraphRenderer::onCommandWillExecuteAsynchronously(const Command*)
{
    _sceneUpdateEnabled = false;
}

void GraphRenderer::onCommandCompleted(const Command*, const QString&)
{
    _sceneUpdateEnabled = true;
    update();
}

void GraphRenderer::resetView()
{
    if(_graphComponentScene != nullptr && mode() == GraphRenderer::Mode::Component)
    {
        _graphComponentScene->startTransition();
        _graphComponentScene->resetView();
    }
}

bool GraphRenderer::viewIsReset() const
{
    if(_graphComponentScene != nullptr && mode() == GraphRenderer::Mode::Component)
        return _graphComponentScene->viewIsReset();

    return true;
}

void GraphRenderer::renderScene()
{
    if(_sceneUpdateEnabled)
    {
        if(_modeTransitionInProgress)
        {
            switch(mode())
            {
                case GraphRenderer::Mode::Overview:
                    finishTransitionToOverviewMode();
                    break;

                case GraphRenderer::Mode::Component:
                    finishTransitionToComponentMode();
                    break;

                default:
                    break;
            }

            _modeTransitionInProgress = false;
        }

        _preUpdateExecutor.execute();
    }

    if(_scene == nullptr || !_scene->initialised())
        return;

    if(_resized)
    {
        _scene->setSize(_width, _height);
        _resized = false;
    }

    if(_sceneUpdateEnabled)
    {
        _graphModel->nodePositions().executeIfUpdated([this]
        {
            for(auto componentId : _graphModel->graph().componentIds())
            {
                auto graphComponentRenderer = componentRendererForId(componentId);

                if(graphComponentRenderer->visible())
                    graphComponentRenderer->updatePositionalData();
            }
        });

        // If there is a transition active then we'll need another
        // frame once we're finished with this one
        if(_scene->transitionActive() || _modeTransitionInProgress)
            update();

        //FIXME should be passing a delta around?
        float time = _time.elapsed() / 1000.0f;
        _transition.update(time);
        _scene->update(time);
    }

    _scene->render();
}

void GraphRenderer::render2D()
{
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, _width, _height);

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

        glDrawBuffer(GL_COLOR_ATTACHMENT1);

        _selectionMarkerDataBuffer.bind();
        _selectionMarkerDataBuffer.allocate(data.data(), static_cast<int>(data.size()) * sizeof(GLfloat));

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

    clear();
    renderScene();
    render2D();
    finishRender();

    std::unique_lock<std::mutex>(_resetOpenGLStateMutex);
    resetOpenGLState();
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
            std::unique_lock<std::mutex>(_resetOpenGLStateMutex);
            resetOpenGLState = []{};
        }, Qt::DirectConnection);
    }

    auto graphQuickItem = qobject_cast<GraphQuickItem*>(item);

    if(graphQuickItem->viewResetPending())
        resetView();

    if(graphQuickItem->overviewModeSwitchPending())
        switchToOverviewMode();

    if(_scene != nullptr && _sceneUpdateEnabled)
    {
        //FIXME try delivering these events by queued connection instead
        while(graphQuickItem->eventsPending())
        {
            auto e = std::move(graphQuickItem->nextEvent());
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

    // Tell the QuickItem what we're doing
    graphQuickItem->setViewIsReset(viewIsReset());
    graphQuickItem->setCanEnterOverviewMode(mode() != Mode::Overview && _numComponents > 1);
}

void GraphRenderer::finishRender()
{
    if(!framebufferObject()->bind())
        qWarning() << "QQuickFrameBufferobject::Renderer FBO not bound";

    glViewport(0, 0, framebufferObject()->width(), framebufferObject()->height());

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    QMatrix4x4 m;
    m.ortho(0, _width, 0, _height, -1.0f, 1.0f);

    _screenQuadDataBuffer.bind();

    _screenQuadVAO.bind();
    glActiveTexture(GL_TEXTURE0);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    glEnable(GL_BLEND);

    // Color texture
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _colorTexture);

    _screenShader.bind();
    _screenShader.setUniformValue("projectionMatrix", m);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    _screenShader.release();

    // Selection texture
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture);

    _selectionShader.bind();
    _selectionShader.setUniformValue("projectionMatrix", m);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    _selectionShader.release();

    _screenQuadVAO.release();
    _screenQuadDataBuffer.release();
}

GraphComponentRenderer* GraphRenderer::componentRendererForId(ComponentId componentId) const
{
    if(componentId.isNull())
        return nullptr;

    return _componentRenderers.at(componentId);
}

bool GraphRenderer::prepareRenderBuffers(int width, int height)
{
    bool valid;

    // Color texture
    if(_colorTexture == 0)
        glGenTextures(1, &_colorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _colorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_MULTISAMPLES, GL_RGBA, width, height, GL_FALSE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Selection texture
    if(_selectionTexture == 0)
        glGenTextures(1, &_selectionTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_MULTISAMPLES, GL_RGBA, width, height, GL_FALSE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Depth texture
    if(_depthTexture == 0)
        glGenTextures(1, &_depthTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _depthTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_MULTISAMPLES, GL_DEPTH_COMPONENT, width, height, GL_FALSE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Visual FBO
    if(_visualFBO == 0)
        glGenFramebuffers(1, &_visualFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, _visualFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, _depthTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    valid = (status == GL_FRAMEBUFFER_COMPLETE);

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
