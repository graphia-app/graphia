#include "graphcomponentrenderersreference.h"

#include "../utils/make_unique.h"

GraphComponentRendererManager::GraphComponentRendererManager() :
    _graphComponentRenderer(std::make_unique<GraphComponentRenderer>())
{
}

GraphComponentRendererManager::GraphComponentRendererManager(GraphComponentRendererManager&& other) :
    _graphComponentRenderer(std::move(other._graphComponentRenderer))
{
}
