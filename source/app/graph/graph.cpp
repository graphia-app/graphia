#include "graph.h"
#include "shared/graph/igrapharray.h"
#include "componentmanager.h"

#include <QtGlobal>
#include <QMetaType>
#include <QDebug>

static void registerQtTypes()
{
    static bool registered = false;
    if(!registered)
    {
        qRegisterMetaType<NodeId>("NodeId");
        qRegisterMetaType<NodeIdSet>("NodeIdSet");
        qRegisterMetaType<EdgeId>("EdgeId");
        qRegisterMetaType<EdgeIdSet>("EdgeIdSet");
        qRegisterMetaType<ComponentId>("ComponentId");
        qRegisterMetaType<ComponentIdSet>("ComponentIdSet");

        registered = true;
    }
}

std::vector<EdgeId> Node::inEdgeIds() const
{
    std::vector<EdgeId> edgeIds;

    for(EdgeId edgeId : _inEdgeIds)
        edgeIds.emplace_back(edgeId);

    return edgeIds;
}

std::vector<EdgeId> Node::outEdgeIds() const
{
    std::vector<EdgeId> edgeIds;

    for(EdgeId edgeId : _outEdgeIds)
        edgeIds.emplace_back(edgeId);

    return edgeIds;
}

std::vector<EdgeId> Node::edgeIds() const
{
    std::vector<EdgeId> edgeIds;

    for(EdgeId edgeId : _inEdgeIds)
        edgeIds.emplace_back(edgeId);

    for(EdgeId edgeId : _outEdgeIds)
        edgeIds.emplace_back(edgeId);

    return edgeIds;
}

Graph::Graph() :
    _nextNodeId(0), _nextEdgeId(0),
    _graphConsistencyChecker(*this)
{
    registerQtTypes();

    connect(this, &Graph::nodeAdded, [this](const Graph*, NodeId nodeId) { reserveNodeId(nodeId); });
    connect(this, &Graph::edgeAdded, [this](const Graph*, EdgeId edgeId) { reserveEdgeId(edgeId); });
}

Graph::~Graph()
{
    // Let the GraphArrays know that we're going away
    for(auto nodeArray : _nodeArrays)
        nodeArray->invalidate();

    for(auto edgeArray : _edgeArrays)
        edgeArray->invalidate();
}

NodeId Graph::firstNodeId() const
{
    return nodeIds().size() > 0 ? nodeIds().at(0) : NodeId();
}

bool Graph::containsNodeId(NodeId nodeId) const
{
    return u::contains(nodeIds(), nodeId);
}

EdgeId Graph::firstEdgeId() const
{
    return edgeIds().size() > 0 ? edgeIds().at(0) : EdgeId();
}

bool Graph::containsEdgeId(EdgeId edgeId) const
{
    return u::contains(edgeIds(), edgeId);
}

void Graph::reserve(const Graph& other)
{
    reserveNodeId(other.largestNodeId());
    reserveEdgeId(other.largestEdgeId());
}

void Graph::enableComponentManagement()
{
    if(_componentManager == nullptr)
    {
        _componentManager = std::make_unique<ComponentManager>(*this);

        connect(_componentManager.get(), &ComponentManager::componentAdded,             this, &Graph::componentAdded,           Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::componentWillBeRemoved,     this, &Graph::componentWillBeRemoved,   Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::componentSplit,             this, &Graph::componentSplit,           Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::componentsWillMerge,        this, &Graph::componentsWillMerge,      Qt::DirectConnection);

        connect(_componentManager.get(), &ComponentManager::nodeAddedToComponent,       this, &Graph::nodeAddedToComponent,     Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::nodeRemovedFromComponent,   this, &Graph::nodeRemovedFromComponent, Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::edgeAddedToComponent,       this, &Graph::edgeAddedToComponent,     Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::edgeRemovedFromComponent,   this, &Graph::edgeRemovedFromComponent, Qt::DirectConnection);

        if(qEnvironmentVariableIntValue("COMPONENTS_DEBUG"))
            _componentManager->enableDebug();
    }
}

template<typename G, typename C> void dumpGraphToQDebug(const G& graph, const C& component, const int detail)
{
    qDebug() << component.numNodes() << "nodes" << component.numEdges() << "edges";

    if(detail > 0)
    {
        for(auto nodeId : component.nodeIds())
        {
            auto edgeIds = graph.edgeIdsForNodeId(nodeId);
            qDebug() << "Node" << nodeId << edgeIds;
        }

        for(auto edgeId : component.edgeIds())
        {
            auto& edge = graph.edgeById(edgeId);
            qDebug() << "Edge" << edgeId << "(" << edge.sourceId() << "->" << edge.targetId() << ")";
        }
    }
}

void Graph::dumpToQDebug(int detail) const
{
    dumpGraphToQDebug(*this, *this, detail);

    if(detail > 1)
    {
        if(_componentManager)
        {
            for(auto componentId : _componentManager->componentIds())
            {
                auto component = _componentManager->componentById(componentId);
                qDebug() << "component" << componentId;
                dumpGraphToQDebug(*this, *component, detail);
            }
        }
    }
}

void Graph::insertNodeArray(IGraphArray* nodeArray) const
{
    std::unique_lock<std::mutex> lock(_nodeArraysMutex);
    _nodeArrays.insert(nodeArray);
}

void Graph::eraseNodeArray(IGraphArray* nodeArray) const
{
    std::unique_lock<std::mutex> lock(_nodeArraysMutex);
    _nodeArrays.erase(nodeArray);
}

void Graph::insertEdgeArray(IGraphArray* edgeArray) const
{
    std::unique_lock<std::mutex> lock(_edgeArraysMutex);
    _edgeArrays.insert(edgeArray);
}

void Graph::eraseEdgeArray(IGraphArray* edgeArray) const
{
    std::unique_lock<std::mutex> lock(_edgeArraysMutex);
    _edgeArrays.erase(edgeArray);
}

int Graph::numComponentArrays() const
{
    if(_componentManager)
        return _componentManager->componentArrayCapacity();

    return 0;
}

void Graph::insertComponentArray(IGraphArray* componentArray) const
{
    if(_componentManager)
        _componentManager->insertComponentArray(componentArray);
}

void Graph::eraseComponentArray(IGraphArray* componentArray) const
{
    if(_componentManager)
        _componentManager->eraseComponentArray(componentArray);
}

NodeId Graph::nextNodeId() const
{
    return _nextNodeId;
}

EdgeId Graph::nextEdgeId() const
{
    return _nextEdgeId;
}

void Graph::reserveNodeId(NodeId nodeId)
{
    if(nodeId < nextNodeId())
        return;

    _nextNodeId = nodeId + 1;

    for(auto nodeArray : _nodeArrays)
        nodeArray->resize(_nextNodeId);
}

void Graph::reserveEdgeId(EdgeId edgeId)
{
    if(edgeId < nextEdgeId())
        return;

    _nextEdgeId = edgeId + 1;

    for(auto edgeArray : _edgeArrays)
        edgeArray->resize(_nextEdgeId);
}

void Graph::clear()
{
    _nextNodeId = 0;
    _nextEdgeId = 0;
}

const std::vector<ComponentId>& Graph::componentIds() const
{
    if(_componentManager)
        return _componentManager->componentIds();

    static std::vector<ComponentId> emptyComponentIdList;

    return emptyComponentIdList;
}

int Graph::numComponents() const
{
    return static_cast<int>(componentIds().size());
}

bool Graph::containsComponentId(ComponentId componentId) const
{
    if(_componentManager)
        return _componentManager->containsComponentId(componentId);

    Q_ASSERT(!"Graph::containsComponentId called with component management disabled");
    return false;
}

const IGraphComponent* Graph::componentById(ComponentId componentId) const
{
    if(_componentManager)
        return _componentManager->componentById(componentId);

    Q_ASSERT(!"Graph::componentById called with component management disabled");
    return nullptr;
}

ComponentId Graph::componentIdOfNode(NodeId nodeId) const
{
    if(_componentManager)
        return _componentManager->componentIdOfNode(nodeId);

    return ComponentId();
}

ComponentId Graph::componentIdOfEdge(EdgeId edgeId) const
{
    if(_componentManager)
        return _componentManager->componentIdOfEdge(edgeId);

    return ComponentId();
}

ComponentId Graph::componentIdOfLargestComponent() const
{
    return componentIdOfLargestComponent(componentIds());
}

std::vector<NodeId> Graph::sourcesOf(NodeId nodeId) const
{
    std::vector<NodeId> nodeIds;

    const auto& node = nodeById(nodeId);

    for(auto edgeId : node.inEdgeIds())
    {
        const auto& edge = edgeById(edgeId);
        nodeIds.emplace_back(edge.oppositeId(nodeId));
    }

    return nodeIds;
}

std::vector<NodeId> Graph::targetsOf(NodeId nodeId) const
{
    std::vector<NodeId> nodeIds;

    const auto& node = nodeById(nodeId);

    for(auto edgeId : node.outEdgeIds())
    {
        const auto& edge = edgeById(edgeId);
        nodeIds.emplace_back(edge.oppositeId(nodeId));
    }

    return nodeIds;
}

std::vector<NodeId> Graph::neighboursOf(NodeId nodeId) const
{
    std::vector<NodeId> nodeIds;

    const auto& node = nodeById(nodeId);

    for(auto edgeId : node.edgeIds())
    {
        const auto& edge = edgeById(edgeId);
        nodeIds.emplace_back(edge.oppositeId(nodeId));
    }

    return nodeIds;
}

void Graph::setPhase(const QString& phase) const
{
    std::unique_lock<std::recursive_mutex> lock(_phaseMutex);

    if(phase != _phase)
    {
        _phase = phase;
        emit phaseChanged();
    }
}

void Graph::clearPhase() const
{
    std::unique_lock<std::recursive_mutex> lock(_phaseMutex);

    setPhase(QLatin1String(""));
}

QString Graph::phase() const
{
    std::unique_lock<std::recursive_mutex> lock(_phaseMutex);
    return _phase;
}
