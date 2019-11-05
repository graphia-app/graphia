#ifndef GRAPHOVERVIEWSCENE_H
#define GRAPHOVERVIEWSCENE_H

#include "scene.h"
#include "transition.h"

#include "graph/componentmanager.h"
#include "shared/graph/grapharray.h"

#include "layout/componentlayout.h"

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
void initialiseFromGraph(const Graph*, Target&); // NOLINT

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

    void resetView(bool doTransition) override;
    bool viewIsReset() const override;

    void onProjectionChanged(Projection projection) override;

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
    GraphRenderer* _graphRenderer = nullptr;
    CommandManager* _commandManager = nullptr;
    GraphModel* _graphModel = nullptr;

    int _width = 0;
    int _height = 0;

    float _zoomFactor = 1.0f;
    QPointF _zoomCentre;
    Transition _zoomTransition;
    bool _autoZooming = true;
    QPointF _offset;

    ComponentArray<float, LockingGraphArray> _previousComponentAlpha;
    ComponentArray<float, LockingGraphArray> _componentAlpha;

    std::atomic<bool> _nextComponentLayoutDataChanged;
    ComponentLayoutData _nextComponentLayoutData;
    ComponentLayoutData _componentLayoutData;
    ComponentLayoutData _previousZoomedComponentLayoutData;
    ComponentLayoutData _zoomedComponentLayoutData;
    std::unique_ptr<ComponentLayout> _componentLayout;

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

    void setVisible(bool visible);

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
