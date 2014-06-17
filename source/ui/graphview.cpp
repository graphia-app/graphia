#include "graphview.h"

#include "graphcomponentinteractor.h"
#include "selectionmanager.h"
#include "commandmanager.h"

#include "../rendering/openglwindow.h"

#include "../graph/graph.h"
#include "../graph/graphmodel.h"

#include "../maths/frustum.h"
#include "../maths/plane.h"
#include "../maths/boundingsphere.h"

#include "../layout/collision.h"

#include "../utils.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>
#include <cmath>

GraphView::GraphView(GraphModel* graphModel, CommandManager* commandManager, SelectionManager* selectionManager, QWidget *parent) :
    QWidget(parent)
{
    OpenGLWindow* window = new OpenGLWindow;

    _graphComponentScene = new GraphComponentScene;
    window->setScene(_graphComponentScene);
    _graphComponentInteractor = new GraphComponentInteractor(graphModel, _graphComponentScene, commandManager, selectionManager);
    window->setInteractor(_graphComponentInteractor);

    connect(_graphComponentInteractor, &Interactor::userInteractionStarted, this, &GraphView::userInteractionStarted);
    connect(_graphComponentInteractor, &Interactor::userInteractionFinished, this, &GraphView::userInteractionFinished);

    setLayout(new QVBoxLayout());
    layout()->addWidget(QWidget::createWindowContainer(window));
}

GraphView::~GraphView()
{
    delete _graphComponentInteractor;
    delete _graphComponentScene;
}
