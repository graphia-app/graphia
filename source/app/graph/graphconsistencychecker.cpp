/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphconsistencychecker.h"
#include "graph.h"
#include "graphcomponent.h"
#include "componentmanager.h"

#include "shared/graph/elementid_debug.h"
#include "shared/utils/container.h"

GraphConsistencyChecker::GraphConsistencyChecker(const Graph& graph) :
    _graph(&graph)
{
}

void GraphConsistencyChecker::toggle()
{
    if(!_enabled)
        enable();
    else
        disable();
}

void GraphConsistencyChecker::enable()
{
    if(!_enabled)
    {
        connect(_graph, &Graph::graphChanged, this, &GraphConsistencyChecker::onGraphChanged, Qt::DirectConnection);
        _enabled = true;
    }
}

void GraphConsistencyChecker::disable()
{
    if(_enabled)
    {
        disconnect(_graph, &Graph::graphChanged, this, &GraphConsistencyChecker::onGraphChanged);
        _enabled = false;
    }
}

template<typename G, typename C> bool check(const G& graph, const C& component,
                                            ComponentId thisComponentId = ComponentId());

static bool checkComponents(const Graph& graph)
{
    bool consistent = true;

    for(auto componentId : graph.componentIds())
    {
        const auto* component = graph.componentById(componentId);
        consistent = check(graph, *component, componentId);
    }

    return consistent;
}

static bool checkComponents(const IGraphComponent&) { return true; }

template<typename G, typename C> bool check(const G& graph, const C& component,
                                            ComponentId thisComponentId)
{
    bool consistent = true;

    for(auto edgeId : component.edgeIds())
    {
        if(graph.containsEdgeId(edgeId))
        {
            auto& edge = graph.edgeById(edgeId);
            auto sourceId = edge.sourceId();
            auto targetId = edge.targetId();

            if(graph.containsNodeId(sourceId))
            {
                if(!u::contains(graph.edgeIdsForNodeId(sourceId), edgeId))
                {
                    consistent = false;
                    qDebug() << "Node" << sourceId << "'s edges don't contain" << edgeId;
                }
            }
            else
            {
                consistent = false;
                qDebug() << "Edge" << edgeId << "'s source" << sourceId << "is not in the graph";
            }

            if(graph.containsNodeId(targetId))
            {
                if(!u::contains(graph.edgeIdsForNodeId(targetId), edgeId))
                {
                    consistent = false;
                    qDebug() << "Node" << targetId << "'s edges don't contain" << edgeId;
                }
            }
            else
            {
                consistent = false;
                qDebug() << "Edge" << edgeId << "'s target" << targetId << "is not in the graph";
            }
        }
        else
        {
            consistent = false;
            qDebug() << "Edge" << edgeId << "is in edgeIds(), but not in the graph";
        }
    }

    for(auto nodeId : component.nodeIds())
    {
        if(graph.containsNodeId(nodeId))
        {
            for(auto edgeId : graph.edgeIdsForNodeId(nodeId))
            {
                if(graph.containsEdgeId(edgeId))
                {
                    auto& edge = graph.edgeById(edgeId);
                    if(nodeId != edge.sourceId() && nodeId != edge.targetId())
                    {
                        consistent = false;
                        qDebug() << "Node" << nodeId << "has edge" << edgeId << "but not vice versa";
                    }
                }
                else
                {
                    consistent = false;
                    qDebug() << "Edge" << edgeId << "is in node" << nodeId << "'s edges, but not in the graph";
                }
            }
        }
        else
        {
            consistent = false;
            qDebug() << "Node" << nodeId << "is in nodeIds(), but not in the graph";
        }
    }

    checkComponents(component);

    if(!consistent)
    {
        if(thisComponentId.isNull())
            graph.dumpToQDebug(2);
        else
            qDebug() << "Component" << thisComponentId << "is inconsistent";
    }

    return consistent;
}

void GraphConsistencyChecker::onGraphChanged(const Graph* graph)
{
    check(*graph, *graph);
}

