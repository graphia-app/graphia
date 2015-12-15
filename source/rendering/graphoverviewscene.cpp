#include "graphoverviewscene.h"
#include "graphrenderer.h"
#include "graphcomponentrenderer.h"

#include "../graph/graphmodel.h"

#include "../ui/graphquickitem.h"

#include "../utils/utils.h"

#include <QPoint>

#include <stack>
#include <vector>
#include <functional>
#include <algorithm>

GraphOverviewScene::GraphOverviewScene(GraphRenderer* graphRenderer) :
    Scene(graphRenderer),
    OpenGLFunctions(),
    _graphRenderer(graphRenderer),
    _graphModel(graphRenderer->graphModel()),
    _renderSizeDivisors(_graphModel->graph()),
    _previousComponentLayout(_graphModel->graph()),
    _componentLayout(_graphModel->graph())
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
    resolveOpenGLFunctions();
}

void GraphOverviewScene::update(float t)
{
    for(auto componentId : _componentIds)
    {
        auto* renderer = _graphRenderer->componentRendererForId(componentId);
        auto& layoutData = _componentLayout[componentId];
        renderer->setDimensions(layoutData._rect);
        renderer->setAlpha(layoutData._alpha);
        renderer->update(t);
    }
}

void GraphOverviewScene::onShow()
{
    for(auto componentId : _componentIds)
    {
        auto renderer = _graphRenderer->componentRendererForId(componentId);
        renderer->setVisible(true);
    }
}

void GraphOverviewScene::onHide()
{
    for(auto componentId : _componentIds)
    {
        auto renderer = _graphRenderer->componentRendererForId(componentId);
        renderer->setVisible(false);
    }
}

void GraphOverviewScene::zoom(float delta)
{
    if(delta > 0.0f)
        setRenderSizeDivisor(_renderSizeDivisor / 2);
    else if(_renderSizeDivisor < 64)
        setRenderSizeDivisor(_renderSizeDivisor * 2);
}

void GraphOverviewScene::setRenderSizeDivisor(int divisor)
{
    if(divisor < 1)
        divisor = 1;

    _renderSizeDivisor = divisor;

    setViewportSize(_width, _height);
}

void GraphOverviewScene::startTransitionFromComponentMode(ComponentId focusComponentId,
                                                          float duration,
                                                          Transition::Type transitionType,
                                                          std::function<void()> finishedFunction)
{
    startTransition(duration, transitionType, finishedFunction);
    _previousComponentLayout = _componentLayout;

    for(auto componentId : _componentIds)
    {
        if(componentId != focusComponentId)
            _previousComponentLayout[componentId]._alpha = 0.0f;
    }

    int left = (_width * 0.5f) - (_height * 0.5f);
    _previousComponentLayout[focusComponentId]._rect = QRect(left, 0, _height, _height);
}

void GraphOverviewScene::startTransitionToComponentMode(ComponentId focusComponentId,
                                                        float duration,
                                                        Transition::Type transitionType,
                                                        std::function<void()> finishedFunction)
{
    _previousComponentLayout = _componentLayout;

    for(auto componentId : _componentIds)
    {
        if(componentId != focusComponentId)
            _componentLayout[componentId]._alpha = 0.0f;
    }

    int left = (_width * 0.5f) - (_height * 0.5f);
    _componentLayout[focusComponentId]._rect = QRect(left, 0, _height, _height);

    startTransition(duration, transitionType, finishedFunction);
}

void GraphOverviewScene::layoutComponents()
{
    std::stack<QPoint> coords;

    std::vector<ComponentId> sortedComponentIds = _componentIds;
    std::sort(sortedComponentIds.begin(), sortedComponentIds.end(),
              [this](const ComponentId& a, const ComponentId& b)
    {
        return _renderSizeDivisors[a] < _renderSizeDivisors[b];
    });

    //FIXME this is a mess
    coords.emplace(0, 0);
    for(auto componentId : sortedComponentIds)
    {
        auto coord = coords.top();
        coords.pop();

        int divisor = _renderSizeDivisors[componentId];
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
    for(auto componentId : _componentIds)
    {
        if(_previousComponentLayout[componentId]._alpha == 0.0f)
        {
            _previousComponentLayout[componentId]._rect =
                    _componentLayout[componentId]._rect;

            auto renderer = _graphRenderer->componentRendererForId(componentId);
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

void GraphOverviewScene::setViewportSize(int width, int height)
{
    _width = width;
    _height = height;

    layoutComponents();

    for(auto componentId : _componentIds)
    {
        auto renderer = _graphRenderer->componentRendererForId(componentId);
        auto& layoutData = _componentLayout[componentId];
        renderer->setViewportSize(_width, _height);
        renderer->setDimensions(layoutData._rect);
    }
}

bool GraphOverviewScene::transitionActive() const
{
    for(auto componentId : _componentIds)
    {
        auto renderer = _graphRenderer->componentRendererForId(componentId);

        if(renderer->transitionActive())
            return true;
    }

    return false;
}

static GraphOverviewScene::LayoutData interpolateLayout(const GraphOverviewScene::LayoutData& a,
                                                        const GraphOverviewScene::LayoutData& b,
                                                        float f)
{
    return GraphOverviewScene::LayoutData(QRect(
        u::interpolate(a._rect.left(),   b._rect.left(),   f),
        u::interpolate(a._rect.top(),    b._rect.top(),    f),
        u::interpolate(a._rect.width(),  b._rect.width(),  f),
        u::interpolate(a._rect.height(), b._rect.height(), f)
        ), u::interpolate(a._alpha, b._alpha, f));
}

void GraphOverviewScene::startTransition(float duration,
                                         Transition::Type transitionType,
                                         std::function<void()> finishedFunction)
{
    auto targetComponentLayout = _componentLayout;

    if(!_graphRenderer->transition().active())
        _graphRenderer->rendererStartedTransition();

    _graphRenderer->transition().start(duration, transitionType,
    [this, targetComponentLayout /*FIXME C++14 move capture*/](float f)
    {
        auto interpolate = [&](const ComponentId componentId)
        {
            _componentLayout[componentId] = interpolateLayout(
                    _previousComponentLayout[componentId],
                    targetComponentLayout[componentId], f);

            _graphRenderer->componentRendererForId(componentId)->updateTransition(f);
        };

        for(auto componentId : _componentIds)
            interpolate(componentId);
    },
    [this]
    {
        _previousComponentLayout = _componentLayout;

        for(auto componentId : _removedComponentIds)
        {
            auto renderer = _graphRenderer->componentRendererForId(componentId);
            renderer->thaw();
        }

        for(auto componentMergeSet : _componentMergeSets)
        {
            auto renderer = _graphRenderer->componentRendererForId(componentMergeSet.newComponentId());
            renderer->thaw();
        }

        _graphRenderer->thaw();

        // Subtract the removed ComponentIds, no we no longer need to render them
        std::vector<ComponentId> postTransitionComponentIds;
        std::sort(_componentIds.begin(), _componentIds.end());
        std::sort(_removedComponentIds.begin(), _removedComponentIds.end());
        std::set_difference(_componentIds.begin(), _componentIds.end(),
                            _removedComponentIds.begin(), _removedComponentIds.end(),
                            std::inserter(postTransitionComponentIds, postTransitionComponentIds.begin()));

        _componentIds = std::move(postTransitionComponentIds);

        _removedComponentIds.clear();
        _componentMergeSets.clear();

        _graphRenderer->rendererFinishedTransition();
    },
    finishedFunction);

    // Reset all components by default
    for(auto componentId : _componentIds)
    {
        auto renderer = _graphRenderer->componentRendererForId(componentId);
        renderer->resetView();
    }

    for(auto componentMergeSet : _componentMergeSets)
    {
        auto mergedComponent = _graphModel->graph().componentById(componentMergeSet.newComponentId());
        auto mergedNodeIds = mergedComponent->nodeIds();
        auto mergedFocusPosition = NodePositions::centreOfMassScaledAndSmoothed(_graphModel->nodePositions(),
                                                                                mergedNodeIds);

        // Use the rotation of the new component
        auto renderer = _graphRenderer->componentRendererForId(componentMergeSet.newComponentId());
        QQuaternion rotation = renderer->camera()->rotation();

        for(auto merger : componentMergeSet.mergers())
        {
            auto renderer = _graphRenderer->componentRendererForId(merger);
            renderer->moveFocusToPositionContainingNodes(mergedFocusPosition, mergedNodeIds, rotation);
        }
    }
}

void GraphOverviewScene::onComponentAdded(const Graph*, ComponentId componentId, bool hasSplit)
{
    if(!hasSplit)
    {
        _graphRenderer->executeOnRendererThread([this, componentId]
        {
            _previousComponentLayout[componentId]._alpha = 0.0f;
        }, "GraphOverviewScene::onComponentAdded (set source alpha to 0)");
    }
}

void GraphOverviewScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool hasMerged)
{
    if(!visible())
        return;

    if(!hasMerged)
    {
        _graphRenderer->executeOnRendererThread([this, componentId]
        {
            auto renderer = _graphRenderer->componentRendererForId(componentId);
            renderer->freeze();
            _graphRenderer->freeze();

            _removedComponentIds.emplace_back(componentId);
            _componentLayout[componentId]._alpha = 0.0f;
        }, "GraphOverviewScene::onComponentWillBeRemoved (freeze renderer, set target alpha to 0)");
    }
}

void GraphOverviewScene::onComponentSplit(const Graph*, const ComponentSplitSet& componentSplitSet)
{
    if(!visible())
        return;

    _graphRenderer->executeOnRendererThread([this, componentSplitSet]
    {
        if(visible())
        {
            auto oldComponentId = componentSplitSet.oldComponentId();
            auto oldGraphComponentRenderer = _graphRenderer->componentRendererForId(oldComponentId);

            for(auto splitter : componentSplitSet.splitters())
            {
                auto renderer = _graphRenderer->componentRendererForId(splitter);
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

    _graphRenderer->executeOnRendererThread([this, componentMergeSet]
    {
        _componentMergeSets.emplace_back(componentMergeSet);

        for(auto merger : componentMergeSet.mergers())
        {
            auto renderer = _graphRenderer->componentRendererForId(merger);
            renderer->freeze();

            if(merger != componentMergeSet.newComponentId())
                _removedComponentIds.emplace_back(merger);
        }

        _graphRenderer->freeze();
    }, "GraphOverviewScene::onComponentsWillMerge (freeze renderers)");
}

void GraphOverviewScene::onGraphWillChange(const Graph*)
{
    // Take a copy of the existing layout before the graph is changed
    _previousComponentLayout = _componentLayout;
}

void GraphOverviewScene::onGraphChanged(const Graph* graph)
{
    _graphRenderer->executeOnRendererThread([this, graph]
    {
        _componentIds = graph->componentIds();

        // Find the number of nodes in the largest component
        int maxNumNodes = 0;

        if(graph->numComponents() > 0)
            maxNumNodes = graph->componentById(graph->componentIdOfLargestComponent())->numNodes();

        for(auto componentId : _componentIds)
        {
            auto component = graph->componentById(componentId);
            int divisor = maxNumNodes / component->numNodes();
            _renderSizeDivisors[componentId] = u::smallestPowerOf2GreaterThan(divisor);
        }

        if(visible())
        {
            onShow();
            setViewportSize(_width, _height);

            startTransition(1.0f, Transition::Type::EaseInEaseOut,
            [this, graph]
            {
                // If graph change has resulted in a single component, switch
                // to component mode once the transition had completed
                if(graph->numComponents() == 1)
                    _graphRenderer->switchToComponentMode();
            });
        }

        // We still need to render any components that have been removed, while they
        // transition away
        _componentIds.insert(_componentIds.end(),
                             _removedComponentIds.begin(),
                             _removedComponentIds.end());
    }, "GraphOverviewScene::onGraphChanged");
}
