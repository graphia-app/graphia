#include "graphscene.h"
#include "graphcomponentrenderer.h"

#include "../graph/graphmodel.h"

#include "../ui/graphwidget.h"

#include "../utils/utils.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>

#include <stack>

GraphScene::GraphScene(GraphWidget* graphWidget)
    : Scene(graphWidget),
      _graphWidget(graphWidget),
      _graphModel(graphWidget->graphModel()),
      _width(0), _height(0),
      _renderSizeDivisors(graphWidget->graphModel()->graph())
{
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &GraphScene::onComponentSplit, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphScene::onGraphChanged, Qt::DirectConnection);
}

void GraphScene::initialise()
{
    _funcs = context().versionFunctions<QOpenGLFunctions_3_3_Core>();
    if(!_funcs)
        qFatal("Could not obtain required OpenGL context version");
    _funcs->initializeOpenGLFunctions();

    onGraphChanged(&_graphWidget->graphModel()->graph());
}

void GraphScene::update(float t)
{
    _graphWidget->updateNodePositions();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto renderer = GraphComponentRenderersReference::renderer(componentId);
        renderer->update(t);
    }

    auto& graph = _graphModel->graph();
    _sortedComponentIds = _graphModel->graph().componentIds();
    std::sort(_sortedComponentIds.begin(), _sortedComponentIds.end(),
              [&graph](const ComponentId& a, const ComponentId& b)
    {
        return graph.componentById(a)->numNodes() > graph.componentById(b)->numNodes();
    });
}

void GraphScene::render()
{
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _funcs->glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    struct Coord
    {
        Coord(int x, int y) { _x = x; _y = y; }
        Coord(const Coord& other) { _x = other._x; _y = other._y; }
        int _x, _y;
    };
    std::stack<Coord> coords;

    coords.emplace(0, 0);
    for(auto componentId : _sortedComponentIds)
    {
        Coord c = coords.top();
        coords.pop();

        auto renderer = GraphComponentRenderersReference::renderer(componentId);

        if(c._y + renderer->height() > _height)
        {
            c = coords.top();
            coords.pop();
        }

        renderer->render(c._x, c._y);

        Coord right(c._x + renderer->width(), c._y);
        Coord down(c._x, c._y + renderer->height());

        if(coords.empty() || right._x < coords.top()._x)
            coords.emplace(right);

        if(down._y < _height)
            coords.emplace(down);
    }
}

void GraphScene::resize(int width, int height)
{
    _width = width;
    _height = height;

    int size = height;

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto renderer = GraphComponentRenderersReference::renderer(componentId);
        int divisor =_renderSizeDivisors[componentId];
        int dividedSize = size / divisor;

        const int MINIMUM_SIZE = 64;
        if(size > MINIMUM_SIZE)
        {
            while(dividedSize < MINIMUM_SIZE)
            {
                divisor /= 2;
                dividedSize = size / divisor;
            }
        }

        renderer->resizeViewport(width, height);
        renderer->resize(dividedSize, dividedSize);
    }
}

void GraphScene::onShow()
{
    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto renderer = GraphComponentRenderersReference::renderer(componentId);
        renderer->setVisible(true);
    }
}

void GraphScene::onHide()
{
    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto renderer = GraphComponentRenderersReference::renderer(componentId);
        renderer->setVisible(false);
    }
}

void GraphScene::onComponentSplit(const Graph*, ComponentId oldComponentId,
                                  const ElementIdSet<ComponentId>& splitters)
{
    auto oldGraphComponentRenderer = GraphComponentRenderersReference::renderer(oldComponentId);

    _graphWidget->executeOnRendererThread([this, oldGraphComponentRenderer, splitters]
    {
        if(visible())
        {
            for(auto splitter : splitters)
            {
                auto renderer = GraphComponentRenderersReference::renderer(splitter);
                renderer->cloneCameraDataFrom(*oldGraphComponentRenderer);
            }
        }
    }, "GraphScene::onComponentSplit (cloneCameraDataFrom)");
}

void GraphScene::onGraphChanged(const Graph* graph)
{
    // Find the number of nodes in the largest component
    int maxNumNodes = 0;
    for(auto componentId : graph->componentIds())
    {
        auto component = graph->componentById(componentId);
        if(component->numNodes() > maxNumNodes)
            maxNumNodes = component->numNodes();
    }

    for(auto componentId : graph->componentIds())
    {
        auto component = graph->componentById(componentId);
        int divisor = maxNumNodes / component->numNodes();
        _renderSizeDivisors[componentId] = Utils::smallestPowerOf2GreaterThan(divisor);
    }

    if(visible())
    {
        _graphWidget->executeOnRendererThread([this]
        {
            resize(_width, _height);
        }, "GraphScene::onGraphChanged (resize)");
    }
}
