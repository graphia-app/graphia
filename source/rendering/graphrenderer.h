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
#include <vector>

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
    virtual ~GraphInitialiser() {}

    void initialiseFromGraph(const Graph* graph);

protected:
    virtual void onGraphChanged(const Graph*) = 0;
    virtual void onComponentAdded(const Graph*, ComponentId, bool) = 0;
};

struct GPUGraphData : protected OpenGLFunctions
{
    GPUGraphData();

    void initialise(QOpenGLShaderProgram& nodesShader,
                    QOpenGLShaderProgram& edgesShader);
    void prepareVertexBuffers();
    void prepareNodeVAO(QOpenGLShaderProgram& shader);
    void prepareEdgeVAO(QOpenGLShaderProgram& shader);

    void reset();

    void upload();

    int numNodes() const;
    int numEdges() const;

    Sphere _sphere;
    Cylinder _cylinder;

    struct NodeData
    {
        float _position[3];
        int _component;
        float _size;
        float _color[3];
        float _outlineColor[3];
    };

    struct EdgeData
    {
        float _sourcePosition[3];
        float _targetPosition[3];
        int _component;
        float _size;
        float _color[3];
        float _outlineColor[3];
    };

    std::vector<NodeData> _nodeData;
    QOpenGLBuffer _nodeVBO;

    std::vector<EdgeData> _edgeData;
    QOpenGLBuffer _edgeVBO;
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

    static const int NUM_MULTISAMPLES = 4;

    ComponentArray<MovablePointer<GraphComponentRenderer>, u::Locking>& componentRenderers() { return _componentRenderers; }
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
    void sceneFinishedTransition();
    void executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description);

    bool layoutChanged() const { return _synchronousLayoutChanged; }

private slots:
    void onNodeAdded(const Graph*, NodeId nodeId);
    void onEdgeAdded(const Graph*, EdgeId edgeId);
    void onNodeAddedToComponent(const Graph*, NodeId nodeId, ComponentId);
    void onEdgeAddedToComponent(const Graph*, EdgeId edgeId, ComponentId);

    void onGraphChanged(const Graph* graph);
    void onComponentAdded(const Graph*, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool);
    void onSelectionChanged(const SelectionManager*);

    void onPreferenceChanged(const QString& key, const QVariant& value);

public slots:
    void onCommandWillExecuteAsynchronously(const Command*);
    void onCommandCompleted(const Command*, const QString&);
    void onLayoutChanged();
    void onComponentFadingChanged(ComponentId componentId);
    void onComponentCleanup(ComponentId componentId);

private:
    std::shared_ptr<GraphModel> _graphModel;
    int _numComponents = 0;

    std::shared_ptr<SelectionManager> _selectionManager;

    // It's important that these are pointers and not values, because the array will
    // be resized during ComponentManager::updateComponents, and we still want to be
    // able to use the existing renderers while this occurs. If the array stored
    // values, then the storage for the renderers themselves would potentially be
    // moved around, as opposed to just the storage for the pointers.
    ComponentArray<MovablePointer<GraphComponentRenderer>, u::Locking> _componentRenderers;
    int _numTransitioningRenderers = 0;
    DeferredExecutor _preUpdateExecutor;

    enum class Mode
    {
        Overview,
        Component
    };

    Mode _mode = Mode::Overview;

    Scene* _scene = nullptr;
    GraphOverviewScene* _graphOverviewScene;
    GraphComponentScene* _graphComponentScene;

    Interactor* _interactor = nullptr;
    GraphOverviewInteractor* _graphOverviewInteractor;
    GraphComponentInteractor* _graphComponentInteractor;

    OpenGLDebugLogger _openGLDebugLogger;

    QOpenGLShaderProgram _screenShader;
    QOpenGLShaderProgram _selectionShader;

    QOpenGLShaderProgram _nodesShader;
    QOpenGLShaderProgram _edgesShader;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLShaderProgram _debugLinesShader;

    int _width = 0;
    int _height = 0;
    bool _resized = false;

    GLuint _colorTexture = 0;
    GLuint _selectionTexture = 0;
    GLuint _depthTexture = 0;
    GLuint _visualFBO = 0;
    bool _FBOcomplete = false;

    QOpenGLVertexArrayObject _screenQuadVAO;
    QOpenGLBuffer _screenQuadDataBuffer;

    QOpenGLBuffer _selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject _selectionMarkerDataVAO;

    NodeArray<bool> _hiddenNodes;
    EdgeArray<bool> _hiddenEdges;

    GPUGraphData _gpuGraphData;
    GPUGraphData _gpuGraphDataAlpha;
    bool _gpuDataRequiresUpdate = false;

    GLuint _componentDataTexture = 0;
    GLuint _componentDataTBO = 0;

    QRect _selectionRect;

    QTime _time;
    float _lastTime = 0.0f;
    bool _sceneUpdateEnabled = false;
    std::mutex _sceneUpdateMutex;

    std::atomic<bool> _layoutChanged;
    bool _synchronousLayoutChanged = false;

    bool _modeTransitionInProgress = false;

    Transition _transition;

    bool prepareRenderBuffers(int width, int height);
    void prepareSelectionMarkerVAO();
    void prepareQuad();

    void prepareComponentDataTexture();

    void enableSceneUpdate();
    void disableSceneUpdate();
    void ifSceneUpdateEnabled(const std::function<void()>& f);

    void clearHiddenElements();

    void updateGPUDataIfRequired();
    enum class When { Later, Now };
    void updateGPUData(When when);
    void updateComponentGPUData();

    void resize(int width, int height);

    void clear();
    void finishRender();

    void renderNodes(GPUGraphData& gpuGraphData);
    void renderEdges(GPUGraphData& gpuGraphData);
    void renderGraph(GPUGraphData& gpuGraphData);
    void renderScene();
    void render2D();

    QOpenGLFramebufferObject* createFramebufferObject(const QSize &size);
    void render();
    void synchronize(QQuickFramebufferObject* item);

    std::mutex _resetOpenGLStateMutex;
    std::function<void()> resetOpenGLState;

    void finishTransitionToOverviewMode();
    void finishTransitionToComponentMode();
    void finishModeTransition();
    void setScene(Scene* scene);
    void setInteractor(Interactor* interactor) { _interactor = interactor; }

    Mode mode() const;
    void setMode(Mode mode);

    void resetTime();
    float secondsElapsed();

    bool transitionActive() const;

signals:
    void modeChanged() const;

    void userInteractionStarted() const;
    void userInteractionFinished() const;

    void taskAddedToExecutor() const;
};

#endif // GRAPHRENDERER_H
