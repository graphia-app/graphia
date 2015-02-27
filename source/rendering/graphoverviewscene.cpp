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
    _graphWidget->transition().update(t);

    auto update = [this](ComponentId componentId, float t)
    {
        auto renderer = rendererForComponentId(componentId);
        renderer->update(t);
    };

    for(auto componentId : _graphModel->graph().componentIds())
        update(componentId, t);

    if(!_graphWidget->transition().finished())
    {
        for(auto componentId : _transitionComponentIds)
            update(componentId, t);
    }
}

void GraphOverviewScene::render()
{
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _funcs->glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
    _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    _graphWidget->clearScene();

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

    std::unique_lock<std::mutex> lock(_cachedComponentIdsMutex);
    if(!_cachedComponentIds.empty())
    {
        for(auto componentId : _cachedComponentIds)
            render(componentId);

        lock.unlock();
    }
    else
    {
        lock.unlock();

        for(auto componentId : _graphModel->graph().componentIds())
            render(componentId);

        for(auto componentId : _transitionComponentIds)
            render(componentId);
    }

    _graphWidget->renderScene();
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

void GraphOverviewScene::resetView()
{
    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto renderer = rendererForComponentId(componentId);
        renderer->resetView();
    }
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

        int divisor =_renderSizeDivisors[componentId];
        int dividedSize = _height / (divisor * _renderSizeDivisor);

        const int MINIMUM_SIZE = 32;
        if(_height > MINIMUM_SIZE)
        {
            while(dividedSize < MINIMUM_SIZE && divisor > 1)
            {
                divisor /= 2;
                dividedSize = _height / (divisor * _renderSizeDivisor);
            }
        }

        if(!coords.empty() && (coord.x() + dividedSize > coords.top().x() ||
            coord.y() + dividedSize > _height))
        {
            coord = coords.top();
            coords.pop();
        }

        auto rect = QRect(coord.x(), coord.y(), dividedSize, dividedSize);
        _componentLayout[componentId] = LayoutData(rect, 1.0f);

        QPoint right(coord.x() + dividedSize, coord.y());
        QPoint down(coord.x(), coord.y() + dividedSize);

        if(coords.empty() || right.x() < coords.top().x())
            coords.emplace(right);

        if(down.y() < _height)
            coords.emplace(down);
    }

    // If the component is fading in, keep it in a fixed position
    for(auto componentId : _graphModel->graph().componentIds())
    {
        if(_previousComponentLayout[componentId]._alpha == 0.0f)
        {
            _previousComponentLayout[componentId]._rect =
                    _componentLayout[componentId]._rect;

            auto renderer = rendererForComponentId(componentId);
            renderer->resetView();
        }
    }

    for(auto componentMergeSet : _componentMergeSets)
    {
        auto newComponentId = componentMergeSet.newComponentId();

        for(auto merger : componentMergeSet.mergers())
            _componentLayout[merger] = _componentLayout[newComponentId];
    }
}

void GraphOverviewScene::resize(int width, int height)
{
    _width = width;
    _height = height;

    _graphWidget->resizeScene(width, height);

    layoutComponents();

    auto resizeComponent = [this](ComponentId componentId)
    {
        auto renderer = rendererForComponentId(componentId);
        auto layoutData = _componentLayout[componentId];
        renderer->resize(_width, _height,
                         layoutData._rect.width(),
                         layoutData._rect.height());
    };

    std::unique_lock<std::mutex> lock(_cachedComponentIdsMutex);
    if(!_cachedComponentIds.empty())
    {
        for(auto componentId : _cachedComponentIds)
            resizeComponent(componentId);

        lock.unlock();
    }
    else
    {
        lock.unlock();

        for(auto componentId : _graphModel->graph().componentIds())
            resizeComponent(componentId);

        for(auto componentId : _transitionComponentIds)
            resizeComponent(componentId);
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
    resize(_width, _height);

    auto targetComponentLayout = _componentLayout;

    if(_graphWidget->transition().finished())
        _graphWidget->rendererStartedTransition();

    _graphWidget->transition().start(1.0f, Transition::Type::EaseInEaseOut,
    [this, targetComponentLayout /*FIXME C++14 move capture*/](float f)
    {
        auto interpolate = [&](const ComponentId componentId)
        {
            _componentLayout[componentId] = interpolateLayout(
                    _previousComponentLayout[componentId],
                    targetComponentLayout[componentId], f);

            rendererForComponentId(componentId)->updateTransition(f);
        };

        for(auto componentId : _graphModel->graph().componentIds())
            interpolate(componentId);

        for(auto componentId : _transitionComponentIds)
            interpolate(componentId);
    },
    [this]
    {
        _previousComponentLayout = _componentLayout;

        for(auto componentId : _transitionComponentIds)
        {
            auto renderer = rendererForComponentId(componentId);
            renderer->thaw();
        }

        for(auto componentMergeSet : _componentMergeSets)
        {
            auto renderer = rendererForComponentId(componentMergeSet.newComponentId());
            renderer->thaw();
        }

        _transitionComponentIds.clear();
        _componentSplitSets.clear();
        _componentMergeSets.clear();

        _graphWidget->rendererFinishedTransition();
    });

    for(auto componentSplitSet : _componentSplitSets)
    {
        for(auto splitter : componentSplitSet.splitters())
        {
            auto renderer = rendererForComponentId(splitter);
            renderer->resetView();
        }
    }

    for(auto componentMergeSet : _componentMergeSets)
    {
        auto mergedComponent = _graphModel->graph().componentById(componentMergeSet.newComponentId());
        auto mergedNodeIds = mergedComponent->nodeIds();
        auto mergedFocusPosition = NodePositions::centreOfMassScaled(_graphModel->nodePositions(),
                                                                     mergedNodeIds);

        for(auto merger : componentMergeSet.mergers())
        {
            auto renderer = rendererForComponentId(merger);
            renderer->moveFocusToPositionContainingNodes(mergedFocusPosition, mergedNodeIds);
        }
    }
}

void GraphOverviewScene::onComponentAdded(const Graph*, ComponentId componentId, bool hasSplit)
{
    if(!hasSplit)
        _previousComponentLayout[componentId]._alpha = 0.0f;
}

void GraphOverviewScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool hasMerged)
{
    if(!visible())
        return;

    if(!hasMerged)
    {
        auto renderer = rendererForComponentId(componentId);
        renderer->freeze();

        _transitionComponentIds.emplace_back(componentId);
        _componentLayout[componentId]._alpha = 0.0f;
    }
}

void GraphOverviewScene::onComponentSplit(const Graph*, const ComponentSplitSet& componentSplitSet)
{
    if(!visible())
        return;

    _componentSplitSets.emplace_back(componentSplitSet);

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
    if(!visible())
        return;

    _componentMergeSets.emplace_back(componentMergeSet);

    for(auto merger : componentMergeSet.mergers())
    {
        auto renderer = rendererForComponentId(merger);
        renderer->freeze();

        if(merger != componentMergeSet.newComponentId())
            _transitionComponentIds.emplace_back(merger);
    }
}

void GraphOverviewScene::onGraphWillChange(const Graph* graph)
{
    // Take a copy of the existing layout before the graph is changed
    _previousComponentLayout = _componentLayout;

    // The ComponentIds will be referenced when we render them, so
    // take a copy to use in case the list is altered during the change
    std::unique_lock<std::mutex> lock(_cachedComponentIdsMutex);
    _cachedComponentIds = graph->componentIds();
}

void GraphOverviewScene::onGraphChanged(const Graph* graph)
{
    std::unique_lock<std::mutex> lock(_cachedComponentIdsMutex);
    _cachedComponentIds.clear();
    lock.unlock();

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
            onShow();
            startTransition();
        }, "GraphOverviewScene::onGraphChanged (resize)");
    }
}
