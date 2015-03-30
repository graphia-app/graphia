#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include "../rendering/graphcomponentrenderersreference.h"
#include "../rendering/transition.h"

#include "../graph/grapharray.h"

#include "../utils/deferredexecutor.h"

#include <QWidget>
#include <QTimer>
#include <QString>
#include <QRect>

#include <vector>
#include <memory>

class GraphRenderer;
class GraphComponentRenderer;
class GraphOverviewScene;
class GraphOverviewInteractor;
class GraphComponentScene;
class GraphComponentInteractor;
class GraphModel;
class Command;
class CommandManager;
class SelectionManager;
class OpenGLWindow;

class GraphWidget : public QWidget
{
    Q_OBJECT
public:
    GraphWidget(std::shared_ptr<GraphModel> graphModel,
                CommandManager& commandManager,
                std::shared_ptr<SelectionManager> selectionManager,
                QWidget *parent = nullptr);

    virtual ~GraphWidget();

    void initialise();

    bool interacting() const;

    void resetView();
    bool viewIsReset() const;

    std::shared_ptr<GraphModel> graphModel() { return _graphModel; }

    enum class Mode
    {
        Overview,
        Component
    };

    Mode mode() const { return _mode; }
    void switchToOverviewMode(bool doTransition = true);
    void switchToComponentMode(ComponentId componentId = ComponentId(), bool doTransition = true);

    void rendererStartedTransition();
    void rendererFinishedTransition();

    void executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description = QString());
    void updateNodePositions();

    Transition& transition() { return _transition; }

    void setSelectionRect(const QRect& rect);
    void clearSelectionRect();

    void resizeScene(int width, int height);
    void clearScene();
    void renderScene();

private:
    void makeContextCurrent();
    void finishTransitionToOverviewMode();
    void finishTransitionToComponentMode();

    bool _initialised;

    OpenGLWindow* _openGLWindow;
    QTimer* _timer;
    bool _sceneUpdateEnabled;

    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<SelectionManager> _selectionManager;

    std::shared_ptr<GraphRenderer> _graphRenderer;
    std::shared_ptr<ComponentArray<GraphComponentRendererManager>> _graphComponentRendererManagers;
    int _numTransitioningRenderers;
    DeferredExecutor _preUpdateExecutor;

    GraphOverviewScene* _graphOverviewScene;
    GraphOverviewInteractor* _graphOverviewInteractor;

    GraphComponentScene* _graphComponentScene;
    GraphComponentInteractor* _graphComponentInteractor;

    bool _modeTransitionInProgress;
    Mode _mode;
    ComponentId _defaultComponentId;

    Transition _transition;

private slots:
    void onGraphChanged(const Graph* graph);
    void onComponentAdded(const Graph*, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool);

    void onSelectionChanged(const SelectionManager*);

    void onUpdate();

public slots:
    void onCommandWillExecuteAsynchronously(const Command* command, const QString& verb);
    void onCommandCompleted(const Command* command);

signals:
    void userInteractionStarted() const;
    void userInteractionFinished() const;
    void layoutChanged() const;
};

#endif // GRAPHWIDGET_H
