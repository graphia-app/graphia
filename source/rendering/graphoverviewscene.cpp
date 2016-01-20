#include "graphoverviewscene.h"
#include "graphrenderer.h"
#include "graphcomponentrenderer.h"

#include "../graph/graphmodel.h"

#include "../layout/powerof2gridcomponentlayout.h"

#include "../ui/graphquickitem.h"

#include "../utils/utils.h"

#include <QPoint>

#include <stack>
#include <vector>
#include <functional>
#include <algorithm>

GraphOverviewScene::GraphOverviewScene(GraphRenderer* graphRenderer) :
    Scene(graphRenderer),
    _graphRenderer(graphRenderer),
    _graphModel(graphRenderer->graphModel()),
    _previousComponentAlpha(_graphModel->graph(), 1.0f),
    _componentAlpha(_graphModel->graph(), 1.0f),
    _componentLayoutData(_graphModel->graph()),
    _previousZoomedComponentLayoutData(_graphModel->graph()),
    _zoomedComponentLayoutData(_graphModel->graph()),
    _componentLayout(std::make_shared<PowerOf2GridComponentLayout>())
{
    connect(&_graphModel->graph(), &Graph::componentAdded, this, &GraphOverviewScene::onComponentAdded, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentWillBeRemoved, this, &GraphOverviewScene::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &GraphOverviewScene::onComponentSplit, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentsWillMerge, this, &GraphOverviewScene::onComponentsWillMerge, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &GraphOverviewScene::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphOverviewScene::onGraphChanged, Qt::DirectConnection);
}

void GraphOverviewScene::update(float t)
{
    for(auto componentId : _componentIds)
    {
        auto* renderer = _graphRenderer->componentRendererForId(componentId);
        renderer->setDimensions(_zoomedComponentLayoutData[componentId]);
        renderer->setAlpha(_componentAlpha[componentId]);
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

void GraphOverviewScene::pan(float dx, float dy)
{
    float scaledDx = dx / (_zoomFactor);
    float scaledDy = dy / (_zoomFactor);

    _offset.setX(_offset.x() - scaledDx);
    _offset.setY(_offset.y() - scaledDy);

    updateZoomedComponentLayoutData();
}

void GraphOverviewScene::zoom(float delta, float x, float y)
{
    float nx = x / _width;
    float ny = y / _height;

    float oldCentreX = (nx * _width) / _zoomFactor;
    float oldCentreY = (ny * _height) / _zoomFactor;

    if(delta > 0.0f)
        _zoomFactor *= 1.25f;
    else
        _zoomFactor *= 0.8f;

    float newCentreX = (nx * _width) / _zoomFactor;
    float newCentreY = (ny * _height) / _zoomFactor;

    _offset.setX(_offset.x() + (oldCentreX - newCentreX));
    _offset.setY(_offset.y() + (oldCentreY - newCentreY));

    _zoomCentre.setX(newCentreX);
    _zoomCentre.setY(newCentreY);

    updateZoomedComponentLayoutData();
}

QRectF GraphOverviewScene::zoomedRect(const QRectF& rect)
{
    QRectF newRect(rect);

    newRect.translate(-_offset.x(), -_offset.y());

    newRect.translate(-_zoomCentre.x(), -_zoomCentre.y());

    newRect.setLeft(newRect.left() * _zoomFactor);
    newRect.setRight(newRect.right() * _zoomFactor);
    newRect.setTop(newRect.top() * _zoomFactor);
    newRect.setBottom(newRect.bottom() * _zoomFactor);

    newRect.translate(_zoomCentre.x() * _zoomFactor, _zoomCentre.y() * _zoomFactor);

    return newRect;
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
    _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;

    for(auto componentId : _componentIds)
    {
        if(componentId != focusComponentId)
            _previousComponentAlpha[componentId] = 0.0f;
    }

    float left = (_width * 0.5f) - (_height * 0.5f);
    _previousZoomedComponentLayoutData[focusComponentId] = QRectF(left, 0.0f, _height, _height);
}

void GraphOverviewScene::startTransitionToComponentMode(ComponentId focusComponentId,
                                                        float duration,
                                                        Transition::Type transitionType,
                                                        std::function<void()> finishedFunction)
{
    _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;

    for(auto componentId : _componentIds)
    {
        if(componentId != focusComponentId)
            _componentAlpha[componentId] = 0.0f;
    }

    float left = (_width * 0.5f) - (_height * 0.5f);
    _zoomedComponentLayoutData[focusComponentId] = QRectF(left, 0, _height, _height);

    startTransition(duration, transitionType, finishedFunction);
}

void GraphOverviewScene::updateZoomedComponentLayoutData()
{
    for(auto componentId : _componentIds)
        _zoomedComponentLayoutData[componentId] = zoomedRect(_componentLayoutData[componentId]);
}

void GraphOverviewScene::layoutComponents()
{
    _componentLayout->execute(_graphModel->graph(), _componentIds,
                              _width, _height, _componentLayoutData);

    updateZoomedComponentLayoutData();

    for(auto componentId : _componentIds)
    {
        _componentAlpha[componentId] = 1.0f;

        // If the component is fading in, keep it in a fixed position
        if(_previousComponentAlpha[componentId] == 0.0f)
        {
            _previousZoomedComponentLayoutData[componentId] = _zoomedComponentLayoutData[componentId];
            _previousComponentAlpha[componentId] = _componentAlpha[componentId];

            auto renderer = _graphRenderer->componentRendererForId(componentId);
            renderer->resetView();
        }
    }

    // Give the mergers the same layout as the new component
    for(auto componentMergeSet : _componentMergeSets)
    {
        auto newComponentId = componentMergeSet.newComponentId();

        for(auto merger : componentMergeSet.mergers())
        {
            _zoomedComponentLayoutData[merger] = _zoomedComponentLayoutData[newComponentId];
            _componentAlpha[merger] = _componentAlpha[newComponentId];
        }
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
        renderer->setViewportSize(_width, _height);
        renderer->setDimensions(_zoomedComponentLayoutData[componentId]);
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

static QRectF interpolateRect(const QRectF& a, const QRectF& b, float f)
{
    return QRectF(
        u::interpolate(a.left(),   b.left(),   f),
        u::interpolate(a.top(),    b.top(),    f),
        u::interpolate(a.width(),  b.width(),  f),
        u::interpolate(a.height(), b.height(), f)
        );
}

void GraphOverviewScene::startTransition(float duration,
                                         Transition::Type transitionType,
                                         std::function<void()> finishedFunction)
{
    auto targetComponentLayout = _zoomedComponentLayoutData;
    auto targetComponentAlpha = _componentAlpha;

    if(!_graphRenderer->transition().active())
        _graphRenderer->rendererStartedTransition();

    _graphRenderer->transition().start(duration, transitionType,
    [this, targetComponentLayout, targetComponentAlpha /*FIXME C++14 move capture*/](float f)
    {
        auto interpolate = [&](const ComponentId componentId)
        {
            _zoomedComponentLayoutData[componentId] = interpolateRect(_previousZoomedComponentLayoutData[componentId],
                                                            targetComponentLayout[componentId], f);
            _componentAlpha[componentId] = u::interpolate(_previousComponentAlpha[componentId],
                                                          targetComponentAlpha[componentId], f);

            _graphRenderer->componentRendererForId(componentId)->updateTransition(f);
        };

        for(auto componentId : _componentIds)
            interpolate(componentId);
    },
    [this]
    {
        _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;
        _previousComponentAlpha = _componentAlpha;

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
        _graphRenderer->sceneFinishedTransition();
    },
    finishedFunction);

    // Reset all components by default
    for(auto componentId : _componentIds)
    {
        auto renderer = _graphRenderer->componentRendererForId(componentId);
        renderer->resetView();
    }

    for(auto& componentMergeSet : _componentMergeSets)
    {
        auto mergedComponent = _graphModel->graph().componentById(componentMergeSet.newComponentId());
        auto& mergedNodeIds = mergedComponent->nodeIds();
        auto mergedFocusPosition = NodePositions::centreOfMassScaledAndSmoothed(_graphModel->nodePositions(),
                                                                                mergedNodeIds);

        // Use the rotation of the new component
        auto renderer = _graphRenderer->componentRendererForId(componentMergeSet.newComponentId());
        QQuaternion rotation = renderer->camera()->rotation();
        auto maxDistance = GraphComponentRenderer::maxNodeDistanceFromPoint(*_graphModel,
                                                                            mergedFocusPosition,
                                                                            mergedNodeIds);

        for(auto merger : componentMergeSet.mergers())
        {
            renderer = _graphRenderer->componentRendererForId(merger);
            renderer->moveFocusToPositionAndRadius(mergedFocusPosition, maxDistance, rotation);
        }
    }
}

void GraphOverviewScene::onComponentAdded(const Graph*, ComponentId componentId, bool hasSplit)
{
    if(!hasSplit)
    {
        _graphRenderer->executeOnRendererThread([this, componentId]
        {
            _previousComponentAlpha[componentId] = 0.0f;
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

            _removedComponentIds.emplace_back(componentId);
            _componentAlpha[componentId] = 0.0f;
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
                _previousZoomedComponentLayoutData[splitter] = _zoomedComponentLayoutData[oldComponentId];
                _previousComponentAlpha[splitter] = _componentAlpha[oldComponentId];
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
    }, "GraphOverviewScene::onComponentsWillMerge (freeze renderers)");
}

void GraphOverviewScene::onGraphWillChange(const Graph*)
{
    // Take a copy of the existing layout before the graph is changed
    _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;
}

void GraphOverviewScene::onGraphChanged(const Graph* graph)
{
    _graphRenderer->executeOnRendererThread([this, graph]
    {
        _componentIds = graph->componentIds();

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
