#include "graphview.h"

#include "../gl/graphscene.h"
#include "../gl/openglwindow.h"

#include <QSurfaceFormat>
#include <QVBoxLayout>

GraphView::GraphView(QWidget *parent) :
    QWidget(parent)
{
    OpenGLWindow* window = new OpenGLWindow(surfaceFormat());

    graphScene = new GraphScene;
    window->setScene(graphScene);

    this->setLayout(new QVBoxLayout());
    this->layout()->addWidget(QWidget::createWindowContainer(window));
}
