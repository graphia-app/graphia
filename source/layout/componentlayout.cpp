#include "componentlayout.h"

void ComponentLayout::execute(const Graph& graph, const std::vector<ComponentId>& componentIds,
                              ComponentLayoutData& componentLayoutData)
{
    executeReal(graph, componentIds, componentLayoutData);

    _boundingBox = QRectF();

    for(auto componentId : componentIds)
        _boundingBox = _boundingBox.united(componentLayoutData[componentId].boundingBox());

    for(auto componentId : componentIds)
        componentLayoutData[componentId].translate(-_boundingBox.topLeft());

    _boundingBox.translate(-_boundingBox.x(), -_boundingBox.y());
}
