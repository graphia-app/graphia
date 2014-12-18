#ifndef GRAPHSCENE_H
#define GRAPHSCENE_H

#include "scene.h"
#include "graphcomponentviewdata.h"
#include "../graph/grapharray.h"

#include <QObject>

#include <memory>

class GraphScene : public Scene
{
    Q_OBJECT

public:
    GraphScene(std::shared_ptr<ComponentArray<GraphComponentViewData>> componentsViewData,
               QObject* parent = nullptr);

    void initialise();
    void cleanup();
    void update(float t);
    void render();
    void resize(int w, int h);
};

#endif // GRAPHSCENE_H
