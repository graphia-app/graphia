#include "graphscene.h"

#include "graphcomponentviewdata.h"

#include <QObject>

GraphScene::GraphScene(std::shared_ptr<ComponentArray<GraphComponentViewData>> /*componentsViewData*/,
                       QObject* parent)
    : Scene(parent)
{
    update(0.0f);
}

void GraphScene::initialise()
{
}

void GraphScene::cleanup()
{
}

void GraphScene::update(float /*t*/)
{
}

void GraphScene::render()
{
}

void GraphScene::resize(int /*w*/, int /*h*/)
{
}
