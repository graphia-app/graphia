#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>

#include <memory>

class GraphComponentScene;
class GraphComponentInteractor;
class GraphModel;
class CommandManager;
class SelectionManager;

class GraphWidget : public QWidget
{
    Q_OBJECT
public:
    GraphWidget(std::shared_ptr<GraphModel> graphModel,
                CommandManager& commandManager,
                std::shared_ptr<SelectionManager> selectionManager,
                QWidget *parent = nullptr);

private:
    std::shared_ptr<GraphComponentScene> _graphComponentScene;
    std::shared_ptr<GraphComponentInteractor> _graphComponentInteractor;

signals:
    void userInteractionStarted();
    void userInteractionFinished();
};

#endif // GRAPHWIDGET_H
