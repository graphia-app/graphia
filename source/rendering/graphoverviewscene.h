#ifndef GRAPHOVERVIEWSCENE_H
#define GRAPHOVERVIEWSCENE_H

#include "scene.h"
#include "graphcomponentrenderersreference.h"

#include "../graph/graph.h"
#include "../graph/grapharray.h"

#include <QRect>

class GraphWidget;
class GraphModel;
class QOpenGLFunctions_3_3_Core;

class GraphOverviewScene : public Scene, public GraphComponentRenderersReference
{
    Q_OBJECT

public:
    GraphOverviewScene(GraphWidget* graphWidget);

    void initialise();
    void update(float t);
    void render();
    void resize(int width, int height);

    void onShow();
    void onHide();

    struct LayoutData
    {
        LayoutData() : _alpha(1.0f) {}
        LayoutData(const QRect& rect, float alpha) :
            _rect(rect), _alpha(alpha) {}

        QRect _rect;
        float _alpha;
    };

    const ComponentArray<LayoutData>& componentLayout() { return _componentLayout; }

    void zoom(float delta);
    int renderSizeDivisor() { return _renderSizeDivisor; }
    void setRenderSizeDivisor(int divisor);

    void resetView();

private:
    GraphWidget* _graphWidget;
    std::shared_ptr<GraphModel> _graphModel;

    int _width;
    int _height;

    int _renderSizeDivisor;
    ComponentArray<int> _renderSizeDivisors;

    QOpenGLFunctions_3_3_Core* _funcs;

    ComponentArray<LayoutData> _previousComponentLayout;
    ComponentArray<LayoutData> _componentLayout;
    void layoutComponents();

    std::vector<ComponentId> _transitionComponentIds;
    std::vector<ComponentSplitSet> _componentSplitSets;
    std::vector<ComponentMergeSet> _componentMergeSets;
    void startTransition();

private slots:
    void onGraphWillChange(const Graph* graph);
    void onGraphChanged(const Graph* graph);
    void onComponentAdded(const Graph* graph, ComponentId componentId, bool hasSplit);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool hasMerged);
    void onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph* graph, const ComponentMergeSet& componentMergeSet);
};

#endif // GRAPHOVERVIEWSCENE_H
