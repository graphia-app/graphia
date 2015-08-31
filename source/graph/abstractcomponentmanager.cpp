#include "abstractcomponentmanager.h"
#include "../graph/grapharray.h"

#include <QtGlobal>

AbstractComponentManager::AbstractComponentManager(Graph& graph, bool ignoreMultiElements) :
    _ignoreMultiElements(ignoreMultiElements)
{
    if(qgetenv("COMPONENTS_DEBUG").toInt())
        _debug = true;

    connect(&graph, &Graph::graphChanged, this, &AbstractComponentManager::onGraphChanged, Qt::DirectConnection);

    connect(this, &AbstractComponentManager::componentAdded, &graph, &Graph::componentAdded, Qt::DirectConnection);
    connect(this, &AbstractComponentManager::componentWillBeRemoved, &graph, &Graph::componentWillBeRemoved, Qt::DirectConnection);
    connect(this, &AbstractComponentManager::componentSplit, &graph, &Graph::componentSplit, Qt::DirectConnection);
    connect(this, &AbstractComponentManager::componentsWillMerge, &graph, &Graph::componentsWillMerge, Qt::DirectConnection);
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
    _nodeIdsList.reserve(otherGraphComponent->_nodeIdsList.size());
    _edgeIdsList.reserve(otherGraphComponent->_edgeIdsList.size());
}

void GraphComponent::cloneFrom(const Graph& other)
{
    const GraphComponent* otherGraphComponent = dynamic_cast<const GraphComponent*>(&other);
    Q_ASSERT(otherGraphComponent != nullptr);

    Q_ASSERT(_graph == otherGraphComponent->_graph);
    _nodeIdsList = otherGraphComponent->_nodeIdsList;
    _edgeIdsList = otherGraphComponent->_edgeIdsList;
}
