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

#ifndef GRAPHCOMPONENTSCENE_H
#define GRAPHCOMPONENTSCENE_H

#include "scene.h"

#include "graph/componentmanager.h"
#include "shared/graph/grapharray.h"
#include "transition.h"

class Graph;
class GraphRenderer;
class GraphComponentRenderer;

template<typename Target>
void initialiseFromGraph(const Graph*, Target&); // NOLINT

class GraphComponentScene :
        public Scene
{
    Q_OBJECT

    friend class GraphRenderer;
    friend void initialiseFromGraph<GraphComponentScene>(const Graph*, GraphComponentScene&);

public:
    explicit GraphComponentScene(GraphRenderer* graphRenderer);

    void update(float t) override;
    void setViewportSize(int width, int height) override;

    bool transitionActive() const override;

    void onShow() override;

    ComponentId componentId() const { return _componentId; }
    void setComponentId(ComponentId componentId, bool doTransition = false);

    int width() const { return _width; }
    int height() const { return _height; }

    void saveViewData() const;
    bool savedViewIsReset() const;
    void restoreViewData() const;

    void resetView(bool doTransition) override;
    bool viewIsReset() const override;

    void setProjection(Projection projection) override;

    void pan(NodeId clickedNodeId, const QPoint &start, const QPoint &end);

    bool focusedOnNodeAtRadius(NodeId nodeId, float radius) const;
    void moveFocusToNode(NodeId nodeId, float radius = -1.0f);

    GraphComponentRenderer* componentRenderer() const;
    GraphComponentRenderer* transitioningComponentRenderer() const;

    Transition& startTransition(float duration = 0.3f,
        Transition::Type transitionType = Transition::Type::EaseInEaseOut);

private:
    GraphRenderer* _graphRenderer;

    int _width = 0;
    int _height = 0;

    ComponentId _defaultComponentId;
    ComponentId _componentId;

    ComponentId _transitioningComponentId;
    float _transitionValue = 0.0f;
    enum class TransitionStyle { None, SlideLeft, SlideRight, Fade } _transitionStyle = TransitionStyle::None;
    NodeId _queuedTransitionNodeId;

    bool _beingRemoved = false;
    size_t _componentSize = 0;

    size_t _numComponentsPriorToChange = 0;

    void updateRendererVisibility();

    void finishComponentTransition(ComponentId componentId, bool doTransition);
    void finishComponentTransitionOnRendererThread(ComponentId componentId, bool doTransition);
    void performQueuedTransition();

    bool componentTransitionActive() const;

private slots:
    void onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph* graph, const ComponentMergeSet& componentMergeSet);
    void onComponentAdded(const Graph* graph, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool);
    void onGraphWillChange(const Graph* graph);
    void onGraphChanged(const Graph* graph, bool changed);
    void onNodeRemoved(const Graph* graph, NodeId nodeId, ComponentId);
};

#endif // GRAPHCOMPONENTSCENE_H
