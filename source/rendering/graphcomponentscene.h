#ifndef GRAPHCOMPONENTSCENE_H
#define GRAPHCOMPONENTSCENE_H

#include "scene.h"
#include "graphrenderer.h"

#include "../graph/graph.h"
#include "../graph/componentmanager.h"
#include "../graph/grapharray.h"
#include "transition.h"

class GraphComponentRenderer;

class GraphComponentScene :
        public Scene,
        public GraphInitialiser
{
    Q_OBJECT

public:
    explicit GraphComponentScene(GraphRenderer* graphRenderer);

    void update(float t);
    void setViewportSize(int width, int height);

    bool transitionActive() const;

    void onShow();

    ComponentId componentId() const { return _componentId; }
    void setComponentId(ComponentId componentId);

    int width() const { return _width; }
    int height() const { return _height; }

    void saveViewData();
    bool savedViewIsReset() const;
    void restoreViewData();

    void resetView(bool doTransition);
    bool viewIsReset() const;

    void pan(NodeId clickedNodeId, const QPoint &start, const QPoint &end);

    GraphComponentRenderer* componentRenderer() const;

    void startTransition(float duration = 0.3f,
                         Transition::Type transitionType = Transition::Type::EaseInEaseOut,
                         std::function<void()> finishedFunction = []{});

private:
    GraphRenderer* _graphRenderer;

    int _width = 0;
    int _height = 0;

    ComponentId _defaultComponentId;
    ComponentId _componentId;

    int _numComponentsPriorToChange = 0;

private slots:
    void onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph* graph, const ComponentMergeSet& componentMergeSet);
    void onComponentAdded(const Graph* graph, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool);
    void onGraphWillChange(const Graph* graph);
    void onGraphChanged(const Graph* graph);
    void onNodeWillBeRemoved(const Graph* graph, const Node* node);
};

#endif // GRAPHCOMPONENTSCENE_H
