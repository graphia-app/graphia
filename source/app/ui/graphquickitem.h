/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef GRAPHQUICKITEM_H
#define GRAPHQUICKITEM_H

#include "graph/qmlelementid.h"
#include "rendering/compute/gpucomputethread.h"
#include "rendering/projection.h"
#include "rendering/shading.h"

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
class IGraphComponent;

class GraphQuickItem : public QQuickFramebufferObject
{
    Q_OBJECT

    Q_PROPERTY(bool initialised MEMBER _initialised NOTIFY initialisedChanged)

    Q_PROPERTY(bool interacting MEMBER _interacting NOTIFY interactingChanged)
    Q_PROPERTY(bool transitioning MEMBER _transitioning NOTIFY transitioningChanged)
    Q_PROPERTY(bool viewIsReset MEMBER _viewIsReset NOTIFY viewIsResetChanged)
    Q_PROPERTY(bool canEnterOverviewMode MEMBER _canEnterOverviewMode NOTIFY canEnterOverviewModeChanged)
    Q_PROPERTY(bool inOverviewMode MEMBER _inOverviewMode NOTIFY inOverviewModeChanged)

    Q_PROPERTY(int numNodes READ numNodes NOTIFY metricsChanged)
    Q_PROPERTY(int numVisibleNodes READ numVisibleNodes NOTIFY metricsChanged)
    Q_PROPERTY(int numEdges READ numEdges NOTIFY metricsChanged)
    Q_PROPERTY(int numVisibleEdges READ numVisibleEdges NOTIFY metricsChanged)
    Q_PROPERTY(int numComponents READ numComponents NOTIFY metricsChanged)

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

    Projection projection() const;
    void setProjection(Projection projection);

    Shading shading2D() const;
    void setShading2D(Shading shading2D);

    Shading shading3D() const;
    void setShading3D(Shading shading3D);

    bool updating() const { return _updating; }
    bool initialised() const { return _initialised; }
    bool interacting() const { return _interacting; }
    void setInteracting(bool interacting) const;
    bool transitioning() const { return _transitioning; }
    void setTransitioning(bool transitioning) const;
    bool viewIsReset() const;
    bool inOverviewMode() const;
    bool canEnterOverviewMode() const;

    void switchToOverviewMode(bool doTransition = true);
    bool overviewModeSwitchPending();

    void moveFocusToNode(NodeId nodeId);
    void moveFocusToNodes(const std::vector<NodeId>& nodeIds);
    std::vector<NodeId> desiredFocusNodeIds();

    void moveFocusToComponent(ComponentId componentId);
    ComponentId desiredFocusComponentId();
    ComponentId focusedComponentId() const;

    Renderer* createRenderer() const override;

    auto& events() { return _eventQueue; }

    float fps() const { return _fps; }

    // These are only called by GraphRenderer so that it can tell
    // interested parties what it's doing
    void setViewIsReset(bool viewIsReset);
    void setCanEnterOverviewMode(bool canEnterOverviewMode);
    void setInOverviewMode(bool inOverviewMode);
    void setFocusedComponentId(ComponentId componentId);

public:
    Q_INVOKABLE void captureScreenshot(int width, int height, const QString& path, int dpi, bool fillSize);
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
    std::vector<NodeId> _desiredFocusNodeIds;
    ComponentId _desiredFocusComponentId;

    bool _initialised = false;
    Projection _projection = Projection::Perspective;
    Shading _shading2D = Shading::Flat;
    Shading _shading3D = Shading::Smooth;
    mutable bool _interacting = false;
    mutable bool _transitioning = false;
    bool _viewIsReset = true;
    bool _canEnterOverviewMode = false;
    bool _inOverviewMode = true;
    ComponentId _focusedComponentId;
    int _visibleComponentIndex = -1;
    bool _updating = false;

    std::queue<std::unique_ptr<QEvent>> _eventQueue;

    mutable float _fps = 0.0f;

    template<typename T> void enqueueEvent(const T* event)
    {
        if(!isEnabled())
            return;

        _eventQueue.emplace(std::unique_ptr<T>(event->clone()));
        update();
    }

    const IGraphComponent* focusedComponent() const;

    int numNodes() const;
    int numVisibleNodes() const;
    int numEdges() const;
    int numVisibleEdges() const;
    int numComponents() const;

    void updateVisibleComponentIndex();

public slots:
    void updateRenderer();
    void onLayoutChanged();

private slots:
    void onRendererInitialised();
    void onSynchronizeComplete();
    void onFPSChanged(float fps);
    void onUserInteractionStarted() const;
    void onUserInteractionFinished();
    void onTransitionStarted() const;
    void onTransitionFinished();
    void onScreenshotComplete(const QImage& screenshot, const QString& path);

signals:
    void initialisedChanged();

    void updatingChanged();
    void interactingChanged() const; // clazy:exclude=const-signal-or-slot
    void transitioningChanged() const; // clazy:exclude=const-signal-or-slot
    void viewIsResetChanged();
    void canEnterOverviewModeChanged();
    void inOverviewModeChanged();
    void focusedComponentIdChanged();
    void visibleComponentIndexChanged();

    void commandsStarted();
    void commandsFinished();

    void layoutChanged();

    void screenshotRequested(int width, int height, QString path, int dpi, bool fillSize);
    void previewRequested(int width, int height, bool fillSize);
    void previewComplete(QString previewBase64);

    void graphChanged();
    void metricsChanged();

    void fpsChanged();

    void clicked(int button, QmlNodeId nodeId);
};

#endif // GRAPHQUICKITEM_H
