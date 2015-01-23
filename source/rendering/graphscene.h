#ifndef GRAPHSCENE_H
#define GRAPHSCENE_H

#include "scene.h"
#include "graphcomponentrenderersreference.h"

#include "../graph/graph.h"
#include "../graph/grapharray.h"

#include <vector>

class GraphWidget;
class GraphModel;
class QOpenGLFunctions_3_3_Core;

class GraphScene : public Scene, public GraphComponentRenderersReference
{
    Q_OBJECT

public:
    GraphScene(GraphWidget* graphWidget);

    void initialise();
    void update(float t);
    void render();
    void resize(int width, int height);

    void onShow();
    void onHide();

private:
    GraphWidget* _graphWidget;
    std::shared_ptr<GraphModel> _graphModel;

    int _width;
    int _height;

    std::vector<ComponentId> _sortedComponentIds;
    ComponentArray<int> _renderSizeDivisors;

    QOpenGLFunctions_3_3_Core* _funcs;

private slots:
    void onGraphChanged(const Graph* graph);
    void onComponentSplit(const Graph* graph, ComponentId oldComponentId, const ElementIdSet<ComponentId>& splitters);
};

#endif // GRAPHSCENE_H
