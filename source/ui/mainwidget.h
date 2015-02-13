#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include "../graph/graph.h"
#include "../commands/commandmanager.h"

#include <memory>
#include <mutex>

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
    void commandWillExecuteAsynchronously(const Command* command, const QString& verb) const;
    void commandProgress(const Command* command, int progress) const;
    void commandCompleted(const Command* command, const QString& pastParticiple) const;
    void selectionChanged(const SelectionManager* selectionManager) const;
    void userInteractionStarted() const;
    void userInteractionFinished() const;

public slots:
    void onLoadCompletion(bool success);

private:
    bool _loadComplete;

    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<SelectionManager> _selectionManager;
    CommandManager _commandManager;
    std::unique_ptr<GraphFileParserThread> _graphFileParserThread;
    std::unique_ptr<NodeLayoutThread> _nodeLayoutThread;
    GraphWidget* _graphWidget;

    std::mutex _autoResumeMutex;
    int _autoResume;

private slots:
    void onGraphWillChange(const Graph*);
    void onGraphChanged(const Graph* graph);

    void onComponentAdded(const Graph*, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool);
    void onComponentSplit(const Graph*, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph*, const ComponentMergeSet& componentMergeSet);

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

    bool busy() const;
    bool interacting() const;

    void resetView();
    bool viewIsReset() const;

    void toggleModes();

    void deleteSelectedNodes();

    bool initFromFile(const QString& filename);
};

#endif // MAINWIDGET_H
