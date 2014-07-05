#include "graphwidget.h"

#include "../rendering/openglwindow.h"
#include "../rendering/graphcomponentscene.h"

#include "graphcomponentinteractor.h"

#include <QVBoxLayout>

GraphWidget::GraphWidget(std::shared_ptr<GraphModel> graphModel,
                         CommandManager &commandManager,
                         std::shared_ptr<SelectionManager> selectionManager, QWidget *parent) :
    QWidget(parent),
    _graphComponentScene(std::make_shared<GraphComponentScene>()),
    _graphComponentInteractor(std::make_shared<GraphComponentInteractor>(graphModel,
                                                                         _graphComponentScene,
                                                                         commandManager,
                                                                         selectionManager))
{
    _openGLWindow = new OpenGLWindow;

    _openGLWindow->setScene(_graphComponentScene);
    _openGLWindow->setInteractor(_graphComponentInteractor);

    connect(_graphComponentInteractor.get(), &Interactor::userInteractionStarted, this, &GraphWidget::userInteractionStarted);
    connect(_graphComponentInteractor.get(), &Interactor::userInteractionFinished, this, &GraphWidget::userInteractionFinished);

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(QWidget::createWindowContainer(_openGLWindow));
}

bool GraphWidget::interacting() const
{
    return _graphComponentInteractor->interacting();
}

void GraphWidget::onCommandWillExecuteAsynchronously(const Command*, const QString&)
{
    _graphComponentScene->disableInteraction();
    _openGLWindow->disableInteraction();
    _openGLWindow->disableSceneUpdate();
}

void GraphWidget::onCommandCompleted(const Command*)
{
    _openGLWindow->enableSceneUpdate();
    _openGLWindow->enableInteraction();
    _graphComponentScene->enableInteraction();
}
