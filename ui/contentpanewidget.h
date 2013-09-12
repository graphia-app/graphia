#ifndef CONTENTPANEWIDGET_H
#define CONTENTPANEWIDGET_H

#include <QWidget>
#include <QMap>

#include "../graph/graph.h"
#include "../graph/graphmodel.h"

class GraphFileParserThread;
class NodeLayoutThread;
class LayoutThread;

class ContentPaneWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContentPaneWidget(QWidget* parent = 0);
    virtual ~ContentPaneWidget();

signals:
    void progress(int percentage) const;
    void complete(int success) const;

public slots:
    void onProgress(int percentage) const { emit progress(percentage); }
    void onCompletion(int success);

private:
    GraphModel* _graphModel;
    GraphFileParserThread *graphFileParserThread;
    NodeLayoutThread* nodeLayoutThread;
    LayoutThread* componentLayoutThread;

    bool resumeLayoutPostChange;

private slots:
    void onGraphWillChange(const Graph*);
    void onGraphChanged(const Graph*);

    void onComponentAdded(const Graph*, ComponentId componentId);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId);
    void onComponentSplit(const Graph*, ComponentId splitter, QSet<ComponentId> splitters);
    void onComponentsWillMerge(const Graph*, QSet<ComponentId> mergers, ComponentId merger);

public:
    GraphModel* graphModel() { return _graphModel; }
    void pauseLayout();
    bool layoutIsPaused();
    void resumeLayout();

    bool initFromFile(const QString& filename);
};

#endif // CONTENTPANEWIDGET_H
