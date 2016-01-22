#ifndef GRAPHOVERVIEWSCENE_H
#define GRAPHOVERVIEWSCENE_H

#include "scene.h"
#include "graphrenderer.h"
#include "transition.h"

#include "../graph/graph.h"
#include "../graph/componentmanager.h"
#include "../graph/grapharray.h"

#include "../layout/componentlayout.h"

#include <vector>
#include <mutex>
#include <memory>

#include <QRect>
#include <QPointF>

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

    const ComponentLayoutData& componentLayout() { return _zoomedComponentLayoutData; }

    void pan(float dx, float dy);
    void zoom(float delta, float x, float y, bool doTransition);
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
    float _zoomFactor = 1.0f;
    QPointF _zoomCentre;
    QPointF _offset;

    ComponentArray<float, u::Locking> _previousComponentAlpha;
    ComponentArray<float, u::Locking> _componentAlpha;

    ComponentLayoutData _componentLayoutData;
    ComponentLayoutData _previousZoomedComponentLayoutData;
    ComponentLayoutData _zoomedComponentLayoutData;
    std::shared_ptr<ComponentLayout> _componentLayout;
    QRectF _componentsBoundingBox;

    void updateComponentLayoutBoundingBox();
    void updateZoomedComponentLayoutData();
    void layoutComponents();

    std::vector<ComponentId> _removedComponentIds;
    std::vector<ComponentMergeSet> _componentMergeSets;
    void startTransition(float duration = 1.0f,
                         Transition::Type transitionType = Transition::Type::EaseInEaseOut,
                         std::function<void()> finishedFunction = []{});
    void startZoomTransition(float duration = 0.3f);

    std::vector<ComponentId> _componentIds;

    QRectF zoomedRect(const QRectF& rect);

    bool setZoomFactor(float zoomFactor);

private slots:
    void onGraphWillChange(const Graph* graph);
    void onGraphChanged(const Graph* graph);
    void onComponentAdded(const Graph* graph, ComponentId componentId, bool hasSplit);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool hasMerged);
    void onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph* graph, const ComponentMergeSet& componentMergeSet);
};

#endif // GRAPHOVERVIEWSCENE_H
