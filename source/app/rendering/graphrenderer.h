/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRAPHRENDERER_H
#define GRAPHRENDERER_H

#include "opengldebuglogger.h"
#include "openglfunctions.h"
#include "graphrenderercore.h"
#include "graphcomponentrenderer.h"
#include "transition.h"
#include "glyphmap.h"
#include "doublebufferedtexture.h"
#include "projection.h"
#include "shading.h"

#include "shared/graph/elementid_containers.h"
#include "shared/graph/grapharray.h"
#include "graph/qmlelementid.h"

#include "shared/utils/movablepointer.h"
#include "shared/utils/deferredexecutor.h"
#include "shared/utils/performancecounter.h"

#include "app/preferenceswatcher.h"

#include "shared/utils/qmlenum.h"

#include <QObject>
#include <QElapsedTimer>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QQuickFramebufferObject>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QSize>

#include <functional>
#include <memory>
#include <atomic>
#include <vector>
#include <array>
#include <queue>

class Graph;
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

class ICommand;

DEFINE_QML_ENUM(
    Q_GADGET, TextAlignment,
    Right, Left, Centre,
    Top, Bottom);
DEFINE_QML_ENUM(
    Q_GADGET, TextState,
    Off, Selected, All, Focused);
DEFINE_QML_ENUM(
    Q_GADGET, EdgeVisualType,
    Cylinder, Arrow);

template<typename Target>
void initialiseFromGraph(const Graph*, Target&); // NOLINT

class GraphRenderer :
        public QObject,
        public GraphRendererCore,
        public QQuickFramebufferObject::Renderer
{
    Q_OBJECT

    friend class GraphComponentRenderer;
    friend class ScreenshotRenderer;
    friend void initialiseFromGraph<GraphRenderer>(const Graph*, GraphRenderer&);

public:
    GraphRenderer(GraphModel* graphModel, CommandManager* commandManager, SelectionManager* selectionManager);
    ~GraphRenderer() override;

    ComponentArray<MovablePointer<GraphComponentRenderer>, LockingGraphArray>& componentRenderers()
    {
        return _componentRenderers;
    }

    const ComponentArray<MovablePointer<GraphComponentRenderer>, LockingGraphArray>&
    componentRenderers() const
    {
        return _componentRenderers;
    }

    GraphComponentRenderer* componentRendererForId(ComponentId componentId) const;
    Transition& transition() { return _transition; }

    GraphModel* graphModel() const { return _graphModel; }

    QRect selectionRect() { return _selectionRect; }
    void setSelectionRect(const QRect& selectionRect) { _selectionRect = selectionRect; }
    void clearSelectionRect() { _selectionRect = QRect(); }

    void resetView();
    bool viewIsReset() const;
    void setTextColor(QColor textColor);

    void switchToOverviewMode(bool doTransition = true,
        const std::vector<ComponentId>& focusComponentIds = {});
    void switchToComponentMode(bool doTransition = true,
        ComponentId componentId = {}, NodeId nodeId = {}, float radius = -1.0f);
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
    void onNodeMovedBetweenComponents(const Graph*, NodeId nodeId, ComponentId, ComponentId);
    void onEdgeMovedBetweenComponents(const Graph*, EdgeId edgeId, ComponentId, ComponentId);

    void onGraphWillChange(const Graph* graph);
    void onGraphChanged(const Graph* graph, bool changed);
    void onComponentAdded(const Graph*, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool);

public slots:
    void onScreenshotRequested(int width, int height, const QString& path, int dpi, bool fillSize);
    void onCommandsStarted();
    void onCommandsFinished();
    void onLayoutChanged();
    void onPreferenceChanged(const QString& key, const QVariant& value);
    void onComponentAlphaChanged(ComponentId componentId);
    void onComponentCleanup(ComponentId componentId);
    void onVisibilityChanged();

    void onPreviewRequested(int width, int height, bool fillSize);

private:
    GraphModel* _graphModel = nullptr;
    size_t _numComponents = 0;

    SelectionManager* _selectionManager = nullptr;

    PreferencesWatcher _preferencesWatcher;

    std::unique_ptr<GlyphMap> _glyphMap;

    // Store a copy of the text layout results as its computation is a long running
    // process that occurs in a separate thread; we don't want to be rendering from
    // a set of results that is currently changing
    GlyphMap::Results _textLayoutResults;

    // It's important that these are pointers and not values, because the array will
    // be resized during ComponentManager::update, and we still want to be
    // able to use the existing renderers while this occurs. If the array stored
    // values, then the storage for the renderers themselves would potentially be
    // moved around, as opposed to just the storage for the pointers.
    ComponentArray<MovablePointer<GraphComponentRenderer>, LockingGraphArray> _componentRenderers;
    bool _transitionPotentiallyInProgress = false;
    DeferredExecutor _preUpdateExecutor;

    std::queue<std::unique_ptr<QEvent>> _eventQueue;

    enum class Mode
    {
        Overview,
        Component
    };

    Mode _mode = Mode::Overview;

    Projection _projection = Projection::Perspective;

    Scene* _scene = nullptr;
    GraphOverviewScene* _graphOverviewScene;
    GraphComponentScene* _graphComponentScene;

    Interactor* _interactor = nullptr;
    GraphOverviewInteractor* _graphOverviewInteractor;
    GraphComponentInteractor* _graphComponentInteractor;

    OpenGLDebugLogger _openGLDebugLogger;

    QOpenGLShaderProgram _debugLinesShader;

    bool _resized = false;

    GLuint _colorTexture = 0;

    DoubleBufferedTexture _sdfTexture;
    QSize _sdfTextureSize;

    std::mutex _initialisationMutex;

    bool _FBOcomplete = false;

    // When elements are added to the scene, it may be that they would lie
    // outside the confines of where they should be rendered, until a transition
    // is completed, so these arrays allow us to hide the elements until such
    // transitions are complete
    NodeArray<bool> _hiddenNodes;
    EdgeArray<bool> _hiddenEdges;

    bool _gpuDataRequiresUpdate = false;

    QRect _selectionRect;

    QElapsedTimer _time;
    float _lastTime = 0.0f;
    int _sceneUpdateDisabled = 1;
    mutable std::recursive_mutex _sceneUpdateMutex;

    std::atomic_bool _layoutChanged;
    bool _synchronousLayoutChanged = false;

    Transition _transition;

    PerformanceCounter _performanceCounter;
    std::unique_ptr<ScreenshotRenderer> _screenshotRenderer;

    GLuint sdfTexture() const override;
    QSize sdfTextureSize() const;
    void swapSdfTexture();

    void updateText();

    void enableSceneUpdate();
    void disableSceneUpdate();
    void ifSceneUpdateEnabled(const std::function<void()>& f) const;

    void clearHiddenElements();

    void updateGPUDataIfRequired();
    enum class When { Later, Now };
    void updateGPUData(When when);
    void updateComponentGPUData();

    // For high DPI displays (mostly MacOS "Retina" display)
    qreal _devicePixelRatio = 1.0;

    void processEventQueue();

    void updateScene();

    Mode bestFocusParameters(GraphQuickItem* graphQuickItem, ComponentIdSet& componentIds,
        NodeId& focusNodeId, float& radius) const;

    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override;
    void render() override;
    void synchronize(QQuickFramebufferObject* item) override;

    void finishTransitionToOverviewMode(bool doTransition, const std::vector<ComponentId>& focusComponentIds);
    void finishTransitionToOverviewModeOnRendererThread(bool doTransition, const std::vector<ComponentId>& focusComponentIds);
    void finishTransitionToComponentMode(bool doTransition);
    void finishTransitionToComponentModeOnRendererThread(bool doTransition);

    void setScene(Scene* scene);
    void setInteractor(Interactor* interactor) { _interactor = interactor; }

    Mode mode() const;
    void setMode(Mode mode);

    Projection projection() const;
    void setProjection(Projection projection);

    void resetTime();
    float secondsElapsed();

    bool transitionActive() const;

    void moveFocusToNode(NodeId nodeId, float radius = -1.0f);
    void moveFocusToComponent(ComponentId componentId);
    void moveFocusToComponents(const std::vector<ComponentId>& componentIds);

    void createGPUGlyphData(const QString& text, const QColor& textColor, const TextAlignment& textAlignment,
                         float textScale, float elementSize, const QVector3D& elementPosition,
                         int componentIndex, GPUGraphData* gpuGraphData);

signals:
    void initialised();

    void modeChanged();

    void synchronizeComplete();

    void userInteractionStarted();
    void userInteractionFinished();

    void transitionStarted();
    void transitionFinished();

    void taskAddedToExecutor();
    // Base64 encoded png image for QML...
    void previewComplete(QString previewBase64);
    // Screenshot doesn't go to QML so we can use QImage
    void screenshotComplete(const QImage& screenshot, const QString& path);

    void fpsChanged(float fps);

    void clicked(int button, int modifiers, QmlNodeId nodeId);
};

#endif // GRAPHRENDERER_H
