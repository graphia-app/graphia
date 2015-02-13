#include "graphoverviewscene.h"
#include "graphcomponentrenderer.h"

#include "../graph/graphmodel.h"

#include "../ui/graphwidget.h"

#include "../utils/utils.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QPoint>

#include <stack>
#include <vector>

GraphOverviewScene::GraphOverviewScene(GraphWidget* graphWidget)
    : Scene(graphWidget),
      _graphWidget(graphWidget),
      _graphModel(graphWidget->graphModel()),
      _width(0), _height(0),
      _renderSizeDivisor(1),
      _renderSizeDivisors(graphWidget->graphModel()->graph()),
      _previousComponentLayout(graphWidget->graphModel()->graph()),
      _componentLayout(graphWidget->graphModel()->graph())
{
    connect(&_graphModel->graph(), &Graph::componentAdded, this, &GraphOverviewScene::onComponentAdded, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentWillBeRemoved, this, &GraphOverviewScene::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &GraphOverviewScene::onComponentSplit, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentsWillMerge, this, &GraphOverviewScene::onComponentsWillMerge, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &GraphOverviewScene::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphOverviewScene::onGraphChanged, Qt::DirectConnection);
}

void GraphOverviewScene::initialise()
{
    _funcs = context().versionFunctions<QOpenGLFunctions_3_3_Core>();
    if(!_funcs)
        qFatal("Could not obtain required OpenGL context version");
    _funcs->initializeOpenGLFunctions();

    onGraphChanged(&_graphWidget->graphModel()->graph());
}

void GraphOverviewScene::update(float t)
{
    _graphWidget->updateNodePositions();

    auto update = [this](ComponentId componentId, float t)
    {
        auto renderer = rendererForComponentId(componentId);
        renderer->update(t);
    };

    for(auto componentId : _graphModel->graph().componentIds())
        update(componentId, t);

    if(!_transition.finished())
    {
        for(auto componentId : _transitionComponentIds)
            update(componentId, t);

        _transition.update(t);
    }
}

void GraphOverviewScene::render()
{
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _funcs->glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    auto render = [this](ComponentId componentId)
    {
        auto renderer = rendererForComponentId(componentId);
        auto layoutData = _componentLayout[componentId];

        renderer->render(layoutData._rect.x(),
                         layoutData._rect.y(),
                         layoutData._rect.width(),
                         layoutData._rect.height(),
                         layoutData._alpha);
    };

    for(auto componentId : _graphModel->graph().componentIds())
    {
        bool componentIdIsMerger = std::any_of(
                    _componentMergeSets.cbegin(),
                    _componentMergeSets.cend(),
                    [componentId](const ComponentMergeSet& componentMergeSet)
        {
            return componentMergeSet.newComponentId() == componentId;
        });


        if(_transition.finished() || !componentIdIsMerger)
            render(componentId);
    }

    if(!_transition.finished())
    {
        for(auto componentId : _transitionComponentIds)
            render(componentId);
    }
}

void GraphOverviewScene::resize(int width, int height)
{
    _width = width;
    _height = height;

    int size = height;

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto renderer = rendererForComponentId(componentId);
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

    layoutComponents();
}

void GraphOverviewScene::onShow()
{
    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto renderer = rendererForComponentId(componentId);
        renderer->setVisible(true);
    }
}

void GraphOverviewScene::onHide()
{
    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto renderer = rendererForComponentId(componentId);
        renderer->setVisible(false);
    }
}

void GraphOverviewScene::zoom(float delta)
{
    if(delta > 0.0f)
        setRenderSizeDivisor(_renderSizeDivisor / 2);
    else
        setRenderSizeDivisor(_renderSizeDivisor * 2);
}

void GraphOverviewScene::setRenderSizeDivisor(int divisor)
{
    if(divisor < 1)
        divisor = 1;

    _renderSizeDivisor = divisor;

    resize(_width, _height);
}

void GraphOverviewScene::layoutComponents()
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

        auto renderer = rendererForComponentId(componentId);

        if(!coords.empty() && (coord.x() + renderer->width() > coords.top().x() ||
            coord.y() + renderer->height() > _height))
        {
            coord = coords.top();
            coords.pop();
        }

        auto rect = QRect(coord.x(), coord.y(), renderer->width(), renderer->height());
        _componentLayout[componentId] = LayoutData(rect, 1.0f);

        QPoint right(coord.x() + renderer->width(), coord.y());
        QPoint down(coord.x(), coord.y() + renderer->height());

        if(coords.empty() || right.x() < coords.top().x())
            coords.emplace(right);

        if(down.y() < _height)
            coords.emplace(down);
    }
}

static GraphOverviewScene::LayoutData interpolateLayout(const GraphOverviewScene::LayoutData& a,
                                                        const GraphOverviewScene::LayoutData& b,
                                                        float f)
{
    return GraphOverviewScene::LayoutData(QRect(
        Utils::interpolate(a._rect.left(),   b._rect.left(),   f),
        Utils::interpolate(a._rect.top(),    b._rect.top(),    f),
        Utils::interpolate(a._rect.width(),  b._rect.width(),  f),
        Utils::interpolate(a._rect.height(), b._rect.height(), f)
        ), Utils::interpolate(a._alpha, b._alpha, f));
}

void GraphOverviewScene::startTransition()
{
    for(auto componentMergeSet : _componentMergeSets)
    {
        auto newComponentId = componentMergeSet.newComponentId();

        for(auto merger : componentMergeSet.mergers())
            _componentLayout[merger] = _componentLayout[newComponentId];
    }

    auto targetComponentLayout = _componentLayout;

    _transition.start(GraphComponentRenderer::TRANSITION_DURATION, Transition::Type::EaseInEaseOut,
    [this, targetComponentLayout /*FIXME C++14 move capture*/](float f)
    {
        auto interpolate = [&](const ComponentId componentId)
        {
            _componentLayout[componentId] = interpolateLayout(
                    _previousComponentLayout[componentId],
                    targetComponentLayout[componentId], f);
        };

        for(auto componentId : _graphModel->graph().componentIds())
            interpolate(componentId);

        for(auto componentId : _transitionComponentIds)
            interpolate(componentId);
    },
    [this]
    {
        for(auto componentId : _transitionComponentIds)
        {
            auto renderer = rendererForComponentId(componentId);
            renderer->thaw();
        }
    });
}

void GraphOverviewScene::onComponentAdded(const Graph*, ComponentId componentId, bool hasSplit)
{
    if(!hasSplit)
        _previousComponentLayout[componentId]._alpha = 0.0f;
}

void GraphOverviewScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool hasMerged)
{
    if(!hasMerged)
    {
        auto renderer = rendererForComponentId(componentId);
        renderer->freeze();

        _transitionComponentIds.emplace_back(componentId);
    }
}

void GraphOverviewScene::onComponentSplit(const Graph*, const ComponentSplitSet& componentSplitSet)
{
    _graphWidget->executeOnRendererThread([this, componentSplitSet]
    {
        if(visible())
        {
            auto oldComponentId = componentSplitSet.oldComponentId();
            auto oldGraphComponentRenderer = rendererForComponentId(oldComponentId);

            for(auto splitter : componentSplitSet.splitters())
            {
                auto renderer = rendererForComponentId(splitter);
                renderer->cloneViewDataFrom(*oldGraphComponentRenderer);
                _previousComponentLayout[splitter] = _componentLayout[oldComponentId];
            }
        }
    }, "GraphOverviewScene::onComponentSplit (cloneCameraDataFrom, component layout)");
}

void GraphOverviewScene::onComponentsWillMerge(const Graph*, const ComponentMergeSet& componentMergeSet)
{
    _componentMergeSets.emplace_back(componentMergeSet);

    for(auto merger : componentMergeSet.mergers())
    {
        auto renderer = rendererForComponentId(merger);
        renderer->freeze();

        _transitionComponentIds.emplace_back(merger);
    }
}

void GraphOverviewScene::onGraphWillChange(const Graph*)
{
    // Take a copy of the existing layout before the graph is changed
    _previousComponentLayout = _componentLayout;

    _transitionComponentIds.clear();
    _componentMergeSets.clear();
}

void GraphOverviewScene::onGraphChanged(const Graph* graph)
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
            onShow();
            startTransition();
        }, "GraphOverviewScene::onGraphChanged (resize)");
    }
}
