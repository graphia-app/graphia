/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "cxparser.h"

#include "shared/utils/container.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/elementid.h"
#include "shared/graph/imutablegraph.h"

#include <map>

CxParser::CxParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData) :
    _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
}

bool parseCx1(const json& jsonArray, IGraphModel* graphModel,
    IParser* parser, IUserNodeData* userNodeData, IUserEdgeData* userEdgeData)
{
    std::map<int, NodeId> cxIdToNodeId;
    std::map<int, EdgeId> cxIdToEdgeId;

    for(const auto& j : jsonArray)
    {
        if(u::contains(j, "nodes"))
        {
            auto nodesArray = j["nodes"];
            auto numNodes = static_cast<int>(nodesArray.size());
            int i = 0;
            for(const auto& node : nodesArray)
            {
                if(!u::contains(node, "@id"))
                {
                    parser->setFailureReason(QObject::tr("Node is missing ID."));
                    return false;
                }

                auto nodeId = graphModel->mutableGraph().addNode();

                if(!node["@id"].is_number())
                {
                    parser->setFailureReason(QObject::tr("Node ID is not a number."));
                    return false;
                }

                auto cxId = node["@id"].get<int>();
                cxIdToNodeId[cxId] = nodeId;

                if(u::contains(node, "n") && node["n"].is_string())
                {
                    auto name = QString::fromStdString(node["n"].get<std::string>());
                    userNodeData->setValueBy(nodeId, QObject::tr("Node Name"), name);
                    graphModel->setNodeName(nodeId, name);
                }

                if(u::contains(node, "r") && node["r"].is_string())
                {
                    auto represents = QString::fromStdString(node["r"].get<std::string>());
                    userNodeData->setValueBy(nodeId, QObject::tr("Node Represents"), represents);
                }

                parser->setProgress((i++ * 100) / numNodes);

                if(parser->cancelled())
                    return false;
            }
        }
    }

    parser->setProgress(-1);

    for(const auto& j : jsonArray)
    {
        if(u::contains(j, "edges"))
        {
            auto edgesArray = j["edges"];
            auto numEdges = static_cast<int>(edgesArray.size());
            int i = 0;
            for(const auto& edge : edgesArray)
            {
                if(!u::contains(edge, "@id"))
                {
                    parser->setFailureReason(QObject::tr("Edge is missing ID."));
                    return false;
                }

                if(!u::contains(edge, "s") || !u::contains(edge, "t"))
                {
                    parser->setFailureReason(QObject::tr("Edge is missing source and/or target IDs."));
                    return false;
                }

                auto sourceCxId = edge["s"].is_number() ? edge["s"].get<int>() : -1;
                auto targetCxId = edge["t"].is_number() ? edge["t"].get<int>() : -1;

                if(!u::contains(cxIdToNodeId, sourceCxId) || !u::contains(cxIdToNodeId, targetCxId))
                {
                    parser->setFailureReason(QObject::tr("Edge source or target ID is unknown."));
                    return false;
                }

                auto sourceId = cxIdToNodeId.at(sourceCxId);
                auto targetId = cxIdToNodeId.at(targetCxId);

                auto edgeId = graphModel->mutableGraph().addEdge(sourceId, targetId);

                if(!edge["@id"].is_number())
                {
                    parser->setFailureReason(QObject::tr("Edge ID is not a number."));
                    return false;
                }

                auto cxId = edge["@id"].get<int>();
                cxIdToEdgeId[cxId] = edgeId;

                if(u::contains(edge, "i") && edge["i"].is_string())
                {
                    auto interaction = QString::fromStdString(edge["i"].get<std::string>());
                    userEdgeData->setValueBy(edgeId, QObject::tr("Edge Interaction"), interaction);
                }

                parser->setProgress((i++ * 100) / numEdges);

                if(parser->cancelled())
                    return false;
            }
        }
    }

    auto addAttribute = [parser](const json& attribute, const char* elementType, const auto& idMap, auto& userData)
    {
        if(!u::contains(attribute, "po") || !attribute["po"].is_number())
        {
            parser->setFailureReason(QObject::tr("%1 attribute is missing ID.").arg(elementType));
            return false;
        }

        if(!u::contains(attribute, "n") || !u::contains(attribute, "v"))
        {
            parser->setFailureReason(QObject::tr("%1 attribute is missing name or value.").arg(elementType));
            return false;
        }

        if(!attribute["n"].is_string())
        {
            parser->setFailureReason(QObject::tr("%1 attribute name is not a string.").arg(elementType));
            return false;
        }

        if(!attribute["v"].is_string() && !attribute["v"].is_array())
        {
            parser->setFailureReason(QObject::tr("%1 attribute value is not a string or list.").arg(elementType));
            return false;
        }

        auto cxId = attribute["po"].get<int>();

        if(!u::contains(idMap, cxId))
        {
            parser->setFailureReason(QObject::tr("%1 ID not in graph.").arg(elementType));
            return false;
        }

        auto elementId = idMap.at(cxId);
        auto attributeName = QObject::tr("%1 %2")
            .arg(elementType, QString::fromStdString(attribute["n"].get<std::string>()));
        auto attributeValue = attribute["v"].is_string() ?
            QString::fromStdString(attribute["v"].get<std::string>()) :
            QString::fromStdString(attribute["v"].dump());

        userData.setValueBy(elementId, attributeName, attributeValue);

        return true;
    };

    parser->setProgress(-1);

    for(const auto& j : jsonArray)
    {
        if(u::contains(j, "nodeAttributes"))
        {
            auto nodeAttributesArray = j["nodeAttributes"];
            auto numNodeAttributes = static_cast<int>(nodeAttributesArray.size());
            int i = 0;
            for(const auto& nodeAttribute : nodeAttributesArray)
            {
                if(!addAttribute(nodeAttribute, "Node", cxIdToNodeId, *userNodeData))
                    return false;

                parser->setProgress((i++ * 100) / numNodeAttributes);

                if(parser->cancelled())
                    return false;
            }
        }
        else if(u::contains(j, "edgeAttributes"))
        {
            auto edgeAttributesArray = j["edgeAttributes"];
            auto numEdgeAttributes = static_cast<int>(edgeAttributesArray.size());
            int i = 0;
            for(const auto& edgeAttribute : edgeAttributesArray)
            {
                if(!addAttribute(edgeAttribute, "Edge", cxIdToEdgeId, *userEdgeData))
                    return false;

                parser->setProgress((i++ * 100) / numEdgeAttributes);

                if(parser->cancelled())
                    return false;
            }
        }
    }

    return true;
}

bool CxParser::parseJson(const json& jsonArray, IGraphModel* graphModel)
{
    if(jsonArray.is_null() || !jsonArray.is_array() || jsonArray.empty())
    {
        setFailureReason(QObject::tr("Body is empty, or not an array."));
        return false;
    }

    auto arrayContainsNonObjects = std::any_of(jsonArray.begin(), jsonArray.end(), [](const auto& j)
    {
        return !j.is_object();
    });

    if(arrayContainsNonObjects)
    {
        setFailureReason(QObject::tr("Body contains elements that aren't objects."));
        return false;
    }

    bool version1 = u::contains(jsonArray.at(0), "numberVerification");
    if(version1)
        return parseCx1(jsonArray, graphModel, this, _userNodeData, _userEdgeData);

    setFailureReason(QObject::tr("Unknown version."));
    return false;
}
