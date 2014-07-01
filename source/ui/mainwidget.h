#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include "../graph/graph.h"
#include "commandmanager.h"

#include <memory>

class GraphModel;
class SelectionManager;
class GraphFileParserThread;
class NodeLayoutThread;
class LayoutThread;
class GraphWidget;

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget* parent = 0);
    virtual ~MainWidget();

signals:
    void progress(int percentage) const;
    void complete(int success) const;
    void graphChanged(const Graph*) const;
    void commandWillExecuteAsynchronously(const CommandManager* commandManager, const Command* command) const;
    void commandProgress(const CommandManager* commandManager, const Command* command, int progress) const;
    void commandCompleted(const CommandManager* commandManager, const Command* command) const;
    void selectionChanged(const SelectionManager* selectionManager) const;

public slots:
    void onCompletion(int success);

private:
    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<SelectionManager> _selectionManager;
    CommandManager _commandManager;
    std::unique_ptr<GraphFileParserThread> _graphFileParserThread;
    std::unique_ptr<NodeLayoutThread> _nodeLayoutThread;
    GraphWidget* _graphWidget;

    bool _resumePreviouslyActiveLayout;

private slots:
    void onGraphWillChange(const Graph*);
    void onGraphChanged(const Graph* graph);

    void onComponentAdded(const Graph*, ComponentId componentId);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId);
    void onComponentSplit(const Graph*, ComponentId splitter, const ElementIdSet<ComponentId>& splitters);
    void onComponentsWillMerge(const Graph*, const ElementIdSet<ComponentId>& mergers, ComponentId merger);

    void onCommandWillExecuteAsynchronously(const CommandManager* commandManager, const Command* command);
    void onCommandCompleted(const CommandManager* commandManager, const Command* command);

public:
    std::shared_ptr<GraphModel> graphModel() { return _graphModel; }
    std::shared_ptr<SelectionManager> selectionManager() { return _selectionManager; }
    void pauseLayout(bool autoResume = false);
    bool layoutIsPaused();
    void resumeLayout(bool autoResume = false);

    void selectAll();
    void selectNone();
    void invertSelection();

    void undo();
    bool canUndo() { return _commandManager.canUndo(); }
    const QString nextUndoAction() const;

    void redo();
    bool canRedo() { return _commandManager.canRedo(); }
    const QString nextRedoAction() const;

    void deleteSelectedNodes();

    bool initFromFile(const QString& filename);
};

#endif // MAINWIDGET_H
