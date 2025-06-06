/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef GRAPHOVERVIEWSCENE_H
#define GRAPHOVERVIEWSCENE_H

#include "scene.h"
#include "transition.h"

#include "app/graph/componentmanager.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/grapharray.h"

#include "app/preferences.h"

#include "app/layout/componentlayout.h"

#include <vector>
#include <mutex>
#include <memory>
#include <atomic>

#include <QRect>
#include <QPointF>

class Graph;
class GraphModel;
class CommandManager;
class GraphRenderer;

template<typename Target>
void initialiseFromGraph(const Graph*, Target&); // NOLINT readability-redundant-declaration

class GraphOverviewScene :
        public Scene
{
    Q_OBJECT

    friend class GraphRenderer;
    friend void initialiseFromGraph<GraphOverviewScene>(const Graph*, GraphOverviewScene&);

public:
    explicit GraphOverviewScene(CommandManager* commandManager,
                                GraphRenderer* graphRenderer);

    void update(float t) override;
    void setViewportSize(int width, int height) override;

    bool transitionActive() const override;

    void onShow() override;
    void onHide() override;

    const ComponentLayoutData& componentLayout() { return _zoomedComponentLayoutData; }

    void resetView(NamedBool<"doTransition"> doTransition = "doTransition"_yes) override;
    bool viewIsReset() const override;

    void setProjection(Projection projection) override;

    void pan(float dx, float dy);
    void zoom(float delta, float x, float y, NamedBool<"doTransition"> doTransition);

    void zoomTo(const std::vector<ComponentId>& componentIds, NamedBool<"doTransition"> doTransition = "doTransition"_yes);

    Transition& startTransitionFromComponentMode(ComponentId componentModeComponentId,
        const std::vector<ComponentId>& focusComponentIds,
        float duration = defaultDuration, Transition::Type transitionType = Transition::Type::EaseInEaseOut);
    Transition& startTransitionToComponentMode(ComponentId componentModeComponentId,
        float duration = defaultDuration, Transition::Type transitionType = Transition::Type::EaseInEaseOut);

private:
    GraphRenderer* _graphRenderer = nullptr;
    CommandManager* _commandManager = nullptr;
    GraphModel* _graphModel = nullptr;

    PreferencesWatcher _preferencesWatcher;

    int _width = 0;
    int _height = 0;

    float _zoomFactor = 1.0f; // Multiply by raw layout data to get screen coords
    Transition _zoomTransition;
    QPointF _offset;

    std::atomic_bool _renderersRequireReset = false;

    ComponentArray<float> _previousComponentAlpha;
    ComponentArray<float> _componentAlpha;

    std::atomic_bool _nextComponentLayoutDataChanged;
    ComponentLayoutData _nextComponentLayoutData;
    ComponentLayoutData _componentLayoutData;
    ComponentLayoutData _previousZoomedComponentLayoutData;
    ComponentLayoutData _zoomedComponentLayoutData;
    QSizeF _componentLayoutSize;
    std::unique_ptr<ComponentLayout> _componentLayout;

    void updateZoomedComponentLayoutData();

    std::vector<ComponentId> _removedComponentIds;
    std::vector<ComponentMergeSet> _componentMergeSets;
    Transition& startTransition(float duration, Transition::Type transitionType);
    void startZoomTransition(float duration = defaultDuration);

    std::vector<ComponentId> _componentIds;

    Circle zoomedLayoutData(const Circle& data) const;

    float minZoomFactor() const;
    bool setZoomFactor(float zoomFactor);
    QPointF defaultOffset() const;
    bool setOffset(QPointF offset);
    bool setOffset(float x, float y);
    bool setOffsetForBoundingBox(const QRectF& boundingBox);

    void setVisible(bool visible);

    void startComponentLayoutTransition();

private slots:
    void onGraphWillChange(const Graph* graph);
    void onGraphChanged(const Graph* graph, bool changed);
    void onComponentAdded(const Graph* graph, ComponentId componentId, bool hasSplit);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool hasMerged);
    void onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph* graph, const ComponentMergeSet& componentMergeSet);

    void onVisualsChanged(VisualChangeFlags nodeChange, VisualChangeFlags edgeChange);

    void onPreferenceChanged(const QString& key, const QVariant& value);
};

#endif // GRAPHOVERVIEWSCENE_H
