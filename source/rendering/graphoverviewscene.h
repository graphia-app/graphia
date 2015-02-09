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

    const ComponentArray<QRect>& componentLayout() { return _componentLayout; }

    void zoom(float delta);
    int renderSizeDivisor() { return _renderSizeDivisor; }
    void setRenderSizeDivisor(int divisor);

private:
    GraphWidget* _graphWidget;
    std::shared_ptr<GraphModel> _graphModel;

    int _width;
    int _height;

    int _renderSizeDivisor;
    ComponentArray<int> _renderSizeDivisors;

    QOpenGLFunctions_3_3_Core* _funcs;

    ComponentArray<QRect> _componentLayout;
    void layoutComponents();

private slots:
    void onGraphChanged(const Graph* graph);
    void onComponentSplit(const Graph* graph, ComponentId oldComponentId, const ElementIdSet<ComponentId>& splitters);
};

#endif // GRAPHOVERVIEWSCENE_H
