#ifndef GRAPHCOMPONENTSCENE_H
#define GRAPHCOMPONENTSCENE_H

#include "scene.h"
#include "graphcomponentrenderersreference.h"

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "transition.h"

class GraphWidget;
class GraphComponentRenderer;

class QOpenGLFunctions_3_3_Core;

class GraphComponentScene : public Scene, public GraphComponentRenderersReference
{
    Q_OBJECT

public:
    GraphComponentScene(GraphWidget* graphWidget);

    static const int multisamples = 4;

    void initialise();
    void update(float t);
    void render();
    void resize(int width, int height);

    void onShow();

    ComponentId componentId() { return _componentId; }
    void setComponentId(ComponentId componentId);

    int width() const { return _width; }
    int height() const { return _height; }

    void saveViewData();
    void restoreViewData();

    void resetView(Transition::Type transitionType = Transition::Type::EaseInEaseOut);
    bool viewIsReset();

    GraphComponentRenderer* renderer();

private:
    GraphWidget* _graphWidget;

    int _width;
    int _height;

    ComponentId _componentId;

    QOpenGLFunctions_3_3_Core* _funcs;

private slots:
    void onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph* graph, const ComponentMergeSet& componentMergeSet);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId, bool);
    void onGraphChanged(const Graph* graph);
};

#endif // GRAPHCOMPONENTSCENE_H
