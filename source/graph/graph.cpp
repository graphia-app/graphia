#include "graph.h"
#include "grapharray.h"
#include "componentmanager.h"
#include "../utils/utils.h"
#include "../utils/cpp1x_hacks.h"

#include <QtGlobal>
#include <QMetaType>
#include <QDebug>

#include <tuple>

Graph::Graph() :
    _nextNodeId(0), _nextEdgeId(0)
{
    qRegisterMetaType<NodeId>("NodeId");
    qRegisterMetaType<NodeIdSet>("NodeIdSet");
    qRegisterMetaType<EdgeId>("EdgeId");
    qRegisterMetaType<EdgeIdSet>("EdgeIdSet");
    qRegisterMetaType<ComponentId>("ComponentId");
    qRegisterMetaType<ComponentIdSet>("ComponentIdSet");

    connect(this, &Graph::nodeAdded, [this](const Graph*, const Node* node) { reserveNodeId(node->id()); });
    connect(this, &Graph::edgeAdded, [this](const Graph*, const Edge* edge) { reserveEdgeId(edge->id()); });
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

void Graph::enableComponentManagement()
{
    if(_componentManager == nullptr)
    {
        _componentManager = std::make_unique<ComponentManager>(*this);

        connect(_componentManager.get(), &ComponentManager::componentAdded,         this, &Graph::componentAdded, Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::componentWillBeRemoved, this, &Graph::componentWillBeRemoved, Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::componentSplit,         this, &Graph::componentSplit, Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::componentsWillMerge,    this, &Graph::componentsWillMerge, Qt::DirectConnection);
    }
}

void Graph::dumpToQDebug(int detail) const
{
    qDebug() << numNodes() << "nodes" << numEdges() << "edges";

    if(detail > 0)
    {
        for(auto nodeId : nodeIds())
        {
            auto& node = nodeById(nodeId);
            qDebug() << "Node" << nodeId << node.edgeIds();
        }

        for(auto edgeId : edgeIds())
        {
            auto& edge = edgeById(edgeId);
            qDebug() << "Edge" << edgeId << "(" << edge.sourceId() << "->" << edge.targetId() << ")";
        }
    }

    if(detail > 1)
    {
        if(_componentManager)
        {
            for(auto componentId : _componentManager->componentIds())
            {
                auto component = _componentManager->componentById(componentId);
                qDebug() << "component" << componentId;
                component->dumpToQDebug(detail);
            }
        }
    }
}

int Graph::numComponentArrays() const
{
    if(_componentManager)
        return _componentManager->componentArrayCapacity();

    return 0;
}

void Graph::insertComponentArray(GraphArray* componentArray) const
{
    if(_componentManager)
        _componentManager->_componentArrays.insert(componentArray);
}

void Graph::eraseComponentArray(GraphArray* componentArray) const
{
    if(_componentManager)
        _componentManager->_componentArrays.erase(componentArray);
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

const Graph* Graph::componentById(ComponentId componentId) const
{
    if(_componentManager)
        return _componentManager->componentById(componentId);

    Q_ASSERT(!"Graph::componentById returning nullptr");
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

void Graph::setPhase(const QString& phase) const
{
    if(phase != _phase)
    {
        _phase = phase;
        emit phaseChanged();
    }
}

void Graph::clearPhase() const
{
    setPhase("");
}

const QString&Graph::phase() const
{
    return _phase;
}
