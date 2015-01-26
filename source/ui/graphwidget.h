#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include "../rendering/graphcomponentrenderersreference.h"

#include "../graph/grapharray.h"

#include "../utils/deferredexecutor.h"

#include <QWidget>
#include <QTimer>
#include <QString>

#include <vector>
#include <memory>

class GraphComponentRendererShared;
class GraphComponentRenderer;
class GraphScene;
class GraphInteractor;
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
    void toggleModes();
    void switchToOverviewMode();
    void switchToComponentMode(ComponentId componentId = ComponentId());

    void rendererStartedTransition();
    void rendererFinishedTransition();

    void executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description = QString());
    void updateNodePositions();

private:
    bool _initialised;

    OpenGLWindow* _openGLWindow;
    QTimer* _timer;

    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<SelectionManager> _selectionManager;

    std::shared_ptr<GraphComponentRendererShared> _graphComponentRendererShared;
    std::shared_ptr<ComponentArray<GraphComponentRendererManager>> _graphComponentRendererManagers;
    int _numTransitioningRenderers;
    DeferredExecutor _preUpdateExecutor;

    GraphScene* _graphScene;
    GraphInteractor* _graphInteractor;

    GraphComponentScene* _graphComponentScene;
    GraphComponentInteractor* _graphComponentInteractor;

    Mode _mode;
    ComponentId _defaultComponentId;

private slots:
    void onGraphChanged(const Graph* graph);
    void onNodeWillBeRemoved(const Graph* graph, NodeId nodeId);
    void onComponentAdded(const Graph*, ComponentId componentId);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId);

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
