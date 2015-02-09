#include "graphscene.h"
#include "graphcomponentrenderer.h"

#include "../graph/graphmodel.h"

#include "../ui/graphwidget.h"

#include "../utils/utils.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QPoint>

#include <stack>
#include <vector>

GraphScene::GraphScene(GraphWidget* graphWidget)
    : Scene(graphWidget),
      _graphWidget(graphWidget),
      _graphModel(graphWidget->graphModel()),
      _width(0), _height(0),
      _renderSizeDivisor(1),
      _renderSizeDivisors(graphWidget->graphModel()->graph()),
      _componentLayout(graphWidget->graphModel()->graph())
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

    layoutComponents();
}

void GraphScene::render()
{
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _funcs->glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto renderer = GraphComponentRenderersReference::renderer(componentId);
        auto rect = _componentLayout[componentId];

        renderer->render(rect.x(), rect.y(), rect.width(), rect.height());
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
        int dividedSize = size / (divisor * _renderSizeDivisor);

        const int MINIMUM_SIZE = 32;
        if(size > MINIMUM_SIZE)
        {
            while(dividedSize < MINIMUM_SIZE && divisor > 1)
            {
                divisor /= 2;
                dividedSize = size / (divisor * _renderSizeDivisor);
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

void GraphScene::zoom(float delta)
{
    if(delta > 0.0f)
        setRenderSizeDivisor(_renderSizeDivisor / 2);
    else
        setRenderSizeDivisor(_renderSizeDivisor * 2);
}

void GraphScene::setRenderSizeDivisor(int divisor)
{
    if(divisor < 1)
        divisor = 1;

    _renderSizeDivisor = divisor;

    resize(_width, _height);
}

void GraphScene::layoutComponents()
{
    auto& graph = _graphModel->graph();
    auto sortedComponentIds = _graphModel->graph().componentIds();
    std::sort(sortedComponentIds.begin(), sortedComponentIds.end(),
              [&graph](const ComponentId& a, const ComponentId& b)
    {
        return graph.componentById(a)->numNodes() > graph.componentById(b)->numNodes();
    });

    std::stack<QPoint> coords;

    //FIXME this is a mess
    coords.emplace(0, 0);
    for(auto componentId : sortedComponentIds)
    {
        auto coord = coords.top();
        coords.pop();

        auto renderer = GraphComponentRenderersReference::renderer(componentId);

        if(!coords.empty() && (coord.x() + renderer->width() > coords.top().x() ||
            coord.y() + renderer->height() > _height))
        {
            coord = coords.top();
            coords.pop();
        }

        _componentLayout[componentId] = QRect(coord.x(), coord.y(), renderer->width(), renderer->height());

        QPoint right(coord.x() + renderer->width(), coord.y());
        QPoint down(coord.x(), coord.y() + renderer->height());

        if(coords.empty() || right.x() < coords.top().x())
            coords.emplace(right);

        if(down.y() < _height)
            coords.emplace(down);
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
                renderer->cloneViewDataFrom(*oldGraphComponentRenderer);
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
        onShow();

        _graphWidget->executeOnRendererThread([this]
        {
            resize(_width, _height);
        }, "GraphScene::onGraphChanged (resize)");
    }
}
