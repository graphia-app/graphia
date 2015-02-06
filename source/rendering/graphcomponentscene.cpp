#include "graphcomponentscene.h"

#include "graphcomponentrenderer.h"

#include "../graph/graphmodel.h"

#include "../ui/graphwidget.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>

GraphComponentScene::GraphComponentScene(GraphWidget* graphWidget)
    : Scene(graphWidget),
      _graphWidget(graphWidget),
      _width(0), _height(0)
{
    connect(&graphWidget->graphModel()->graph(), &Graph::componentSplit, this, &GraphComponentScene::onComponentSplit, Qt::DirectConnection);
    connect(&graphWidget->graphModel()->graph(), &Graph::componentsWillMerge, this, &GraphComponentScene::onComponentsWillMerge, Qt::DirectConnection);
    connect(&graphWidget->graphModel()->graph(), &Graph::componentWillBeRemoved, this, &GraphComponentScene::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&graphWidget->graphModel()->graph(), &Graph::graphChanged, this, &GraphComponentScene::onGraphChanged, Qt::DirectConnection);
}

void GraphComponentScene::initialise()
{
    _funcs = context().versionFunctions<QOpenGLFunctions_3_3_Core>();
    if(!_funcs)
        qFatal("Could not obtain required OpenGL context version");
    _funcs->initializeOpenGLFunctions();
}

void GraphComponentScene::update(float t)
{
    _graphWidget->updateNodePositions();

    if(renderer() != nullptr)
        renderer()->update(t);
}

void GraphComponentScene::render()
{
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _funcs->glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    if(renderer() != nullptr)
        renderer()->render(0, 0);
}

void GraphComponentScene::resize(int width, int height)
{
    _width = width;
    _height = height;

    if(renderer() != nullptr)
    {
        renderer()->resizeViewport(width, height);
        renderer()->resize(width, height);
    }
}

void GraphComponentScene::onShow()
{
    if(visible())
    {
        for(auto& rendererManager : rendererManagers())
        {
            auto renderer = rendererManager.get();
            renderer->setVisible(renderer->componentId() == _componentId);
        }
    }
}

void GraphComponentScene::setComponentId(ComponentId componentId)
{
    _componentId = componentId;
    onShow();
}

void GraphComponentScene::resetView(Transition::Type transitionType)
{
    if(renderer() != nullptr)
        renderer()->resetView(transitionType);
}

bool GraphComponentScene::viewIsReset()
{
    if(renderer() == nullptr)
        return true;

    return renderer()->viewIsReset();
}

GraphComponentRenderer* GraphComponentScene::renderer()
{
    return GraphComponentRenderersReference::renderer(_componentId);
}

void GraphComponentScene::onComponentSplit(const Graph* graph, ComponentId oldComponentId,
                                           const ElementIdSet<ComponentId>& splitters)
{
    if(oldComponentId == _componentId)
    {
        ComponentId largestSplitter;

        for(auto splitter : splitters)
        {

            if(largestSplitter.isNull())
                largestSplitter = splitter;
            else
            {
                auto splitterNumNodes = graph->componentById(splitter)->numNodes();
                auto largestNumNodes = graph->componentById(largestSplitter)->numNodes();

                if(splitterNumNodes > largestNumNodes)
                    largestSplitter = splitter;
            }
        }

        auto oldGraphComponentRenderer = GraphComponentRenderersReference::renderer(oldComponentId); 

        _graphWidget->executeOnRendererThread([this, largestSplitter,
                                              oldGraphComponentRenderer]
        {
            auto oldFocusNodeId = oldGraphComponentRenderer->focusNodeId();
            auto& graph = _graphWidget->graphModel()->graph();

            ComponentId newComponentId;

            if(oldFocusNodeId.isNull())
                newComponentId = largestSplitter;
            else
                newComponentId = graph.componentIdOfNode(oldFocusNodeId);

            Q_ASSERT(!newComponentId.isNull());

            auto newGraphComponentRenderer = GraphComponentRenderersReference::renderer(newComponentId);

            newGraphComponentRenderer->cloneCameraDataFrom(*oldGraphComponentRenderer);
            setComponentId(newComponentId);
        }, "GraphComponentScene::onComponentSplit (clone camera data, set component ID)");
    }
}

void GraphComponentScene::onComponentsWillMerge(const Graph*, const ElementIdSet<ComponentId>& mergers,
                                                ComponentId newComponentId)
{
    for(auto merger : mergers)
    {
        if(merger == _componentId)
        {
            auto newGraphComponentRenderer = GraphComponentRenderersReference::renderer(newComponentId);
            auto oldGraphComponentRenderer = GraphComponentRenderersReference::renderer(_componentId);
            _graphWidget->executeOnRendererThread([this, newComponentId,
                                                  newGraphComponentRenderer,
                                                  oldGraphComponentRenderer]
            {
                newGraphComponentRenderer->cloneCameraDataFrom(*oldGraphComponentRenderer);
                setComponentId(newComponentId);
            }, "GraphComponentScene::onComponentsWillMerge (clone camera data, set component ID)");
            break;
        }
    }
}

void GraphComponentScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId)
{
    if(visible())
    {
        _graphWidget->executeOnRendererThread([this, componentId]
        {
            if(componentId == _componentId)
                _graphWidget->switchToOverviewMode();
        }, "GraphComponentScene::onComponentWillBeRemoved (switch to overview mode)");
    }
}

void GraphComponentScene::onGraphChanged(const Graph*)
{
    if(visible())
    {
        _graphWidget->executeOnRendererThread([this]
        {
            resize(_width, _height);
        }, "GraphComponentScene::onGraphChanged (resize)");
    }
}
