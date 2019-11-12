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

    void saveViewData();
    bool savedViewIsReset() const;
    void restoreViewData();

    void resetView(bool doTransition) override;
    bool viewIsReset() const override;

    void setProjection(Projection projection) override;

    void pan(NodeId clickedNodeId, const QPoint &start, const QPoint &end);

    bool focusedOnNodeAtRadius(NodeId nodeId, float radius) const;
    void moveFocusToNode(NodeId nodeId, float radius = -1.0f);

    GraphComponentRenderer* componentRenderer() const;
    GraphComponentRenderer* transitioningComponentRenderer() const;

    void startTransition(std::function<void()> finishedFunction = []{}, float duration = 0.3f,
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
    int _componentSize = 0;

    int _numComponentsPriorToChange = 0;

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
