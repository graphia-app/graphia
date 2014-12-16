#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include "../graph/grapharray.h"

#include <QWidget>

#include <memory>

class GraphComponentViewData;
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

    bool interacting() const;

    void resetView();
    bool viewIsReset() const;

private:
    std::shared_ptr<ComponentArray<GraphComponentViewData>> _graphComponentViewData;

    std::shared_ptr<GraphComponentScene> _graphComponentScene;
    std::shared_ptr<GraphComponentInteractor> _graphComponentInteractor;
    OpenGLWindow* _openGLWindow;

private slots:
    void onGraphChanged(const Graph* graph);
    void onNodeWillBeRemoved(const Graph*, NodeId nodeId);
    void onComponentAdded(const Graph*, ComponentId);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId);
    void onComponentSplit(const Graph* graph, ComponentId oldComponentId, const ElementIdSet<ComponentId>& splitters);
    void onComponentsWillMerge(const Graph* graph, const ElementIdSet<ComponentId>& mergers, ComponentId merged);

public slots:
    void onCommandWillExecuteAsynchronously(const Command* command, const QString& verb);
    void onCommandCompleted(const Command* command);

signals:
    void userInteractionStarted() const;
    void userInteractionFinished() const;
    void layoutChanged() const;
};

#endif // GRAPHWIDGET_H
