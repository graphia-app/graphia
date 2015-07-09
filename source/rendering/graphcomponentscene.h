#ifndef GRAPHCOMPONENTSCENE_H
#define GRAPHCOMPONENTSCENE_H

#include "scene.h"
#include "openglfunctions.h"

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "transition.h"

class GraphRenderer;
class GraphComponentRenderer;

class GraphComponentScene :
        public Scene,
        protected OpenGLFunctions
{
    Q_OBJECT

public:
    GraphComponentScene(GraphRenderer* graphRenderer);

    void initialise();
    void update(float t);
    void render();
    void setSize(int width, int height);

    bool transitionActive() const;

    void onShow();

    ComponentId componentId() const { return _componentId; }
    void setComponentId(ComponentId componentId);

    int width() const { return _width; }
    int height() const { return _height; }

    void saveViewData();
    bool savedViewIsReset() const;
    void restoreViewData();

    void resetView();
    bool viewIsReset() const;

    GraphComponentRenderer* componentRenderer() const;

    void startTransition(float duration = 0.3f,
                         Transition::Type transitionType = Transition::Type::EaseInEaseOut,
                         std::function<void()> finishedFunction = []{});

private:
    GraphRenderer* _graphRenderer;

    int _width;
    int _height;

    ComponentId _defaultComponentId;
    ComponentId _componentId;

private slots:
    void onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph* graph, const ComponentMergeSet& componentMergeSet);
    void onComponentAdded(const Graph* graph, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool);
    void onGraphChanged(const Graph* graph);
    void onNodeWillBeRemoved(const Graph* graph, NodeId nodeId);
};

#endif // GRAPHCOMPONENTSCENE_H
