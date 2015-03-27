#include "graphcomponentrenderersreference.h"

#include "../utils/cpp1x_hacks.h"

GraphComponentRendererManager::GraphComponentRendererManager() :
    _graphComponentRenderer(std::make_unique<GraphComponentRenderer>())
{
}

GraphComponentRendererManager::GraphComponentRendererManager(GraphComponentRendererManager&& other) noexcept :
    _graphComponentRenderer(std::move(other._graphComponentRenderer))
{
}

void GraphComponentRenderersReference::setGraphComponentRendererManagers(std::shared_ptr<ComponentArray<GraphComponentRendererManager> > rendererManagers)
{
    _rendererManagers = rendererManagers;
}

ComponentArray<GraphComponentRendererManager>& GraphComponentRenderersReference::rendererManagers()
{
    return *_rendererManagers.get();
}

GraphComponentRenderer* GraphComponentRenderersReference::rendererForComponentId(ComponentId componentId)
{
    if(componentId.isNull())
        return nullptr;

    return _rendererManagers->at(componentId).get();
}
