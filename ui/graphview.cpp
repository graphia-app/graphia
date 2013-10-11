#include "graphview.h"

#include "../gl/graphscene.h"
#include "../gl/openglwindow.h"
#include "../layout/spatialoctree.h"

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

void GraphView::layoutChanged()
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(_graphModel->graph(), _graphModel->nodePositions());
    SpatialOctTree octree(boundingBox, _graphModel->graph().nodeIds(), _graphModel->nodePositions());
    octree.debugRenderOctTree(graphScene);
}
