#ifndef CONTENTPANEWIDGET_H
#define CONTENTPANEWIDGET_H

#include <QWidget>
#include <QMap>

#include "../graph/graph.h"
#include "../graph/graphmodel.h"
#include "commandmanager.h"

class SelectionManager;
class GraphFileParserThread;
class NodeLayoutThread;
class LayoutThread;
class CommandManager;

class ContentPaneWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContentPaneWidget(QWidget* parent = 0);
    virtual ~ContentPaneWidget();

signals:
    void progress(int percentage) const;
    void complete(int success) const;
    void graphChanged(const Graph*) const;
    void commandStackChanged(const CommandManager& commandManager) const;

public slots:
    void onCompletion(int success);

private:
    GraphModel* _graphModel;
    SelectionManager* _selectionManager;
    CommandManager _commandManager;
    GraphFileParserThread* _graphFileParserThread;
    NodeLayoutThread* _nodeLayoutThread;
    LayoutThread* _componentLayoutThread;

    bool _resumePreviouslyActiveLayout;

private slots:
    void onGraphWillChange(const Graph*);
    void onGraphChanged(const Graph* graph);

    void onComponentAdded(const Graph*, ComponentId componentId);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId);
    void onComponentSplit(const Graph*, ComponentId splitter, const QSet<ComponentId>& splitters);
    void onComponentsWillMerge(const Graph*, const QSet<ComponentId>& mergers, ComponentId merger);

public:
    GraphModel* graphModel() { return _graphModel; }
    SelectionManager* selectionManager() { return _selectionManager; }
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

    bool initFromFile(const QString& filename);
};

#endif // CONTENTPANEWIDGET_H
