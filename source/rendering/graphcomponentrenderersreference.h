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
    GraphComponentRendererManager(GraphComponentRendererManager&& other);

    GraphComponentRenderer* get() { return _graphComponentRenderer.get(); }
};

class GraphComponentRenderersReference
{
private:
    std::shared_ptr<ComponentArray<GraphComponentRendererManager>> _rendererManagers;

public:
    void setGraphComponentRendererManagers(std::shared_ptr<ComponentArray<GraphComponentRendererManager>> rendererManagers)
    {
        _rendererManagers = rendererManagers;
    }

    ComponentArray<GraphComponentRendererManager>& rendererManagers()
    {
        return *_rendererManagers.get();
    }

    GraphComponentRenderer* renderer(ComponentId componentId)
    {
        if(componentId.isNull())
            return nullptr;

        return _rendererManagers->at(componentId).get();
    }
};

#endif // GRAPHCOMPONENTRENDERERSREFERENCE_CPP
