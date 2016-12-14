#ifndef GRAPHQUICKITEM_H
#define GRAPHQUICKITEM_H

#include "graph/graph.h"
#include "rendering/compute/gpucomputethread.h"

#include <QQuickFramebufferObject>
#include <QTimer>
#include <QString>
#include <QEvent>

#include <vector>
#include <queue>
#include <memory>

class GraphModel;
class Command;
class CommandManager;
class SelectionManager;

class GraphQuickItem : public QQuickFramebufferObject
{
    Q_OBJECT

    Q_PROPERTY(bool interacting MEMBER _interacting NOTIFY interactingChanged)
    Q_PROPERTY(bool viewIsReset MEMBER _viewIsReset NOTIFY viewIsResetChanged)
    Q_PROPERTY(bool canEnterOverviewMode MEMBER _canEnterOverviewMode NOTIFY canEnterOverviewModeChanged)

    Q_PROPERTY(int numNodes READ numNodes NOTIFY graphChanged)
    Q_PROPERTY(int numVisibleNodes READ numVisibleNodes NOTIFY graphChanged)
    Q_PROPERTY(int numEdges READ numEdges NOTIFY graphChanged)
    Q_PROPERTY(int numVisibleEdges READ numVisibleEdges NOTIFY graphChanged)
    Q_PROPERTY(int numComponents READ numComponents NOTIFY graphChanged)

    Q_PROPERTY(float fps READ fps NOTIFY fpsChanged)

public:
    explicit GraphQuickItem(QQuickItem* parent = nullptr);

    void initialise(std::shared_ptr<GraphModel> graphModel,
                    CommandManager& commandManager,
                    std::shared_ptr<SelectionManager> selectionManager,
                    std::shared_ptr<GPUComputeThread> gpuComputeThread);

    std::shared_ptr<GraphModel> graphModel() { return _graphModel; }
    std::shared_ptr<SelectionManager> selectionManager() { return _selectionManager; }

    void resetView();
    bool viewResetPending();

    bool interacting() const;
    void setInteracting(bool interacting) const;
    bool viewIsReset() const;
    bool canEnterOverviewMode() const;

    void switchToOverviewMode(bool doTransition = true);
    bool overviewModeSwitchPending();

    void moveFocusToNode(NodeId nodeId);
    NodeId desiredFocusNodeId();

    void moveFocusToComponent(ComponentId componentId);
    ComponentId desiredFocusComponentId();
    ComponentId focusedComponentId() const;

    Renderer* createRenderer() const;

    bool eventsPending();
    std::unique_ptr<QEvent> nextEvent();

    float fps() const { return _fps; }

    // These are only called by GraphRenderer so that it can tell
    // interested parties what it's doing
    void setViewIsReset(bool viewIsReset);
    void setCanEnterOverviewMode(bool canEnterOverviewMode);
    void setFocusedComponentId(ComponentId componentId);

private:
    bool event(QEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<GPUComputeThread> _gpuComputeThread;
    CommandManager* _commandManager = nullptr;
    std::shared_ptr<SelectionManager> _selectionManager;

    bool _viewResetPending = false;
    bool _overviewModeSwitchPending = false;
    NodeId _desiredFocusNodeId;
    ComponentId _desiredFocusComponentId;

    mutable bool _interacting = false;
    bool _viewIsReset = true;
    bool _canEnterOverviewMode = false;
    ComponentId _focusedComponentId;

    std::queue<std::unique_ptr<QEvent>> _eventQueue;

    mutable float _fps = 0.0f;

    template<typename T> void enqueueEvent(const T* event)
    {
        _eventQueue.emplace(std::make_unique<T>(*event));
        update();
    }

    int numNodes() const;
    int numVisibleNodes() const;
    int numEdges() const;
    int numVisibleEdges() const;
    int numComponents() const;

public slots:
    void onLayoutChanged();
    void onFPSChanged(int fps);
    void onUserInteractionStarted();
    void onUserInteractionFinished();

signals:
    void interactingChanged() const;
    void viewIsResetChanged() const;
    void canEnterOverviewModeChanged() const;
    void focusedComponentIdChanged() const;

    void commandWillExecute(const Command* command) const;
    void commandCompleted(const Command* command, const QString& pastParticiple) const;

    void layoutChanged() const;

    void graphChanged() const;

    void fpsChanged() const;
};

#endif // GRAPHQUICKITEM_H
