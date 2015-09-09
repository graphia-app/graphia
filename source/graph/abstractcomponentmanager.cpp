#include "abstractcomponentmanager.h"
#include "../graph/grapharray.h"

#include <QtGlobal>

AbstractComponentManager::AbstractComponentManager(Graph& graph, bool ignoreMultiElements) :
    _ignoreMultiElements(ignoreMultiElements)
{
    if(qgetenv("COMPONENTS_DEBUG").toInt())
        _debug = true;

    connect(&graph, &Graph::graphChanged, this, &AbstractComponentManager::onGraphChanged, Qt::DirectConnection);
}

AbstractComponentManager::~AbstractComponentManager()
{
    // Let the ComponentArrays know that we're going away
    for(auto componentArray : _componentArrays)
        componentArray->invalidate();
}

void GraphComponent::reserve(const Graph& other)
{
    const GraphComponent* otherGraphComponent = dynamic_cast<const GraphComponent*>(&other);
    Q_ASSERT(otherGraphComponent != nullptr);

    Q_ASSERT(_graph == otherGraphComponent->_graph);
    _nodeIds.reserve(otherGraphComponent->_nodeIds.size());
    _edgeIds.reserve(otherGraphComponent->_edgeIds.size());
}

void GraphComponent::cloneFrom(const Graph& other)
{
    const GraphComponent* otherGraphComponent = dynamic_cast<const GraphComponent*>(&other);
    Q_ASSERT(otherGraphComponent != nullptr);

    Q_ASSERT(_graph == otherGraphComponent->_graph);
    _nodeIds = otherGraphComponent->_nodeIds;
    _edgeIds = otherGraphComponent->_edgeIds;
}
