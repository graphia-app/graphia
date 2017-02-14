#ifndef GRAPHRENDERER_H
#define GRAPHRENDERER_H

#include "opengldebuglogger.h"
#include "openglfunctions.h"
#include "graphcomponentrenderer.h"
#include "transition.h"
#include "glyphmap.h"

#include "primitives/arrow.h"
#include "primitives/sphere.h"
#include "primitives/rectangle.h"

#include "shared/graph/grapharray.h"

#include "shared/utils/movablepointer.h"
#include "shared/utils/deferredexecutor.h"
#include "shared/utils/performancecounter.h"

#include "utils/qmlenum.h"

#include <QObject>
#include <QTime>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QQuickFramebufferObject>
#include <array>

#include <functional>
#include <memory>
#include <atomic>
#include <vector>
#include <QImage>

class GraphQuickItem;
class GraphModel;
class CommandManager;
class SelectionManager;
class GPUComputeThread;
class QOpenGLDebugMessage;

class Scene;
class GraphOverviewScene;
class GraphComponentScene;

class Interactor;
class GraphOverviewInteractor;
class GraphComponentInteractor;

class ICommand;

class GraphInitialiser
{
public:
    virtual ~GraphInitialiser() {}

    void initialiseFromGraph(const Graph* graph);

protected:
    virtual void onGraphChanged(const Graph*, bool) = 0;
    virtual void onComponentAdded(const Graph*, ComponentId, bool) = 0;
};

struct GPUGraphData : OpenGLFunctions
{
    GPUGraphData();
    virtual ~GPUGraphData();

    void initialise(QOpenGLShaderProgram& nodesShader,
                    QOpenGLShaderProgram& edgesShader,
                    QOpenGLShaderProgram& textShader);
    void prepareVertexBuffers();
    void prepareNodeVAO(QOpenGLShaderProgram& shader);
    void prepareEdgeVAO(QOpenGLShaderProgram& shader);
    void prepareTextVAO(QOpenGLShaderProgram &shader);

    bool prepareRenderBuffers(int width, int height, GLuint depthTexture);

    void reset();
    void clearFramebuffer();
    void clearDepthbuffer();

    void upload();

    int numNodes() const;
    int numEdges() const;

    Primitive::Sphere _sphere;
    Primitive::Arrow _arrow;
    Primitive::Rectangle _rectangle;

    float alpha() const;

    bool unused() const;

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
        float _sourceSize;
        float _targetSize;
        int _edgeType;
        int _component;
        float _size;
        float _color[3];
        float _outlineColor[3];
    };

    struct GlyphData
    {
        int _component;
        float _textureCoord[2];
        int _textureLayer;
        float _basePosition[3];
        float _glyphOffset[2];
        float _glyphSize[2];
        float _color[3];
    };

    // There are two alpha values so that we can split the alpha blended layers
    // depending on their purpose. The rendering occurs in order based on _alpha1,
    // going from opaque to transparent, then resorting to _alpha2 in the same order,
    // when the values of _alpha1 match
    float _alpha1 = 0.0f;
    float _alpha2 = 0.0f;

    std::vector<NodeData> _nodeData;
    QOpenGLBuffer _nodeVBO;

    std::vector<GlyphData> _glyphData;
    QOpenGLBuffer _textVBO;

    std::vector<EdgeData> _edgeData;
    QOpenGLBuffer _edgeVBO;

    GLuint _fbo = 0;
    GLuint _colorTexture = 0;
    GLuint _selectionTexture = 0;
};

enum class TextAlignment
{
    Right,
    Left,
    Centre,
    Top,
    Bottom
};
Q_DECLARE_METATYPE(TextAlignment)

DEFINE_QML_ENUM(Q_GADGET, NodeTextState,
                Off, Selected, All);
DEFINE_QML_ENUM(Q_GADGET, EdgeVisualType,
                Cylinder, Arrow);

class GraphRenderer :
        public QObject,
        public OpenGLFunctions,
        public GraphInitialiser,
        public QQuickFramebufferObject::Renderer
{
    Q_OBJECT

    friend class GraphComponentRenderer;

public:
    GraphRenderer(const std::shared_ptr<GraphModel>& graphModel,
                  CommandManager& commandManager,
                  const std::shared_ptr<SelectionManager>& selectionManager,
                  const std::shared_ptr<GPUComputeThread>& gpuComputeThread);
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
    void setTextColor(QColor textColor);

    void switchToOverviewMode(bool doTransition = true);
    void switchToComponentMode(bool doTransition = true, ComponentId componentId = ComponentId());
    void rendererStartedTransition();
    void rendererFinishedTransition();
    void sceneFinishedTransition();
    void executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description);
    void pauseRendererThreadExecution();
    void resumeRendererThreadExecution();

    bool layoutChanged() const { return _synchronousLayoutChanged; }

    bool visible() const;
private slots:
    void onNodeAdded(const Graph*, NodeId nodeId);
    void onEdgeAdded(const Graph*, EdgeId edgeId);
    void onNodeAddedToComponent(const Graph*, NodeId nodeId, ComponentId);
    void onEdgeAddedToComponent(const Graph*, EdgeId edgeId, ComponentId);

    void onGraphWillChange(const Graph* graph);
    void onGraphChanged(const Graph* graph, bool changed);
    void onComponentAdded(const Graph*, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool);

public slots:
    void onCommandWillExecute(const ICommand*);
    void onCommandCompleted(const ICommand*, const QString&);
    void onLayoutChanged();
    void onPreferenceChanged(const QString& key, const QVariant& value);
    void onComponentAlphaChanged(ComponentId componentId);
    void onComponentCleanup(ComponentId componentId);
    void onVisibilityChanged();

private:
    std::shared_ptr<GraphModel> _graphModel;
    int _numComponents = 0;

    std::shared_ptr<SelectionManager> _selectionManager;

    std::shared_ptr<GPUComputeThread> _gpuComputeThread;
    std::shared_ptr<GlyphMap> _glyphMap;

    // Store a copy of the text layout results as its computation is a long running
    // process that occurs in a separate thread; we don't want to be rendering from
    // a set of results that is currently changing
    GlyphMap::Results _textLayoutResults;

    // It's important that these are pointers and not values, because the array will
    // be resized during ComponentManager::update, and we still want to be
    // able to use the existing renderers while this occurs. If the array stored
    // values, then the storage for the renderers themselves would potentially be
    // moved around, as opposed to just the storage for the pointers.
    ComponentArray<MovablePointer<GraphComponentRenderer>, u::Locking> _componentRenderers;
    bool _transitionPotentiallyInProgress = false;
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
    QOpenGLShaderProgram _sdfShader;

    QOpenGLShaderProgram _nodesShader;
    QOpenGLShaderProgram _edgesShader;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLShaderProgram _textShader;
    QOpenGLShaderProgram _debugLinesShader;

    int _width = 0;
    int _height = 0;
    bool _resized = false;

    GLuint _colorTexture = 0;

    // The SDF textures are double buffered to prevent flicker when updating
    int _currentSDFTextureIndex = 0;
    std::array<GLuint, 2> _sdfTextures = {};

    GLuint _depthTexture = 0;

    bool _FBOcomplete = false;

    QOpenGLVertexArrayObject _screenQuadVAO;
    QOpenGLBuffer _screenQuadDataBuffer;

    QOpenGLBuffer _selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject _selectionMarkerDataVAO;

    // When elements are added to the scene, it may be that they would lie
    // outside the confines of where they should be rendered, until a transition
    // is completed, so these arrays allow us to hide the elements until such
    // transitions are complete
    NodeArray<bool> _hiddenNodes;
    EdgeArray<bool> _hiddenEdges;

    std::array<GPUGraphData, 6> _gpuGraphData;
    bool _gpuDataRequiresUpdate = false;

    GLuint _componentDataTexture = 0;
    GLuint _componentDataTBO = 0;

    QRect _selectionRect;

    QTime _time;
    float _lastTime = 0.0f;
    int _sceneUpdateDisabled = 1;
    std::mutex _sceneUpdateMutex;

    std::atomic<bool> _layoutChanged;
    bool _synchronousLayoutChanged = false;

    Transition _transition;

    PerformanceCounter _performanceCounter;

    void prepareSDFTextures();
    void prepareSelectionMarkerVAO();
    void prepareQuad();

    GLuint sdfTextureCurrent() const;
    GLuint sdfTextureOffscreen() const;
    void swapSdfTexture();

    void updateText(bool waitForCompletion = false);

    void prepareComponentDataTexture();

    void enableSceneUpdate();
    void disableSceneUpdate();
    void ifSceneUpdateEnabled(const std::function<void()>& f);

    void clearHiddenElements();

    GPUGraphData* gpuGraphDataForAlpha(float alpha1, float alpha2);
    void updateGPUDataIfRequired();
    enum class When { Later, Now };
    void updateGPUData(When when);
    void updateComponentGPUData();

    void resize(int width, int height);

    void render2DComposite(QOpenGLShaderProgram& shader, GLuint texture, float alpha);
    void finishRender();

    std::vector<int> gpuGraphDataRenderOrder() const;

    void renderNodes(GPUGraphData& gpuGraphData);
    void renderEdges(GPUGraphData& gpuGraphData);
    void renderText(GPUGraphData& gpuGraphData);
    void renderGraph(GPUGraphData& gpuGraphData);
    void renderScene();
    void render2D();

    QOpenGLFramebufferObject* createFramebufferObject(const QSize &size);
    void render();
    void synchronize(QQuickFramebufferObject* item);

    std::mutex _resetOpenGLStateMutex;
    std::function<void()> resetOpenGLState;

    void finishTransitionToOverviewMode(bool doTransition);
    void finishTransitionToOverviewModeOnRendererThread(bool doTransition);
    void finishTransitionToComponentMode(bool doTransition);
    void finishTransitionToComponentModeOnRendererThread(bool doTransition);

    void setScene(Scene* scene);
    void setInteractor(Interactor* interactor) { _interactor = interactor; }

    Mode mode() const;
    void setMode(Mode mode);

    void resetTime();
    float secondsElapsed();

    bool transitionActive() const;

    void moveFocusToNode(NodeId nodeId, float cameraDistance = -1.0f);
    void moveFocusToComponent(ComponentId componentId);

    void createGPUGlyphData(const QString& text, const QColor& textColor, const TextAlignment& textAlignment,
                         float textScale, float elementSize, const QVector3D& elementPosition,
                         int componentIndex, GPUGraphData* gpuGraphData);

signals:
    void modeChanged() const;

    void userInteractionStarted() const;
    void userInteractionFinished() const;

    void taskAddedToExecutor() const;

    void fpsChanged(float fps) const;
};

#endif // GRAPHRENDERER_H
