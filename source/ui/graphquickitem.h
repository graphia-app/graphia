#ifndef GRAPHQUICKITEM_H
#define GRAPHQUICKITEM_H

#include "../graph/graph.h"

#include <QQuickFramebufferObject>
#include <QTimer>
#include <QString>
#include <QEvent>

#include <vector>
#include <queue>
#include <memory>

class GraphRenderer;
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

public:
    explicit GraphQuickItem(QQuickItem* parent = nullptr);

    void initialise(std::shared_ptr<GraphModel> graphModel,
                    CommandManager& commandManager,
                    std::shared_ptr<SelectionManager> selectionManager);

    std::shared_ptr<GraphModel> graphModel() { return _graphModel; }
    std::shared_ptr<SelectionManager> selectionManager() { return _selectionManager; }

    void resetView();
    bool viewResetPending();

    bool interacting() const;
    void setInteracting(bool interacting) const;
    bool viewIsReset() const;
    void setViewIsReset(bool viewIsReset);
    bool canEnterOverviewMode() const;
    void setCanEnterOverviewMode(bool canEnterOverviewMode);

    void switchToOverviewMode(bool doTransition = true);
    bool overviewModeSwitchPending();

    Renderer* createRenderer() const;

    bool eventsPending();
    std::unique_ptr<QEvent> nextEvent();

private:
    bool event(QEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    std::shared_ptr<GraphModel> _graphModel;
    CommandManager* _commandManager = nullptr;
    std::shared_ptr<SelectionManager> _selectionManager;

    bool _viewResetPending = false;
    bool _overviewModeSwitchPending = false;

    mutable bool _interacting = false;
    bool _viewIsReset = true;
    bool _canEnterOverviewMode = false;

    std::queue<std::unique_ptr<QEvent>> _eventQueue;

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

signals:
    void userInteractionStarted() const;
    void userInteractionFinished() const;

    void interactingChanged() const;
    void viewIsResetChanged() const;
    void canEnterOverviewModeChanged() const;

    void commandWillExecuteAsynchronously(const Command* command) const;
    void commandCompleted(const Command* command, const QString& pastParticiple) const;

    void layoutChanged() const;

    void graphChanged() const;
};

#endif // GRAPHQUICKITEM_H
