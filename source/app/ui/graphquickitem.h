#ifndef GRAPHQUICKITEM_H
#define GRAPHQUICKITEM_H

#include "graph/qmlelementid.h"
#include "rendering/compute/gpucomputethread.h"

#include <QQuickFramebufferObject>
#include <QTimer>
#include <QString>
#include <QEvent>
#include <QImage>
#include <QDesktopServices>

#include <vector>
#include <queue>
#include <memory>

class GraphModel;
class ICommand;
class CommandManager;
class SelectionManager;

class GraphQuickItem : public QQuickFramebufferObject
{
    Q_OBJECT

    Q_PROPERTY(bool interacting MEMBER _interacting NOTIFY interactingChanged)
    Q_PROPERTY(bool viewIsReset MEMBER _viewIsReset NOTIFY viewIsResetChanged)
    Q_PROPERTY(bool canEnterOverviewMode MEMBER _canEnterOverviewMode NOTIFY canEnterOverviewModeChanged)
    Q_PROPERTY(bool inOverviewMode MEMBER _inOverviewMode NOTIFY inOverviewModeChanged)

    Q_PROPERTY(int numNodes READ numNodes NOTIFY graphChanged)
    Q_PROPERTY(int numVisibleNodes READ numVisibleNodes NOTIFY graphChanged)
    Q_PROPERTY(int numEdges READ numEdges NOTIFY graphChanged)
    Q_PROPERTY(int numVisibleEdges READ numVisibleEdges NOTIFY graphChanged)
    Q_PROPERTY(int numComponents READ numComponents NOTIFY graphChanged)

    Q_PROPERTY(int visibleComponentIndex MEMBER _visibleComponentIndex NOTIFY visibleComponentIndexChanged)

    Q_PROPERTY(float fps READ fps NOTIFY fpsChanged)

public:
    explicit GraphQuickItem(QQuickItem* parent = nullptr);

    void initialise(GraphModel *graphModel,
                    CommandManager* commandManager,
                    SelectionManager *selectionManager,
                    GPUComputeThread *gpuComputeThread);

    GraphModel* graphModel() { return _graphModel; }
    SelectionManager* selectionManager() { return _selectionManager; }

    void resetView();
    bool viewResetPending();

    bool interacting() const;
    void setInteracting(bool interacting) const;
    bool viewIsReset() const;
    bool inOverviewMode() const;
    bool canEnterOverviewMode() const;

    void switchToOverviewMode(bool doTransition = true);
    bool overviewModeSwitchPending();

    void moveFocusToNode(NodeId nodeId);
    NodeId desiredFocusNodeId();

    void moveFocusToComponent(ComponentId componentId);
    ComponentId desiredFocusComponentId();
    ComponentId focusedComponentId() const;

    Renderer* createRenderer() const override;

    bool eventsPending();
    std::unique_ptr<QEvent> nextEvent();

    float fps() const { return _fps; }

    // These are only called by GraphRenderer so that it can tell
    // interested parties what it's doing
    void setViewIsReset(bool viewIsReset);
    void setCanEnterOverviewMode(bool canEnterOverviewMode);
    void setInOverviewMode(bool inOverviewMode);
    void setFocusedComponentId(ComponentId componentId);
    Q_INVOKABLE void captureScreenshot(int width, int height, QString path, int dpi, bool fillSize);
    Q_INVOKABLE void requestPreview(int width, int height, bool fillSize);

private:
    bool event(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

    GraphModel* _graphModel = nullptr;
    GPUComputeThread* _gpuComputeThread = nullptr;
    CommandManager* _commandManager = nullptr;
    SelectionManager* _selectionManager = nullptr;

    bool _viewResetPending = false;
    bool _overviewModeSwitchPending = false;
    NodeId _desiredFocusNodeId;
    ComponentId _desiredFocusComponentId;

    mutable bool _interacting = false;
    bool _viewIsReset = true;
    bool _canEnterOverviewMode = false;
    bool _inOverviewMode = true;
    ComponentId _focusedComponentId;
    int _visibleComponentIndex = -1;

    std::queue<std::unique_ptr<QEvent>> _eventQueue;

    mutable float _fps = 0.0f;

    template<typename T> void enqueueEvent(const T* event)
    {
        if(!isEnabled())
            return;

        _eventQueue.emplace(std::make_unique<T>(*event));
        update();
    }

    int numNodes() const;
    int numVisibleNodes() const;
    int numEdges() const;
    int numVisibleEdges() const;
    int numComponents() const;

    void updateVisibleComponentIndex();

public slots:
    void onLayoutChanged();
    void onFPSChanged(float fps);
    void onUserInteractionStarted();
    void onUserInteractionFinished();
    void onScreenshotComplete(QImage screenshot, QString path);

signals:
    void interactingChanged() const;
    void viewIsResetChanged() const;
    void canEnterOverviewModeChanged() const;
    void inOverviewModeChanged() const;
    void focusedComponentIdChanged() const;
    void visibleComponentIndexChanged() const;

    void commandWillExecute() const;
    void commandCompleted() const;

    void layoutChanged() const;

    void screenshotRequested(int width, int height, QString path, int dpi, bool fillSize) const;
    void previewRequested(int width, int height, bool fillSize) const;
    void previewComplete(QString previewBase64) const;

    void graphChanged() const;

    void fpsChanged() const;

    void clicked(int button, QmlNodeId nodeId) const;
};

#endif // GRAPHQUICKITEM_H
