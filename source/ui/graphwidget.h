#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>

#include <memory>

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

private:
    std::shared_ptr<GraphComponentScene> _graphComponentScene;
    std::shared_ptr<GraphComponentInteractor> _graphComponentInteractor;
    OpenGLWindow* _openGLWindow;

public slots:
    void onCommandWillExecuteAsynchronously(std::shared_ptr<const Command> command, const QString& verb);
    void onCommandCompleted(std::shared_ptr<const Command> command);

signals:
    void userInteractionStarted();
    void userInteractionFinished();
};

#endif // GRAPHWIDGET_H
