#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>

class GraphComponentScene;
class GraphComponentInteractor;
class GraphModel;
class CommandManager;
class SelectionManager;

class GraphWidget : public QWidget
{
    Q_OBJECT
public:
    GraphWidget(GraphModel* graphModel,
                CommandManager* commandManager,
                SelectionManager* selectionManager,
                QWidget *parent = nullptr);
    virtual ~GraphWidget();

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

#endif // GRAPHWIDGET_H
