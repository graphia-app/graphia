#include "graphwidget.h"

#include "../rendering/openglwindow.h"
#include "../rendering/graphcomponentscene.h"

#include "graphcomponentinteractor.h"

#include <QVBoxLayout>

GraphWidget::GraphWidget(GraphModel* graphModel,
                         CommandManager* commandManager,
                         SelectionManager* selectionManager, QWidget *parent) :
    QWidget(parent)
{
    OpenGLWindow* window = new OpenGLWindow;

    _graphComponentScene = new GraphComponentScene;
    _graphComponentInteractor = new GraphComponentInteractor(graphModel, _graphComponentScene, commandManager, selectionManager);
    window->setScene(_graphComponentScene);
    window->setInteractor(_graphComponentInteractor);

    connect(_graphComponentInteractor, &Interactor::userInteractionStarted, this, &GraphWidget::userInteractionStarted);
    connect(_graphComponentInteractor, &Interactor::userInteractionFinished, this, &GraphWidget::userInteractionFinished);

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(QWidget::createWindowContainer(window));
}

GraphWidget::~GraphWidget()
{
    delete _graphComponentInteractor;
    delete _graphComponentScene;
}
