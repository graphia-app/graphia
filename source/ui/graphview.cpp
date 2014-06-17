#include "graphview.h"

#include "../rendering/openglwindow.h"
#include "../rendering/graphcomponentscene.h"

#include "graphcomponentinteractor.h"

#include <QVBoxLayout>

GraphView::GraphView(GraphModel* graphModel, CommandManager* commandManager, SelectionManager* selectionManager, QWidget *parent) :
    QWidget(parent)
{
    OpenGLWindow* window = new OpenGLWindow;

    _graphComponentScene = new GraphComponentScene;
    _graphComponentInteractor = new GraphComponentInteractor(graphModel, _graphComponentScene, commandManager, selectionManager);
    window->setScene(_graphComponentScene);
    window->setInteractor(_graphComponentInteractor);

    connect(_graphComponentScene, &Scene::userInteractionStarted, this, &GraphView::userInteractionStarted);
    connect(_graphComponentInteractor, &Interactor::userInteractionStarted, this, &GraphView::userInteractionStarted);
    connect(_graphComponentScene, &Scene::userInteractionFinished, this, &GraphView::userInteractionFinished);
    connect(_graphComponentInteractor, &Interactor::userInteractionFinished, this, &GraphView::userInteractionFinished);

    setLayout(new QVBoxLayout());
    layout()->addWidget(QWidget::createWindowContainer(window));
}

GraphView::~GraphView()
{
    delete _graphComponentInteractor;
    delete _graphComponentScene;
}
