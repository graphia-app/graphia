#include "graphview.h"

#include "../gl/graphscene.h"
#include "../gl/openglwindow.h"

#include <QVBoxLayout>

GraphView::GraphView(QWidget *parent) :
    QWidget(parent)
{
    OpenGLWindow* window = new OpenGLWindow(surfaceFormat());

    graphScene = new GraphScene;
    window->setScene(graphScene);

    connect(graphScene, &GraphScene::userInteractionStarted, this, &GraphView::userInteractionStarted);
    connect(graphScene, &GraphScene::userInteractionFinished, this, &GraphView::userInteractionFinished);

    this->setLayout(new QVBoxLayout());
    this->layout()->addWidget(QWidget::createWindowContainer(window));
}

void GraphView::layoutChanged()
{
}
