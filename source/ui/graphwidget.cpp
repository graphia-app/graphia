#include "graphwidget.h"

#include "../rendering/openglwindow.h"
#include "../rendering/graphcomponentscene.h"

#include "graphcomponentinteractor.h"

#include <QVBoxLayout>

GraphWidget::GraphWidget(std::shared_ptr<GraphModel> graphModel,
                         CommandManager &commandManager,
                         std::shared_ptr<SelectionManager> selectionManager, QWidget *parent) :
    QWidget(parent),
    _graphComponentScene(new GraphComponentScene),
    _graphComponentInteractor(new GraphComponentInteractor(graphModel,
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

void GraphWidget::enableInteraction()
{
    _openGLWindow->enableInteraction();
    _graphComponentScene->enableInteraction();
}

void GraphWidget::disableInteraction()
{
    _graphComponentScene->disableInteraction();
    _openGLWindow->disableInteraction();
}
