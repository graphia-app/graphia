#ifndef GRAPHOVERVIEWSCENE_H
#define GRAPHOVERVIEWSCENE_H

#include "scene.h"
#include "graphrenderer.h"
#include "transition.h"

#include "graph/graph.h"
#include "graph/componentmanager.h"
#include "shared/graph/grapharray.h"

#include "layout/componentlayout.h"

#include <vector>
#include <mutex>
#include <memory>

#include <QRect>
#include <QPointF>

class GraphModel;
class CommandManager;

class GraphOverviewScene :
        public Scene,
        public GraphInitialiser
{
    Q_OBJECT

public:
    explicit GraphOverviewScene(CommandManager& commandManager,
                                GraphRenderer* graphRenderer);

    void update(float t);
    void setViewportSize(int width, int height);

    bool transitionActive() const;

    void onShow();
    void onHide();

    const ComponentLayoutData& componentLayout() { return _zoomedComponentLayoutData; }

    void resetView(bool doTransition);
    bool viewIsReset() const;

    void pan(float dx, float dy);

    enum class ZoomType
    {
        In,
        Out
    };

    void zoom(ZoomType zoomType, float x, float y, bool doTransition);
    void zoom(float delta, float x, float y, bool doTransition);

    void startTransitionFromComponentMode(ComponentId focusComponentId,
                                          std::function<void()> finishedFunction = []{},
                                          float duration = 0.3f,
                                          Transition::Type transitionType = Transition::Type::EaseInEaseOut);
    void startTransitionToComponentMode(ComponentId focusComponentId,
                                        std::function<void()> finishedFunction = []{},
                                        float duration = 0.3f,
                                        Transition::Type transitionType = Transition::Type::EaseInEaseOut);

private:
    GraphRenderer* _graphRenderer;
    CommandManager* _commandManager;
    std::shared_ptr<GraphModel> _graphModel;

    int _width = 0;
    int _height = 0;

    float _zoomFactor = 1.0f;
    QPointF _zoomCentre;
    Transition _zoomTransition;
    bool _autoZooming = true;
    QPointF _offset;

    ComponentArray<float, u::Locking> _previousComponentAlpha;
    ComponentArray<float, u::Locking> _componentAlpha;

    std::atomic<bool> _nextComponentLayoutDataChanged;
    ComponentLayoutData _nextComponentLayoutData;
    ComponentLayoutData _componentLayoutData;
    ComponentLayoutData _previousZoomedComponentLayoutData;
    ComponentLayoutData _zoomedComponentLayoutData;
    std::shared_ptr<ComponentLayout> _componentLayout;

    void updateZoomedComponentLayoutData();
    void applyComponentLayout();

    std::vector<ComponentId> _removedComponentIds;
    std::vector<ComponentMergeSet> _componentMergeSets;
    void startTransition(std::function<void()> finishedFunction = []{}, float duration = 1.0f,
                         Transition::Type transitionType = Transition::Type::EaseInEaseOut);
    void startZoomTransition(float duration = 0.3f);

    std::vector<ComponentId> _componentIds;

    Circle zoomedLayoutData(const Circle& data);

    float minZoomFactor() const;
    bool setZoomFactor(float zoomFactor);
    void setOffset(float x, float y);

    void startComponentLayoutTransition();

private slots:
    void onGraphWillChange(const Graph* graph);
    void onGraphChanged(const Graph* graph, bool changed);
    void onComponentAdded(const Graph* graph, ComponentId componentId, bool hasSplit);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool hasMerged);
    void onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph* graph, const ComponentMergeSet& componentMergeSet);

    void onPreferenceChanged(const QString& key, const QVariant& value);
};

#endif // GRAPHOVERVIEWSCENE_H
