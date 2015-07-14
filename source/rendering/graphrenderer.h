#ifndef GRAPHRENDERER_H
#define GRAPHRENDERER_H

#include "opengldebuglogger.h"
#include "openglfunctions.h"
#include "graphcomponentrenderer.h"
#include "transition.h"

#include "../graph/grapharray.h"

#include "../utils/movablepointer.h"
#include "../utils/deferredexecutor.h"

#include <QObject>
#include <QTime>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QQuickFramebufferObject>

#include <functional>
#include <memory>
#include <atomic>

class GraphQuickItem;
class GraphModel;
class CommandManager;
class SelectionManager;
class QOpenGLDebugMessage;

class Scene;
class GraphOverviewScene;
class GraphComponentScene;

class Interactor;
class GraphOverviewInteractor;
class GraphComponentInteractor;

class Command;

class GraphInitialiser
{
public:
    void initialiseFromGraph(const Graph* graph);

protected:
    virtual void onGraphChanged(const Graph*) = 0;
    virtual void onComponentAdded(const Graph*, ComponentId, bool) = 0;
};

class GraphRenderer :
        public QObject,
        protected OpenGLFunctions,
        public GraphInitialiser,
        public QQuickFramebufferObject::Renderer
{
    Q_OBJECT

    friend class GraphComponentRenderer;

public:
    GraphRenderer(std::shared_ptr<GraphModel> graphModel,
                  CommandManager& commandManager,
                  std::shared_ptr<SelectionManager> selectionManager);
    virtual ~GraphRenderer();

    static const int NUM_MULTISAMPLES = 4; //FIXME pass to screen.frag

    ComponentArray<MovablePointer<GraphComponentRenderer>>& componentRenderers() { return _componentRenderers; }
    GraphComponentRenderer* componentRendererForId(ComponentId componentId) const;
    Transition& transition() { return _transition; }

    std::shared_ptr<GraphModel> graphModel() { return _graphModel; }

    QRect selectionRect() { return _selectionRect; }
    void setSelectionRect(const QRect& selectionRect) { _selectionRect = selectionRect; }
    void clearSelectionRect() { _selectionRect = QRect(); }

    void resetView();
    bool viewIsReset() const;

    void switchToOverviewMode(bool doTransition = true);
    void switchToComponentMode(bool doTransition = true, ComponentId componentId = ComponentId());
    void rendererStartedTransition();
    void rendererFinishedTransition();
    void executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description);

private slots:
    void onGraphChanged(const Graph* graph);
    void onComponentAdded(const Graph*, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool);
    void onSelectionChanged(const SelectionManager*);

public slots:
    void onCommandWillExecuteAsynchronously(const Command*);
    void onCommandCompleted(const Command*, const QString&);

private:
    std::shared_ptr<GraphModel> _graphModel;
    int _numComponents;

    std::shared_ptr<SelectionManager> _selectionManager;

    // It's important that these are pointers and not values, because the array will
    // be resized during ComponentManager::updateComponents, and we still want to be
    // able to use the existing renderers while this occurs. If the array stored
    // values, then the storage for the renderers themselves would potentially be
    // moved around, as opposed to just the storage for the pointers.
    ComponentArray<MovablePointer<GraphComponentRenderer>> _componentRenderers;
    int _numTransitioningRenderers;
    DeferredExecutor _preUpdateExecutor;

    enum class Mode
    {
        Overview,
        Component
    };

    Mode _mode;

    Scene* _scene;
    GraphOverviewScene* _graphOverviewScene;
    GraphComponentScene* _graphComponentScene;

    Interactor* _interactor;
    GraphOverviewInteractor* _graphOverviewInteractor;
    GraphComponentInteractor* _graphComponentInteractor;

    OpenGLDebugLogger _openGLDebugLogger;

    QOpenGLShaderProgram _screenShader;
    QOpenGLShaderProgram _selectionShader;

    QOpenGLShaderProgram _nodesShader;
    QOpenGLShaderProgram _edgesShader;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLShaderProgram _debugLinesShader;

    int _width;
    int _height;
    bool _resized;

    GLuint _colorTexture;
    GLuint _selectionTexture;
    GLuint _depthTexture;
    GLuint _visualFBO;
    bool _FBOcomplete;

    QOpenGLVertexArrayObject _screenQuadVAO;
    QOpenGLBuffer _screenQuadDataBuffer;

    QOpenGLBuffer _selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject _selectionMarkerDataVAO;

    QRect _selectionRect;

    QTime _time;
    std::atomic<bool> _sceneUpdateEnabled;

    bool _modeTransitionInProgress;

    Transition _transition;

    bool prepareRenderBuffers(int width, int height);
    void prepareSelectionMarkerVAO();
    void prepareQuad();

    void resize(int width, int height);

    void clear();
    void finishRender();
    void renderScene();
    void render2D();

    QOpenGLFramebufferObject* createFramebufferObject(const QSize &size);
    void render();
    void synchronize(QQuickFramebufferObject* item);

    std::function<void()> resetOpenGLState;

    void finishTransitionToOverviewMode();
    void finishTransitionToComponentMode();
    void setScene(Scene* scene);
    void setInteractor(Interactor* interactor) { _interactor = interactor; }

    Mode mode() const;
    void setMode(Mode mode);

signals:
    void modeChanged() const;

    void userInteractionStarted() const;
    void userInteractionFinished() const;

    void taskAddedToExecutor() const;
};

#endif // GRAPHRENDERER_H
