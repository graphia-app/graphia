#ifndef GRAPHCOMPONENTRENDERERSREFERENCE_CPP
#define GRAPHCOMPONENTRENDERERSREFERENCE_CPP

#include "../graph/grapharray.h"
#include "graphcomponentrenderer.h"

#include <memory>

class GraphComponentRendererManager
{
private:
    std::unique_ptr<GraphComponentRenderer> _graphComponentRenderer;

public:
    GraphComponentRendererManager();
    GraphComponentRendererManager(GraphComponentRendererManager&& other) noexcept;

    GraphComponentRenderer* get() { return _graphComponentRenderer.get(); }
};

class GraphComponentRenderersReference
{
private:
    std::shared_ptr<ComponentArray<GraphComponentRendererManager>> _rendererManagers;

public:
    void setGraphComponentRendererManagers(std::shared_ptr<ComponentArray<GraphComponentRendererManager>> rendererManagers);
    ComponentArray<GraphComponentRendererManager>& rendererManagers();
    GraphComponentRenderer* rendererForComponentId(ComponentId componentId);
};

#endif // GRAPHCOMPONENTRENDERERSREFERENCE_CPP
