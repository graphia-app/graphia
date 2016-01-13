#ifndef GRAPHOVERVIEWSCENE_H
#define GRAPHOVERVIEWSCENE_H

#include "scene.h"
#include "graphrenderer.h"
#include "transition.h"

#include "../graph/graph.h"
#include "../graph/componentmanager.h"
#include "../graph/grapharray.h"

#include <vector>
#include <mutex>
#include <memory>

#include <QRect>

class GraphModel;

class GraphOverviewScene :
        public Scene,
        public GraphInitialiser
{
    Q_OBJECT

public:
    explicit GraphOverviewScene(GraphRenderer* graphRenderer);

    void update(float t);
    void setViewportSize(int width, int height);

    bool transitionActive() const;

    void onShow();
    void onHide();

    const ComponentArray<QRect, u::Locking>& componentLayout() { return _componentLayout; }

    void zoom(float delta);
    int renderSizeDivisor() { return _renderSizeDivisor; }
    void setRenderSizeDivisor(int divisor);

    void startTransitionFromComponentMode(ComponentId focusComponentId,
                                          float duration = 0.3f,
                                          Transition::Type transitionType = Transition::Type::EaseInEaseOut,
                                          std::function<void()> finishedFunction = []{});
    void startTransitionToComponentMode(ComponentId focusComponentId,
                                        float duration = 0.3f,
                                        Transition::Type transitionType = Transition::Type::EaseInEaseOut,
                                        std::function<void()> finishedFunction = []{});

private:
    GraphRenderer* _graphRenderer;
    std::shared_ptr<GraphModel> _graphModel;

    int _width = 0;
    int _height = 0;

    int _renderSizeDivisor = 1;

    ComponentArray<float, u::Locking> _previousComponentAlpha;
    ComponentArray<float, u::Locking> _componentAlpha;

    ComponentArray<QRect, u::Locking> _previousComponentLayout;
    ComponentArray<QRect, u::Locking> _componentLayout;
    void layoutComponents();

    std::vector<ComponentId> _removedComponentIds;
    std::vector<ComponentMergeSet> _componentMergeSets;
    void startTransition(float duration = 1.0f,
                         Transition::Type transitionType = Transition::Type::EaseInEaseOut,
                         std::function<void()> finishedFunction = []{});

    std::vector<ComponentId> _componentIds;

private slots:
    void onGraphWillChange(const Graph* graph);
    void onGraphChanged(const Graph* graph);
    void onComponentAdded(const Graph* graph, ComponentId componentId, bool hasSplit);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool hasMerged);
    void onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph* graph, const ComponentMergeSet& componentMergeSet);
};

#endif // GRAPHOVERVIEWSCENE_H
