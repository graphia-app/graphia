/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "graphoverviewscene.h"
#include "graphrenderer.h"
#include "graphcomponentrenderer.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include "layout/nodepositions.h"
#include "layout/powerof2gridcomponentlayout.h"
#include "layout/circlepackcomponentlayout.h"

#include "commands/commandmanager.h"

#include "ui/graphquickitem.h"

#include "shared/utils/utils.h"
#include "shared/utils/container.h"
#include "shared/utils/scope_exit.h"
#include "shared/utils/flags.h"
#include "app/preferences.h"

#include <QPoint>

#include <stack>
#include <algorithm>
#include <functional>
#include <vector>

using namespace Qt::Literals::StringLiterals;

static float zoomFactorFor(float sceneWidth, float sceneHeight,
    float boundingWidth, float boundingHeight)
{
    if(boundingWidth <= 0.0f && boundingHeight <= 0.0f)
        return 1.0f;

    const float minWidthZoomFactor = sceneWidth / boundingWidth;
    const float minHeightZoomFactor = sceneHeight / boundingHeight;

    return std::min(minWidthZoomFactor, minHeightZoomFactor);
}

static QPointF offsetFor(float sceneWidth, float sceneHeight, const QRectF& boundingBox, float zoomFactor)
{
    const auto scaledSceneWidth = sceneWidth / zoomFactor;
    const auto scaledSceneHeight = sceneHeight / zoomFactor;

    const auto xOffset = (scaledSceneWidth - static_cast<float>(boundingBox.width())) * 0.5f;
    const auto yOffset = (scaledSceneHeight - static_cast<float>(boundingBox.height())) * 0.5f;

    return {xOffset - static_cast<float>(boundingBox.x()),
        yOffset - static_cast<float>(boundingBox.y())};
}

GraphOverviewScene::GraphOverviewScene(CommandManager* commandManager, GraphRenderer* graphRenderer) :
    Scene(graphRenderer),
    _graphRenderer(graphRenderer),
    _commandManager(commandManager),
    _graphModel(graphRenderer->graphModel()),
    _previousComponentAlpha(_graphModel->graph(), 1.0f),
    _componentAlpha(_graphModel->graph(), 1.0f),
    _nextComponentLayoutDataChanged(false),
    _nextComponentLayoutData(_graphModel->graph()),
    _componentLayoutData(_graphModel->graph()),
    _previousZoomedComponentLayoutData(_graphModel->graph()),
    _zoomedComponentLayoutData(_graphModel->graph()),
    _componentLayout(std::make_unique<CirclePackComponentLayout>())
{
    connect(&_graphModel->graph(), &Graph::componentAdded, this, &GraphOverviewScene::onComponentAdded, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentWillBeRemoved, this, &GraphOverviewScene::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &GraphOverviewScene::onComponentSplit, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::componentsWillMerge, this, &GraphOverviewScene::onComponentsWillMerge, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &GraphOverviewScene::onGraphWillChange, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphOverviewScene::onGraphChanged, Qt::DirectConnection);

    connect(_graphModel, &GraphModel::visualsChanged, this, &GraphOverviewScene::onVisualsChanged, Qt::DirectConnection);

    connect(&_zoomTransition, &Transition::started, _graphRenderer, &GraphRenderer::rendererStartedTransition, Qt::DirectConnection);
    connect(&_zoomTransition, &Transition::finished, _graphRenderer, &GraphRenderer::rendererFinishedTransition, Qt::DirectConnection);

    connect(&_preferencesWatcher, &PreferencesWatcher::preferenceChanged,
        this, &GraphOverviewScene::onPreferenceChanged, Qt::DirectConnection);
}

void GraphOverviewScene::update(float t)
{
    _zoomTransition.update(t);

    // See ::onVisualsChanged
    if(_renderersRequireReset)
    {
        for(auto componentId : _componentIds)
        {
            auto* renderer = _graphRenderer->componentRendererForId(componentId);
            renderer->resetView();
        }

        _renderersRequireReset = false;
    }

    for(auto componentId : _componentIds)
    {
        auto* renderer = _graphRenderer->componentRendererForId(componentId);
        Q_ASSERT(renderer->initialised());
        renderer->setDimensions(_zoomedComponentLayoutData[componentId].boundingBox());
        renderer->setAlpha(_componentAlpha[componentId]);
        renderer->update(t);
    }
}

void GraphOverviewScene::setVisible(bool visible)
{
    for(auto componentId : _componentIds)
    {
        auto* renderer = _graphRenderer->componentRendererForId(componentId);
        renderer->setVisible(visible);
    }

    _graphRenderer->onVisibilityChanged();
}

void GraphOverviewScene::onShow()
{
    // Make previous and current match, in case we're being shown for the first time
    _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;

    setVisible(true);
}

void GraphOverviewScene::onHide()
{
    setVisible(false);
}

void GraphOverviewScene::resetView(bool doTransition)
{
    setZoomFactor(minZoomFactor());
    setOffset(defaultOffset());

    if(doTransition)
        startZoomTransition();
    else
        updateZoomedComponentLayoutData();
}

bool GraphOverviewScene::viewIsReset() const
{
    return _zoomFactor == minZoomFactor() && _offset == defaultOffset();
}

void GraphOverviewScene::pan(float dx, float dy)
{
    const float scaledDx = dx / (_zoomFactor);
    const float scaledDy = dy / (_zoomFactor);

    setOffset(static_cast<float>(_offset.x()) + scaledDx,
              static_cast<float>(_offset.y()) + scaledDy);

    updateZoomedComponentLayoutData();
}

void GraphOverviewScene::zoom(float delta, float x, float y, bool doTransition)
{
    const float oldCentreX = x / _zoomFactor;
    const float oldCentreY = y / _zoomFactor;

    if(!setZoomFactor(_zoomFactor + (delta * _zoomFactor)))
        return;

    if(_zoomFactor == minZoomFactor())
    {
        resetView(doTransition);
        return;
    }

    const float newCentreX = x / _zoomFactor;
    const float newCentreY = y / _zoomFactor;

    setOffset(static_cast<float>(_offset.x()) - (oldCentreX - newCentreX),
        static_cast<float>(_offset.y()) - (oldCentreY - newCentreY));

    if(doTransition)
        startZoomTransition();
    else
        updateZoomedComponentLayoutData();
}

void GraphOverviewScene::zoomTo(const std::vector<ComponentId>& componentIds, bool doTransition)
{
    Q_ASSERT(!componentIds.empty());

    auto zoomedBoundingBox = _componentLayout->boundingBoxFor(componentIds, _componentLayoutData);
    auto componentsZoomFactor = zoomFactorFor(static_cast<float>(_width), static_cast<float>(_height),
        static_cast<float>(zoomedBoundingBox.width()), static_cast<float>(zoomedBoundingBox.height()));

    bool zoomFactorChanged = setZoomFactor(componentsZoomFactor);
    bool offsetChanged = setOffsetForBoundingBox(zoomedBoundingBox);
    bool force = _graphRenderer->transition().active();

    if(!zoomFactorChanged && !offsetChanged && !force)
        return;

    if(doTransition)
        startZoomTransition();
    else
        updateZoomedComponentLayoutData();
}

Circle GraphOverviewScene::zoomedLayoutData(const Circle& data) const
{
    Circle newData(data);

    newData.translate(_offset);
    newData.scale(_zoomFactor);

    return newData;
}

float GraphOverviewScene::minZoomFactor() const
{
    return zoomFactorFor(static_cast<float>(_width), static_cast<float>(_height),
        static_cast<float>(_componentLayoutSize.width()),
        static_cast<float>(_componentLayoutSize.height()));
}

bool GraphOverviewScene::setZoomFactor(float zoomFactor)
{
    zoomFactor = std::max(minZoomFactor(), zoomFactor);
    const bool changed = _zoomFactor != zoomFactor;
    _zoomFactor = zoomFactor;

    return changed;
}

QPointF GraphOverviewScene::defaultOffset() const
{
    return offsetFor(static_cast<float>(_width), static_cast<float>(_height), {0.0, 0.0,
        _componentLayoutSize.width(), _componentLayoutSize.height()}, _zoomFactor);
}

bool GraphOverviewScene::setOffset(QPointF offset)
{
    const auto scaledHalfSceneWidth = (static_cast<double>(_width) * 0.5) / static_cast<double>(_zoomFactor);
    const auto scaledHalfSceneHeight = (static_cast<double>(_height) * 0.5) / static_cast<double>(_zoomFactor);

    const auto xMin = scaledHalfSceneWidth - _componentLayoutSize.width();
    const auto xMax = scaledHalfSceneWidth;
    offset.setX(std::clamp(offset.x(), xMin, xMax));

    const auto yMin = scaledHalfSceneHeight - _componentLayoutSize.height();
    const auto yMax = scaledHalfSceneHeight;
    offset.setY(std::clamp(offset.y(), yMin, yMax));

    if(offset != _offset)
    {
        _offset = offset;
        return true;
    }

    return false;
}

bool GraphOverviewScene::setOffset(float x, float y)
{
    return setOffset({x, y});
}

bool GraphOverviewScene::setOffsetForBoundingBox(const QRectF& boundingBox)
{
    return setOffset(offsetFor(static_cast<float>(_width), static_cast<float>(_height), boundingBox, _zoomFactor));
}

Transition& GraphOverviewScene::startTransitionFromComponentMode(ComponentId componentModeComponentId,
    const std::vector<ComponentId>& focusComponentIds,
    float duration, Transition::Type transitionType)
{
    Q_ASSERT(!componentModeComponentId.isNull());

    const float halfWidth = static_cast<float>(_width) * 0.5f;
    const float halfHeight = static_cast<float>(_height) * 0.5f;
    const Circle componentModeComponentLayout(halfWidth, halfHeight, std::min(halfWidth, halfHeight));

    // If the component that has focus isn't in the overview scene's component list then it's
    // going away, in which case we need to deal with it
    if(!u::contains(_componentIds, componentModeComponentId))
    {
        _removedComponentIds.emplace_back(componentModeComponentId);
        _componentIds.emplace_back(componentModeComponentId);

        // Target display properties
        _zoomedComponentLayoutData[componentModeComponentId] = componentModeComponentLayout;
        _componentAlpha[componentModeComponentId] = 0.0f;

        // The renderer should have already been frozen by GraphComponentScene::onComponentWillBeRemoved,
        // but let's make sure
        auto* renderer = _graphRenderer->componentRendererForId(componentModeComponentId);
        renderer->freeze();
    }

    if(!focusComponentIds.empty())
        zoomTo(focusComponentIds, false);

    auto& transition = startTransition(duration, transitionType);

    _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;
    _previousComponentAlpha.fill(0.0f);

    // The focus component always starts covering the viewport and fully opaque
    _previousZoomedComponentLayoutData[componentModeComponentId] = componentModeComponentLayout;
    _previousComponentAlpha[componentModeComponentId] = 1.0f;

    return transition;
}

Transition& GraphOverviewScene::startTransitionToComponentMode(ComponentId componentModeComponentId,
    float duration, Transition::Type transitionType)
{
    Q_ASSERT(!componentModeComponentId.isNull());

    _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;
    _previousComponentAlpha = _componentAlpha;

    for(auto componentId : _componentIds)
    {
        if(componentId != componentModeComponentId)
            _componentAlpha[componentId] = 0.0f;
    }

    const float halfWidth = static_cast<float>(_width) * 0.5f;
    const float halfHeight = static_cast<float>(_height) * 0.5f;
    _zoomedComponentLayoutData[componentModeComponentId].set(
        halfWidth, halfHeight, std::min(halfWidth, halfHeight));

    return startTransition(duration, transitionType);
}

void GraphOverviewScene::updateZoomedComponentLayoutData()
{
    for(auto componentId : _componentIds)
        _zoomedComponentLayoutData[componentId] = zoomedLayoutData(_componentLayoutData[componentId]);
}

void GraphOverviewScene::setViewportSize(int width, int height)
{
    bool viewWasReset = viewIsReset();

    _width = width;
    _height = height;

    if(_nextComponentLayoutDataChanged.exchange(false))
    {
        _componentLayoutData = _nextComponentLayoutData;
        auto boundingBox = _componentLayout->boundingBoxFor(_componentIds, _componentLayoutData);
        _componentLayoutSize = boundingBox.size();
    }

    if(viewWasReset)
    {
        setZoomFactor(minZoomFactor());
        setOffset(defaultOffset());
    }
    else
    {
        setZoomFactor(_zoomFactor);
        setOffset(_offset);
    }

    updateZoomedComponentLayoutData();

    for(auto componentId : _componentIds)
    {
        _componentAlpha[componentId] = 1.0f;

        // If the component is fading in, keep it in a fixed position
        if(_previousComponentAlpha[componentId] == 0.0f)
            _previousZoomedComponentLayoutData[componentId] = _zoomedComponentLayoutData[componentId];
    }

    // Give the mergers the same layout as the new component
    for(const auto& componentMergeSet : _componentMergeSets)
    {
        auto newComponentId = componentMergeSet.newComponentId();

        for(const auto& merger : componentMergeSet.mergers())
        {
            _zoomedComponentLayoutData[merger] = _zoomedComponentLayoutData[newComponentId];
            _componentAlpha[merger] = _componentAlpha[newComponentId];
        }
    }

    for(auto componentId : _componentIds)
    {
        auto* renderer = _graphRenderer->componentRendererForId(componentId);
        renderer->setViewportSize(_width, _height);
        renderer->setDimensions(_zoomedComponentLayoutData[componentId].boundingBox());
    }
}

bool GraphOverviewScene::transitionActive() const
{
    if(_zoomTransition.active())
        return true;

    for(auto componentId : _componentIds)
    {
        auto* renderer = _graphRenderer->componentRendererForId(componentId);

        if(renderer->transitionActive())
            return true;
    }

    return false;
}

static Circle interpolateCircle(const Circle& a, const Circle& b, float f)
{
    return {
        u::interpolate(a.x(),       b.x(),      f),
        u::interpolate(a.y(),       b.y(),      f),
        u::interpolate(a.radius(),  b.radius(), f)
        };
}

Transition& GraphOverviewScene::startTransition(float duration, Transition::Type transitionType)
{
    _zoomTransition.cancel();

    auto targetComponentLayoutData = _zoomedComponentLayoutData;
    auto targetComponentAlpha = _componentAlpha;

    auto& transition = _graphRenderer->transition().start(duration, transitionType,
    [this, targetComponentLayoutData = std::move(targetComponentLayoutData),
           targetComponentAlpha = std::move(targetComponentAlpha)](float f)
    {
        auto interpolate = [&](ComponentId componentId)
        {
            _zoomedComponentLayoutData[componentId] = interpolateCircle(
                _previousZoomedComponentLayoutData[componentId],
                targetComponentLayoutData[componentId], f);

            _componentAlpha[componentId] = u::interpolate(
                _previousComponentAlpha[componentId],
                targetComponentAlpha[componentId], f);

            _graphRenderer->componentRendererForId(componentId)->updateTransition(f);
        };

        for(auto componentId : _componentIds)
            interpolate(componentId);
    }).then(
    [this]
    {
        _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;
        _previousComponentAlpha = _componentAlpha;

        for(auto componentId : _removedComponentIds)
        {
            auto* renderer = _graphRenderer->componentRendererForId(componentId);
            renderer->thaw();
        }

        for(const auto& componentMergeSet : _componentMergeSets)
        {
            auto* renderer = _graphRenderer->componentRendererForId(componentMergeSet.newComponentId());
            renderer->thaw();
        }

        // Subtract the removed ComponentIds, as we no longer need to render them
        std::vector<ComponentId> postTransitionComponentIds;
        std::sort(_componentIds.begin(), _componentIds.end());
        std::sort(_removedComponentIds.begin(), _removedComponentIds.end());
        std::set_difference(_componentIds.begin(), _componentIds.end(),
            _removedComponentIds.begin(), _removedComponentIds.end(),
            std::inserter(postTransitionComponentIds, postTransitionComponentIds.begin()));

        _componentIds = std::move(postTransitionComponentIds);

        _removedComponentIds.clear();
        _componentMergeSets.clear();

        _graphRenderer->sceneFinishedTransition();
    });

    // Reset all components by default
    for(auto componentId : _componentIds)
    {
        auto* renderer = _graphRenderer->componentRendererForId(componentId);
        renderer->resetView();
    }

    for(const auto& componentMergeSet : _componentMergeSets)
    {
        const auto* mergedComponent = _graphModel->graph().componentById(componentMergeSet.newComponentId());
        const auto& mergedNodeIds = mergedComponent->nodeIds();

        auto centreOfMass = _graphModel->nodePositions().centreOfMass(mergedNodeIds);
        auto radius = GraphComponentRenderer::maxNodeDistanceFromPoint(
            *_graphModel, centreOfMass, mergedNodeIds);

        // Use the rotation of the new component
        auto* renderer = _graphRenderer->componentRendererForId(componentMergeSet.newComponentId());
        const QQuaternion rotation = renderer->camera()->rotation();

        for(auto merger : componentMergeSet.mergers())
        {
            renderer = _graphRenderer->componentRendererForId(merger);
            renderer->moveFocusTo(centreOfMass, radius, rotation);
        }
    }

    return transition;
}

void GraphOverviewScene::startZoomTransition(float duration)
{
    _graphRenderer->transition().cancel();

    ComponentLayoutData targetZoomedComponentLayoutData(_graphModel->graph());
    ComponentArray<float> targetComponentAlpha(_graphModel->graph(), 1.0f);

    _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;
    _previousComponentAlpha = _componentAlpha;

    for(auto componentId : _componentIds)
        targetZoomedComponentLayoutData[componentId] = zoomedLayoutData(_componentLayoutData[componentId]);

    _zoomTransition.start(duration, Transition::Type::InversePower,
    [this, targetZoomedComponentLayoutData, targetComponentAlpha](float f)
    {
        for(auto componentId : _componentIds)
        {
            _zoomedComponentLayoutData[componentId] =
                interpolateCircle(_previousZoomedComponentLayoutData[componentId],
                targetZoomedComponentLayoutData[componentId], f);

            _componentAlpha[componentId] = u::interpolate(
                _previousComponentAlpha[componentId],
                targetComponentAlpha[componentId], f);
        }
    }).then(
    [this]
    {
        // When the zoom is complete, don't leave previous data out of date
        _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;
        _previousComponentAlpha = _componentAlpha;
    });
}

void GraphOverviewScene::onComponentAdded(const Graph*, ComponentId componentId, bool hasSplit)
{
    if(!hasSplit)
    {
        _graphRenderer->executeOnRendererThread([this, componentId]
        {
            if(visible())
                _previousComponentAlpha[componentId] = 0.0f;
        }, u"GraphOverviewScene::onComponentAdded (set source alpha to 0)"_s);
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
            auto* renderer = _graphRenderer->componentRendererForId(componentId);
            renderer->freeze();

            _removedComponentIds.emplace_back(componentId);
            _componentAlpha[componentId] = 0.0f;
        }, u"GraphOverviewScene::onComponentWillBeRemoved (freeze renderer, set target alpha to 0)"_s);
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
            auto* oldGraphComponentRenderer = _graphRenderer->componentRendererForId(oldComponentId);

            for(auto splitter : componentSplitSet.splitters())
            {
                auto* renderer = _graphRenderer->componentRendererForId(splitter);
                renderer->cloneViewDataFrom(*oldGraphComponentRenderer);
                _previousZoomedComponentLayoutData[splitter] = _zoomedComponentLayoutData[oldComponentId];
                _previousComponentAlpha[splitter] = _componentAlpha[oldComponentId];
            }
        }
    }, u"GraphOverviewScene::onComponentSplit (cloneCameraDataFrom, component layout)"_s);
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
            auto* renderer = _graphRenderer->componentRendererForId(merger);
            renderer->freeze();

            if(merger != componentMergeSet.newComponentId())
                _removedComponentIds.emplace_back(merger);
        }
    }, u"GraphOverviewScene::onComponentsWillMerge (freeze renderers)"_s);
}

void GraphOverviewScene::onVisualsChanged(VisualChangeFlags nodeChange, VisualChangeFlags)
{
    if(!Flags<VisualChangeFlags>(nodeChange).test(VisualChangeFlags::Size))
        return;

    _graphRenderer->executeOnRendererThread([this]
    {
        // The camera distance for component renderers is calculated in
        // part on the maximum size of the nodes in the component, so we
        // must force it to be updated when the node sizes changes; this
        // causes each renderer to be reset in ::update
        _renderersRequireReset = true;
    }, u"GraphOverviewScene::onVisualsChanged (reset renderers)"_s);
}

void GraphOverviewScene::onGraphWillChange(const Graph*)
{
    // Take a copy of the existing layout before the graph is changed
    _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;
}

void GraphOverviewScene::startComponentLayoutTransition()
{
    if(visible())
    {
        const bool componentLayoutDataChanged = _componentLayoutData != _nextComponentLayoutData;
        const float duration = !componentLayoutDataChanged ? 0.0f : u::pref(u"visuals/transitionTime"_s).toFloat();

        setVisible(true); // Show new components
        setViewportSize(_width, _height);

        startTransition(duration, Transition::Type::EaseInEaseOut).then(
        [this]
        {
            // If a graph change has resulted in a single component, switch
            // to component mode once the transition has completed
            if(_graphModel->graph().numComponents() == 1)
            {
                _graphRenderer->transition().willBeImmediatelyReused();
                _graphRenderer->switchToComponentMode();
            }
        });
    }
}

void GraphOverviewScene::onGraphChanged(const Graph* graph, bool changed)
{
    auto atExit = std::experimental::make_scope_exit([this] { _graphRenderer->resumeRendererThreadExecution(); });

    if(!changed)
        return;

    _commandManager->setPhase(tr("Component Layout"));
    _componentLayout->execute(*graph, graph->componentIds(), _nextComponentLayoutData);
    _commandManager->clearPhase();

    _nextComponentLayoutDataChanged = true;

    _graphRenderer->executeOnRendererThread([this, graph]
    {
        _componentIds = graph->componentIds();

        startComponentLayoutTransition();

        // We still need to render any components that have been removed, while they
        // transition away
        _componentIds.insert(_componentIds.end(),
            _removedComponentIds.begin(), _removedComponentIds.end());
    }, u"GraphOverviewScene::onGraphChanged"_s);
}

void GraphOverviewScene::onPreferenceChanged(const QString& key, const QVariant&)
{
    if(visible() && key == u"visuals/minimumComponentRadius"_s)
    {
        _commandManager->executeOnce(
        [this](Command&)
        {
            const auto* graph = &_graphModel->graph();

            _previousZoomedComponentLayoutData = _zoomedComponentLayoutData;

            _componentLayout->execute(*graph, graph->componentIds(), _nextComponentLayoutData);

            if(_nextComponentLayoutData != _componentLayoutData)
            {
                _nextComponentLayoutDataChanged = true;
                _graphRenderer->executeOnRendererThread([this]
                {
                    this->startComponentLayoutTransition();
                }, u"GraphOverviewScene::onPreferenceChanged"_s);
            }
        }, {tr("Component Layout")});
    }
}

void GraphOverviewScene::setProjection(Projection projection)
{
    if(!visible())
        return;

    _graphRenderer->executeOnRendererThread([this, projection]
    {
        startTransition(defaultDuration, projection == Projection::Perspective ?
            Transition::Type::Power : Transition::Type::InversePower);

        for(GraphComponentRenderer* componentRenderer : _graphRenderer->componentRenderers())
        {
            componentRenderer->setProjection(projection);
            componentRenderer->doProjectionTransition();
        }
    }, u"GraphOverviewScene::setProjection"_s);
}
