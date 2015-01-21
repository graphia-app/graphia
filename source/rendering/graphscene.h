#ifndef GRAPHSCENE_H
#define GRAPHSCENE_H

#include "scene.h"
#include "graphcomponentrenderersreference.h"

class GraphWidget;

class GraphScene : public Scene, public GraphComponentRenderersReference
{
    Q_OBJECT

public:
    GraphScene(GraphWidget* parent = nullptr);

    void initialise();
    void cleanup();
    void update(float t);
    void render();
    void resize(int width, int height);
};

#endif // GRAPHSCENE_H
