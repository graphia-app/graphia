#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>

class GraphComponentScene;
class GraphComponentInteractor;
class GraphModel;
class CommandManager;
class SelectionManager;

class GraphView : public QWidget
{
    Q_OBJECT
public:
    GraphView(GraphModel* graphModel,
              CommandManager* commandManager,
              SelectionManager* selectionManager,
              QWidget *parent = nullptr);
    virtual ~GraphView();

private:
    GraphComponentScene* _graphComponentScene;
    GraphComponentInteractor* _graphComponentInteractor;

    GraphModel* _graphModel;
    CommandManager* _commandManager;
    SelectionManager* _selectionManager;

signals:
    void userInteractionStarted();
    void userInteractionFinished();
};

#endif // GRAPHVIEW_H
