/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphcomponentscene.h"
#include "graphrenderer.h"

#include "graphcomponentrenderer.h"

#include "shared/utils/scope_exit.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include "layout/nodepositions.h"

#include "ui/graphquickitem.h"

GraphComponentScene::GraphComponentScene(GraphRenderer* graphRenderer) :
    Scene(graphRenderer),
    _graphRenderer(graphRenderer)
{
    connect(&_graphRenderer->graphModel()->graph(), &Graph::componentSplit, this, &GraphComponentScene::onComponentSplit, Qt::DirectConnection);
    connect(&_graphRenderer->graphModel()->graph(), &Graph::componentsWillMerge, this, &GraphComponentScene::onComponentsWillMerge, Qt::DirectConnection);
    connect(&_graphRenderer->graphModel()->graph(), &Graph::componentAdded, this, &GraphComponentScene::onComponentAdded, Qt::DirectConnection);
    connect(&_graphRenderer->graphModel()->graph(), &Graph::componentWillBeRemoved, this, &GraphComponentScene::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&_graphRenderer->graphModel()->graph(), &Graph::graphWillChange, this, &GraphComponentScene::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphRenderer->graphModel()->graph(), &Graph::graphChanged, this, &GraphComponentScene::onGraphChanged, Qt::DirectConnection);

    // Use nodeRemovedFromComponent instead of nodeRemoved, becuse it is emitted after
    // componentWillBeRemoved; this is important for proper ordering of deferred rendering tasks
    connect(&_graphRenderer->graphModel()->graph(), &Graph::nodeRemovedFromComponent, this, &GraphComponentScene::onNodeRemoved, Qt::DirectConnection);

    _defaultComponentId = _graphRenderer->graphModel()->graph().componentIdOfLargestComponent();
}

void GraphComponentScene::update(float t)
{
    float offset = 0.0f;
    float outOffset = 0.0f;

    switch(_transitionStyle)
    {
    case TransitionStyle::SlideLeft:
        offset = (1.0f - _transitionValue) * static_cast<float>(_width);
        outOffset = offset - static_cast<float>(_width);
        break;

    case TransitionStyle::SlideRight:
        offset = -(1.0f - _transitionValue) * static_cast<float>(_width);
        outOffset = offset + static_cast<float>(_width);
        break;

    default:
        break;
    }

    // The static component, or the one transitioning in
    if(componentRenderer() != nullptr)
    {
        Q_ASSERT(componentRenderer()->initialised());

        componentRenderer()->setDimensions(
            QRect(static_cast<int>(offset), 0, _width, _height));

        if(_transitionStyle == TransitionStyle::Fade ||
                _transitionStyle == TransitionStyle::SlideLeft ||
                _transitionStyle == TransitionStyle::SlideRight)
            componentRenderer()->setAlpha(_transitionValue);
        else
            componentRenderer()->setAlpha(1.0f);

        componentRenderer()->update(t);
    }

    // The component transitioning out
    if(transitioningComponentRenderer() != nullptr &&
       transitioningComponentRenderer() != componentRenderer())
    {
        Q_ASSERT(transitioningComponentRenderer()->initialised());

        transitioningComponentRenderer()->setDimensions(
            QRect(static_cast<int>(outOffset), 0, _width, _height));

        if(_transitionStyle == TransitionStyle::Fade ||
                _transitionStyle == TransitionStyle::SlideLeft ||
                _transitionStyle == TransitionStyle::SlideRight)
            transitioningComponentRenderer()->setAlpha(1.0f - _transitionValue);
        else
            transitioningComponentRenderer()->setAlpha(1.0f);

        transitioningComponentRenderer()->update(t);
    }
}

void GraphComponentScene::setViewportSize(int width, int height)
{
    _width = width;
    _height = height;

    const auto& componentIds = _graphRenderer->graphModel()->graph().componentIds();
    for(auto componentId : componentIds)
    {
        auto* renderer = _graphRenderer->componentRendererForId(componentId);
        renderer->setDimensions(QRect(0, 0, width, height));
        renderer->setViewportSize(width, height);
    }
}

bool GraphComponentScene::transitionActive() const
{
    if(componentRenderer() != nullptr)
        return componentRenderer()->transitionActive();

    return false;
}

void GraphComponentScene::onShow()
{
    updateRendererVisibility();
}

void GraphComponentScene::finishComponentTransition(ComponentId componentId, bool doTransition)
{
    // KEEP IN MIND: at this point _componentId may refer to a frozen component that is no longer
    // in the graph, and is only being kept around to refer to its renderer; in other words don't
    // use it to query the graph itself, because it might not work

    auto transitionType = Transition::Type::InversePower;
    TransitionStyle transitionStyle = TransitionStyle::None;

    const auto& componentIds = _graphRenderer->graphModel()->graph().componentIds();

    if(!componentIds.empty())
    {
        // If we're wrapping around the range of componentIds, we need to slide in the opposite direction
        if(_componentId == componentIds.front() && componentId == componentIds.back())
            transitionStyle = TransitionStyle::SlideRight;
        else if(_componentId == componentIds.back() && componentId == componentIds.front())
            transitionStyle = TransitionStyle::SlideLeft;
        else if(!componentId.isNull() && !_componentId.isNull())
        {
            auto incomingComponentSize = _graphRenderer->graphModel()->graph().componentById(componentId)->numNodes();

            if(incomingComponentSize > _componentSize)
                transitionStyle = TransitionStyle::SlideRight;
            else if(incomingComponentSize < _componentSize)
                transitionStyle = TransitionStyle::SlideLeft;
            else if(componentId < _componentId)
                transitionStyle = TransitionStyle::SlideRight;
            else
                transitionStyle = TransitionStyle::SlideLeft;
        }
    }

    if(componentId.isNull())
        _componentId = _defaultComponentId;
    else
        _componentId = componentId;

    if(_componentId.isNull() || _transitioningComponentId.isNull())
    {
        transitionType = Transition::Type::EaseInEaseOut;
        transitionStyle = TransitionStyle::Fade;
    }

    if(!_componentId.isNull())
    {
        Q_ASSERT(_graphRenderer->graphModel()->graph().containsComponentId(_componentId));
        _componentSize = _graphRenderer->graphModel()->graph().componentById(_componentId)->numNodes();
    }

    if(!_componentId.isNull() && !_transitioningComponentId.isNull())
        componentRenderer()->resetView();

    if(doTransition)
    {
        _transitionStyle = transitionStyle;
        _graphRenderer->transition().start(0.3f, transitionType,
        [this](float f)
        {
            _transitionValue = f;
        }).then(
        [this]
        {
            _transitionValue = 0.0f;
            _transitionStyle = TransitionStyle::None;

            if(transitioningComponentRenderer() != nullptr)
                transitioningComponentRenderer()->thaw();

            _transitioningComponentId.setToNull();
            updateRendererVisibility();

            if(!savedViewIsReset() && _queuedTransitionNodeId.isNull())
            {
                _graphRenderer->executeOnRendererThread([this]
                {
                    _graphRenderer->transition().willBeImmediatelyReused();
                    startTransition().then([this] { performQueuedTransition(); });
                    restoreViewData();
                }, QStringLiteral("GraphComponentScene::finishComponentTransition (restoreViewData)"));
            }
            else
                performQueuedTransition();
        });
    }

    updateRendererVisibility();
}

void GraphComponentScene::finishComponentTransitionOnRendererThread(ComponentId componentId, bool doTransition)
{
    _graphRenderer->executeOnRendererThread([this, componentId, doTransition]
    {
        finishComponentTransition(componentId, doTransition);
    }, QStringLiteral("GraphComponentScene::finishComponentTransition"));
}

void GraphComponentScene::performQueuedTransition()
{
    if(!_queuedTransitionNodeId.isNull())
    {
        _graphRenderer->executeOnRendererThread([this, nodeId = _queuedTransitionNodeId]
        {
            moveFocusToNode(nodeId);
        }, QStringLiteral("GraphComponentScene::performQueuedTransition"));

        _queuedTransitionNodeId.setToNull();
    }
}

bool GraphComponentScene::componentTransitionActive() const
{
    return !_transitioningComponentId.isNull();
}

void GraphComponentScene::setComponentId(ComponentId componentId, bool doTransition)
{
    _beingRemoved = false;

    // Do nothing if component already focused
    if(!componentId.isNull() && componentId == _componentId)
        return;

    if(visible())
        saveViewData();

    if(doTransition)
    {
        _transitioningComponentId = _componentId;
        if(!componentId.isNull() && !viewIsReset())
        {
            startTransition().then(
            [this, componentId]
            {
                _graphRenderer->transition().willBeImmediatelyReused();
                finishComponentTransitionOnRendererThread(componentId, true);
            });

            resetView(false);
        }
        else
            finishComponentTransitionOnRendererThread(componentId, true);
    }
    else
        finishComponentTransition(componentId, false);
}

void GraphComponentScene::saveViewData() const
{
    if(componentRenderer() != nullptr)
        componentRenderer()->saveViewData();
}

bool GraphComponentScene::savedViewIsReset() const
{
    if(componentRenderer() == nullptr)
        return true;

    return componentRenderer()->savedViewIsReset();
}

void GraphComponentScene::restoreViewData() const
{
    if(componentRenderer() != nullptr)
        componentRenderer()->restoreViewData();
}

void GraphComponentScene::resetView(bool doTransition)
{
    if(componentRenderer() != nullptr)
    {
        if(doTransition)
            startTransition();

        componentRenderer()->resetView();
    }
}

bool GraphComponentScene::viewIsReset() const
{
    if(componentRenderer() == nullptr)
        return true;

    return componentRenderer()->viewIsReset();
}

// NOLINTNEXTLINE readability-make-member-function-const
void GraphComponentScene::pan(NodeId clickedNodeId, const QPoint& start, const QPoint& end)
{
    Camera* camera = componentRenderer()->camera();
    QVector3D pointOnTranslationPlane;

    if(!clickedNodeId.isNull())
        pointOnTranslationPlane = _graphRenderer->graphModel()->nodePositions().get(clickedNodeId);
    else
        pointOnTranslationPlane = componentRenderer()->focusPosition();

    Plane translationPlane(pointOnTranslationPlane, camera->viewVector());

    QVector3D prevPoint = translationPlane.rayIntersection(
                camera->rayForViewportCoordinates(start.x(), start.y()));
    QVector3D curPoint = translationPlane.rayIntersection(
                camera->rayForViewportCoordinates(end.x(), end.y()));
    QVector3D newFocus = camera->focus() + (prevPoint - curPoint);

    camera->setFocus(newFocus);
}

bool GraphComponentScene::focusedOnNodeAtRadius(NodeId nodeId, float radius) const
{
    if(_componentId.isNull())
        return false;

    return componentRenderer()->focusedOnNodeAtRadius(nodeId, radius);
}

void GraphComponentScene::moveFocusToNode(NodeId nodeId, float radius)
{
    // Do nothing if node already focused
    if(focusedOnNodeAtRadius(nodeId, radius))
        return;

    ComponentId componentId = _graphRenderer->graphModel()->graph().componentIdOfNode(nodeId);
    Q_ASSERT(!componentId.isNull());
    bool componentTransitionRequired = (componentId != _componentId);

    if(componentTransitionRequired && !transitionActive())
    {
        // This node is in a different component, so focus it directly there,
        // and transition to the component itself
        auto* newComponentRenderer = _graphRenderer->componentRendererForId(componentId);
        newComponentRenderer->moveFocusToNode(nodeId, radius);
        newComponentRenderer->saveViewData();
        newComponentRenderer->resetView();

        setComponentId(componentId, true);
    }
    else if(!componentTransitionRequired && !componentTransitionActive())
    {
        _queuedTransitionNodeId.setToNull();
        startTransition().then([this] { performQueuedTransition(); });
        componentRenderer()->moveFocusToNode(nodeId, radius);
    }
    else
    {
        // A component transition is already in progress,
        // so queue the refocus up for later
        _queuedTransitionNodeId = nodeId;
    }
}

GraphComponentRenderer* GraphComponentScene::componentRenderer() const
{
    return _graphRenderer->componentRendererForId(_componentId);
}

GraphComponentRenderer* GraphComponentScene::transitioningComponentRenderer() const
{
    return _graphRenderer->componentRendererForId(_transitioningComponentId);
}

Transition& GraphComponentScene::startTransition(float duration, Transition::Type transitionType)
{
    return _graphRenderer->transition().start(duration, transitionType,
    [this](float f)
    {
        componentRenderer()->updateTransition(f);
    });
}

void GraphComponentScene::updateRendererVisibility()
{
    _graphRenderer->executeOnRendererThread([this]
    {
        if(visible())
        {
            for(GraphComponentRenderer* componentRenderer : _graphRenderer->componentRenderers())
            {
                if(!componentRenderer->initialised())
                    continue;

                bool isTransitioningRenderer = componentRenderer->componentId() == _transitioningComponentId;
                bool isMainRenderer = componentRenderer->componentId() == _componentId;

                componentRenderer->setVisible(isTransitioningRenderer || isMainRenderer);
            }

            _graphRenderer->onVisibilityChanged();
        }
    }, QStringLiteral("GraphComponentScene::updateRendererVisibility"));
}

void GraphComponentScene::onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet)
{
    if(!visible())
        return;

    auto oldComponentId = componentSplitSet.oldComponentId();
    if(oldComponentId == _componentId)
    {
        // Both of these things still exist after this returns
        auto largestSplitter = graph->componentIdOfLargestComponent(componentSplitSet.splitters());
        auto* oldGraphComponentRenderer = _graphRenderer->componentRendererForId(oldComponentId);

        _graphRenderer->executeOnRendererThread([this, largestSplitter, oldGraphComponentRenderer]
        {
            ComponentId newComponentId;

            if(!oldGraphComponentRenderer->trackingCentreOfComponent() &&
                _graphRenderer->graphModel()->graph().containsNodeId(
                oldGraphComponentRenderer->focusNodeId()))
            {
                newComponentId = _graphRenderer->graphModel()->graph().componentIdOfNode(
                    oldGraphComponentRenderer->focusNodeId());
            }
            else
                newComponentId = largestSplitter;

            Q_ASSERT(!newComponentId.isNull());

            auto* newGraphComponentRenderer = _graphRenderer->componentRendererForId(newComponentId);

            newGraphComponentRenderer->cloneViewDataFrom(*oldGraphComponentRenderer);
            setViewportSize(_width, _height);
            setComponentId(newComponentId);
        }, QStringLiteral("GraphComponentScene::onComponentSplit (clone camera data, set component ID)"));
    }
}

void GraphComponentScene::onComponentsWillMerge(const Graph*, const ComponentMergeSet& componentMergeSet)
{
    if(!visible())
        return;

    const auto& mergers = componentMergeSet.mergers();
    if(mergers.find(_componentId) == mergers.end())
        return;

    auto newComponentId = componentMergeSet.newComponentId();
    auto* newGraphComponentRenderer = _graphRenderer->componentRendererForId(newComponentId);
    auto* oldGraphComponentRenderer = _graphRenderer->componentRendererForId(_componentId);

    _graphRenderer->executeOnRendererThread(
    [this, newComponentId, newGraphComponentRenderer, oldGraphComponentRenderer]
    {
        // This occurs before GraphComponentRenderer::cleanup is called on oldGraphComponentRenderer
        newGraphComponentRenderer->cloneViewDataFrom(*oldGraphComponentRenderer);
        setComponentId(newComponentId);
    }, QStringLiteral("GraphComponentScene::onComponentsWillMerge (clone camera data, set component ID)"));
}

void GraphComponentScene::onComponentAdded(const Graph*, ComponentId componentId, bool)
{
    _graphRenderer->executeOnRendererThread([this, componentId]
    {
        if(_componentId.isNull())
            setComponentId(componentId, visible());
    }, QStringLiteral("GraphComponentScene::onComponentAdded"));
}

void GraphComponentScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool hasMerged)
{
    _graphRenderer->executeOnRendererThread([this, componentId, hasMerged]
    {
        if(componentId == _componentId && visible() && !hasMerged)
        {
            // Keep the component alive until any transitions have finished
            _beingRemoved = true;
            componentRenderer()->freeze();
        }
    }, QStringLiteral("GraphComponentScene::onComponentWillBeRemoved"));
}

void GraphComponentScene::onGraphWillChange(const Graph* graph)
{
    _numComponentsPriorToChange = graph->numComponents();
}

void GraphComponentScene::onGraphChanged(const Graph* graph, bool changed)
{
    auto atExit = std::experimental::make_scope_exit([this] { _graphRenderer->resumeRendererThreadExecution(); });

    if(!changed)
        return;

    _graphRenderer->executeOnRendererThread([this, graph]
    {
        _defaultComponentId = graph->componentIdOfLargestComponent();

        if(visible())
        {
            setViewportSize(_width, _height);

            auto finishTransition = [this, graph](bool fromTransition = true)
            {
                _graphRenderer->sceneFinishedTransition();

                // If graph change has resulted in multiple components, switch
                // to overview mode once the transition had completed
                if(_numComponentsPriorToChange == 1 && graph->numComponents() > 1)
                {
                    if(fromTransition)
                        _graphRenderer->transition().willBeImmediatelyReused();

                    _graphRenderer->switchToOverviewMode();

                    return true;
                }

                if(_beingRemoved)
                {
                    setComponentId(_defaultComponentId, true);
                    return true;
                }

                return false;
            };

            // Graph changes may significantly alter the centre; ease the transition
            if(!_beingRemoved && _numComponentsPriorToChange > 0 &&
               componentRenderer() != nullptr && componentRenderer()->transitionRequired())
            {
                startTransition().then(finishTransition);
                componentRenderer()->computeTransition();
            }
            else
            {
                // If we don't start a transition, we still want the renderer to do the things
                // it would have when the transition finished
                if(!finishTransition(false))
                    _graphRenderer->rendererFinishedTransition();
            }
        }
    }, QStringLiteral("GraphComponentScene::onGraphChanged (setSize/moveFocusToCentreOfComponent)"));
}

void GraphComponentScene::onNodeRemoved(const Graph*, NodeId nodeId, ComponentId)
{
    if(visible() && componentRenderer()->focusNodeId() == nodeId)
    {
        _graphRenderer->executeOnRendererThread([this]
        {
            // If the whole component is going away, we can't refocus
            if(_beingRemoved)
                return;

            startTransition();
            componentRenderer()->moveFocusToCentreOfComponent();
        }, QStringLiteral("GraphComponentScene::onNodeRemoved"));
    }
}

void GraphComponentScene::setProjection(Projection projection)
{
    if(!visible())
        return;

    for(GraphComponentRenderer* componentRenderer : _graphRenderer->componentRenderers())
    {
        bool isRendererForThisComponent = componentRenderer->componentId() == _componentId;

        if(!isRendererForThisComponent)
        {
            componentRenderer->setProjection(projection);
            componentRenderer->doProjectionTransition();
        }
    }

    auto startProjectionTransition = [this, projection]
    {
        _graphRenderer->executeOnRendererThread([this, projection]
        {
            startTransition(0.3f, projection == Projection::Perspective ?
                Transition::Type::Power : Transition::Type::InversePower);

            componentRenderer()->setProjection(projection);
            componentRenderer()->doProjectionTransition();
        }, QStringLiteral("GraphComponentScene::setProjection"));
    };

    if(!viewIsReset())
    {
        startTransition().then(
        [this, startProjectionTransition]
        {
            _graphRenderer->transition().willBeImmediatelyReused();
            startProjectionTransition();
        });

        resetView(false);
    }
    else
        startProjectionTransition();
}
