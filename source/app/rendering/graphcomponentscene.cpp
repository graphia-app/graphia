#include "graphcomponentscene.h"
#include "graphrenderer.h"

#include "graphcomponentrenderer.h"

#include "../graph/graphmodel.h"

#include "../ui/graphquickitem.h"

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
    connect(&_graphRenderer->graphModel()->graph(), &Graph::nodeRemoved, this, &GraphComponentScene::onNodeRemoved, Qt::DirectConnection);

    _defaultComponentId = _graphRenderer->graphModel()->graph().componentIdOfLargestComponent();
}

void GraphComponentScene::update(float t)
{
    float offset = 0.0f;
    float outOffset = 0.0f;

    switch(_transitionDirection)
    {
    case Direction::Left:
        offset = (1.0f - _transitionValue) * _width;
        outOffset = offset - _width;
        break;

    case Direction::Right:
        offset = -(1.0f - _transitionValue) * _width;
        outOffset = offset + _width;
        break;

    case Direction::NotSliding:
    default:
        break;
    }

    // The static component, or the one sliding in
    if(componentRenderer() != nullptr)
    {
        componentRenderer()->setDimensions(QRect(offset, 0, _width, _height));
        componentRenderer()->setAlpha(1.0f);
        componentRenderer()->update(t);
    }

    // The component sliding out
    if(transitioningComponentRenderer() != nullptr &&
       transitioningComponentRenderer() != componentRenderer())
    {
        transitioningComponentRenderer()->setDimensions(QRect(outOffset, 0, _width, _height));
        transitioningComponentRenderer()->setAlpha(1.0f);
        transitioningComponentRenderer()->update(t);
    }
}

void GraphComponentScene::setViewportSize(int width, int height)
{
    _width = width;
    _height = height;

    if(componentRenderer() != nullptr)
    {
        componentRenderer()->setDimensions(QRect(0, 0, width, height));
        componentRenderer()->setViewportSize(width, height);
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
    auto transitionDirection = componentId < _componentId ? Direction::Right : Direction::Left;

    if(componentId.isNull())
        _componentId = _defaultComponentId;
    else
        _componentId = componentId;

    if(doTransition)
    {
        _transitionDirection = transitionDirection;
        _graphRenderer->transition().start(0.3f, Transition::Type::InversePower,
        [this](float f)
        {
            _transitionValue = f;
        },
        [this]
        {
            _transitionValue = 0.0f;
            _transitionDirection = Direction::NotSliding;
            _transitioningComponentId.setToNull();
            updateRendererVisibility();

            if(!savedViewIsReset() && _queuedTransitionNodeId.isNull())
            {
                _graphRenderer->executeOnRendererThread([this]
                {
                    _graphRenderer->transition().willBeImmediatelyReused();
                    startTransition([this] { performQueuedTransition(); });
                    restoreViewData();
                }, "GraphComponentScene::finishComponentTransition (restoreViewData)");
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
    }, "GraphComponentScene::finishComponentTransition");
}

void GraphComponentScene::performQueuedTransition()
{
    if(!_queuedTransitionNodeId.isNull())
    {
        _graphRenderer->executeOnRendererThread([this, nodeId = _queuedTransitionNodeId]
        {
            moveFocusToNode(nodeId);
        }, "GraphComponentScene::performQueuedTransition");

        _queuedTransitionNodeId.setToNull();
    }
}

bool GraphComponentScene::componentTransitionActive() const
{
    return transitionActive() && !_transitioningComponentId.isNull();
}

void GraphComponentScene::setComponentId(ComponentId componentId, bool doTransition)
{
    // Do nothing if component already focused
    if(!componentId.isNull() && componentId == _componentId)
        return;

    saveViewData();

    if(doTransition)
    {
        _transitioningComponentId = _componentId;
        if(!viewIsReset())
        {
            startTransition([this, componentId]
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

void GraphComponentScene::saveViewData()
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

void GraphComponentScene::restoreViewData()
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

void GraphComponentScene::pan(NodeId clickedNodeId, const QPoint& start, const QPoint& end)
{
    if(!clickedNodeId.isNull())
    {
        Camera* camera = componentRenderer()->camera();

        const QVector3D clickedNodePosition =
                _graphRenderer->graphModel()->nodePositions().getScaledAndSmoothed(clickedNodeId);

        Plane translationPlane(clickedNodePosition, camera->viewVector());

        QVector3D prevPoint = translationPlane.rayIntersection(
                    camera->rayForViewportCoordinates(start.x(), start.y()));
        QVector3D curPoint = translationPlane.rayIntersection(
                    camera->rayForViewportCoordinates(end.x(), end.y()));
        QVector3D translation = prevPoint - curPoint;

        camera->translate(translation);
    }
}

void GraphComponentScene::moveFocusToNode(NodeId nodeId)
{
    // Do nothing if node already focused
    if(componentRenderer()->focusNodeId() == nodeId)
        return;

    ComponentId componentId = _graphRenderer->graphModel()->graph().componentIdOfNode(nodeId);
    bool componentTransitionRequired = (componentId != _componentId);

    if(componentTransitionRequired && !transitionActive())
    {
        // This node is in a different component, so focus it directly there,
        // and transition to the component itself
        auto* newComponentRenderer = _graphRenderer->componentRendererForId(componentId);
        newComponentRenderer->moveFocusToNode(nodeId);
        newComponentRenderer->saveViewData();
        newComponentRenderer->resetView();

        setComponentId(componentId, true);
    }
    else if(!componentTransitionRequired && !componentTransitionActive())
    {
        _queuedTransitionNodeId.setToNull();
        startTransition([this] { performQueuedTransition(); });
        componentRenderer()->moveFocusToNode(nodeId);
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

void GraphComponentScene::startTransition(std::function<void()> finishedFunction,
                                          float duration, Transition::Type transitionType)
{
    _graphRenderer->transition().start(duration, transitionType,
    [this](float f)
    {
        componentRenderer()->updateTransition(f);
    },
    finishedFunction);
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
    }, "GraphComponentScene::onShow (setVisible)");
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
        auto oldGraphComponentRenderer = _graphRenderer->componentRendererForId(oldComponentId);

        _graphRenderer->executeOnRendererThread([this, largestSplitter, oldGraphComponentRenderer]
        {
            ComponentId newComponentId;

            if(!oldGraphComponentRenderer->trackingCentreOfComponent())
            {
                newComponentId = _graphRenderer->graphModel()->graph().componentIdOfNode(
                    oldGraphComponentRenderer->focusNodeId());
            }
            else
                newComponentId = largestSplitter;


            Q_ASSERT(!newComponentId.isNull());

            auto newGraphComponentRenderer = _graphRenderer->componentRendererForId(newComponentId);

            newGraphComponentRenderer->cloneViewDataFrom(*oldGraphComponentRenderer);
            setComponentId(newComponentId);
        }, "GraphComponentScene::onComponentSplit (clone camera data, set component ID)");
    }
}

void GraphComponentScene::onComponentsWillMerge(const Graph*, const ComponentMergeSet& componentMergeSet)
{
    if(!visible())
        return;

    for(auto merger : componentMergeSet.mergers())
    {
        if(merger == _componentId)
        {
            auto newComponentId = componentMergeSet.newComponentId();
            auto newGraphComponentRenderer = _graphRenderer->componentRendererForId(newComponentId);
            auto oldGraphComponentRenderer = _graphRenderer->componentRendererForId(_componentId);

            _graphRenderer->executeOnRendererThread([this, newComponentId,
                                                    newGraphComponentRenderer,
                                                    oldGraphComponentRenderer]
            {
                // This occurs before GraphComponentRenderer::cleanup is called on oldGraphComponentRenderer
                newGraphComponentRenderer->cloneViewDataFrom(*oldGraphComponentRenderer);
                setComponentId(newComponentId);
            }, "GraphComponentScene::onComponentsWillMerge (clone camera data, set component ID)");
            break;
        }
    }
}

void GraphComponentScene::onComponentAdded(const Graph*, ComponentId componentId, bool)
{
    if(_componentId.isNull())
        setComponentId(componentId);
}

void GraphComponentScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool hasMerged)
{
    if(componentId == _componentId)
    {
        if(visible() && !hasMerged)
            _graphRenderer->switchToOverviewMode();

        _componentId.setToNull();
    }
}

void GraphComponentScene::onGraphWillChange(const Graph* graph)
{
    _numComponentsPriorToChange = graph->numComponents();
}

void GraphComponentScene::onGraphChanged(const Graph* graph)
{
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

                return false;
            };

            // Graph changes may significantly alter the centre; ease the transition
            if(componentRenderer() != nullptr && componentRenderer()->transitionRequired())
            {
                startTransition(finishTransition);
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
    }, "GraphComponentScene::onGraphChanged (setSize/moveFocusToCentreOfComponent)");

    _graphRenderer->resumeRendererThreadExecution();
}

void GraphComponentScene::onNodeRemoved(const Graph*, NodeId nodeId)
{
    if(visible() && componentRenderer()->focusNodeId() == nodeId)
    {
        _graphRenderer->executeOnRendererThread([this]
        {
            startTransition();
            componentRenderer()->moveFocusToCentreOfComponent();
        }, "GraphWidget::onNodeRemoved");
    }
}
