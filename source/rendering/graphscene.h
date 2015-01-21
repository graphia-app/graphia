#ifndef GRAPHSCENE_H
#define GRAPHSCENE_H

#include "scene.h"
#include "graphcomponentrenderersreference.h"

#include "../graph/graph.h"
#include "../graph/grapharray.h"

class GraphWidget;

class GraphScene : public Scene, public GraphComponentRenderersReference
{
    Q_OBJECT

public:
    GraphScene(GraphWidget* graphWidget);

    void initialise();
    void cleanup();
    void update(float t);
    void render();
    void resize(int width, int height);

private:
    GraphWidget* _graphWidget;

    ComponentArray<int> _renderSizeDivisors;

private slots:
    void onGraphChanged(const Graph* graph);
};

#endif // GRAPHSCENE_H
