/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "biopaxfileparser.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QUrl>

#include <stack>
#include <map>

// http://www.biopax.org/owldoc/Level3/
// Effectively, Entity and all subclasses are Nodes.
// The members and properties of entities define the edges

// CamelCase is Class definitions, mixedCase is properties

namespace
{
bool isNodeElementName(const QString& name)
{
    const QStringList nodeElementNames =
    {
        "Entity",
        "Interaction",
        "PhysicalEntity",
        "Conversion",
        "Pathway",
        "DnaRegion",
        "SmallMolecule",
        "Dna",
        "Rna",
        "Complex",
        "Protein",
        "RnaRegion",
        "Gene",
        "Complex",
        "BiochemicalReaction",
        "Control",
        "Catalysis",
        "Degradation",
        "GeneticInteraction",
        "MolecularInteraction",
        "Modulation",
        "TemplateReaction",
        "TemplateReactionRegulation",
        "Transport",
        "TransportWithBiochemicalReaction"
    };

    return nodeElementNames.contains(name);
}

bool isEdgeElementName(const QString& name)
{
    // Edges are participant object members subclasses
    // http://www.biopax.org/owldoc/Level3/objectproperties/participant___-1675119396.html
    // Complex and Pathway components are linked by edges too
    const QStringList edgeElementNames =
    {
        "pathwayComponent",
        "memberPhysicalEntity",
        "left",
        "right",
        "controller",
        "controlled",
        "component",
        "product",
        "cofactor",
        "template",
        "participant"
    };

    return edgeElementNames.contains(name);
}
} // namespace

BiopaxFileParser::BiopaxFileParser(IUserNodeData* userNodeData) : _userNodeData(userNodeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
}

struct TempEdge
{
    QStringList _sources;
    QStringList _targets;
    friend bool operator<(const TempEdge& l, const TempEdge& r)
    {
        return std::tie(l._sources, l._targets) < std::tie(r._sources, r._targets);
    }
};

bool BiopaxFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    QFile file(url.toLocalFile());
    if(!file.open(QFile::ReadOnly))
    {
        setFailureReason(QObject::tr("Unable to open file."));
        return false;
    }

    auto fileSize = file.size();
    if(fileSize == 0)
    {
        setFailureReason(QObject::tr("File is empty."));
        return false;
    }

    setProgress(-1);

    QXmlStreamReader xsr(&file);

    std::map<QString, NodeId> nodes;
    std::stack<std::pair<NodeId, QString>> activeNodes;
    std::stack<QString> activeElements;

    std::vector<TempEdge> tempEdges;

    auto processToken = [&](QXmlStreamReader::TokenType tokenType)
    {
        NodeId activeNodeId;
        QString activeNodeName;

        if(!activeNodes.empty())
        {
            activeNodeId = activeNodes.top().first;
            activeNodeName = activeNodes.top().second;
        }

        switch(tokenType)
        {
        case QXmlStreamReader::StartElement:
        {
            const auto& elementName = xsr.name().toString();
            const auto& attributes = xsr.attributes();

            if(isNodeElementName(elementName) && (activeElements.empty() ||
                !isNodeElementName(activeElements.top())))
            {
                if(!attributes.hasAttribute(QStringLiteral("rdf:ID")))
                {
                    setFailureReason(QObject::tr("Node element has no id."));
                    return false;
                }

                auto rdfId = attributes.value(QStringLiteral("rdf:ID")).toString();

                auto nodeId = graphModel->mutableGraph().addNode();
                nodes.emplace(rdfId, nodeId);
                activeNodes.emplace(nodeId, rdfId);

                if(_userNodeData != nullptr)
                {
                    _userNodeData->setValueBy(nodeId, QObject::tr("ID"), rdfId);
                    _userNodeData->setValueBy(nodeId, QObject::tr("Class"), elementName);
                }
            }
            else if(isEdgeElementName(elementName))
            {
                if(!activeNodes.empty())
                {
                    if(!attributes.hasAttribute(QStringLiteral("rdf:resource")))
                    {
                        setFailureReason(QObject::tr("Edge element has no resource."));
                        return false;
                    }

                    auto rdfResource = attributes.value(QStringLiteral("rdf:resource"))
                        .toString().remove(QStringLiteral("#"));

                    auto& tempEdge = tempEdges.emplace_back();

                    tempEdge._sources.push_back(activeNodeName);
                    tempEdge._targets.push_back(rdfResource);

                    if(elementName == QStringLiteral("right") || elementName == QStringLiteral("controlled"))
                    {
                        tempEdge._targets.push_back(rdfResource);
                        tempEdge._sources.push_back(activeNodeName);
                    }
                }
            }

            activeElements.emplace(elementName);

            break;
        }

        case QXmlStreamReader::Characters:
        {
            if(activeNodes.empty())
                break;

            const auto& data = xsr.text().toString();

            if(activeElements.top() == QStringLiteral("displayName"))
            {
                _userNodeData->setValueBy(activeNodeId, QObject::tr("Node Name"), data);
                graphModel->setNodeName(activeNodeId, data);
            }
            else if(activeElements.top() == QStringLiteral("comment"))
                _userNodeData->setValueBy(activeNodeId, QObject::tr("Comment"), data);

            break;
        }

        case QXmlStreamReader::EndElement:
        {
            const auto& elementName = xsr.name().toString();

            if(activeElements.empty())
            {
                setFailureReason(QObject::tr("Orphan end element: %1").arg(elementName));
                return false;
            }

            const auto& top = activeElements.top();
            if(top != xsr.name())
            {
                setFailureReason(QObject::tr("Start and end element mismatch: %1 != %2").arg(top, elementName));
                return false;
            }

            activeElements.pop();

            if(isNodeElementName(elementName))
            {
                // If the node hasn't been assigned a name, just use its ID
                if(graphModel->nodeName(activeNodeId).isEmpty())
                {
                    auto nodeIdString = _userNodeData->valueBy(activeNodeId, QObject::tr("ID")).toString();
                    _userNodeData->setValueBy(activeNodeId, QObject::tr("Node Name"), nodeIdString);
                    graphModel->setNodeName(activeNodeId, nodeIdString);
                }

                activeNodes.pop();
            }

            break;
        }

        case QXmlStreamReader::EndDocument:
        {
            for(const auto& tempEdge : tempEdges)
            {
                for(const auto& sourceNode : tempEdge._sources)
                {
                    auto sourceNodeId = nodes.find(sourceNode);
                    if(sourceNodeId == nodes.end())
                    {
                        setFailureReason(QObject::tr("Invalid Edge Source: %1").arg(sourceNode));
                        return false;
                    }

                    for(const auto& targetNode : tempEdge._targets)
                    {
                        auto targetNodeId = nodes.find(targetNode);
                        if(targetNodeId == nodes.end())
                        {
                            setFailureReason(QObject::tr(
                                "Invalid Edge Target. source node: %1, target node: %2")
                                .arg(sourceNode, targetNode));
                            return false;
                        }

                        graphModel->mutableGraph().addEdge(sourceNodeId->second, targetNodeId->second);
                    }
                }
            }

            break;
        }

        default:
            break;
        }

        return true;
    };

    bool success = true;
    while(!xsr.atEnd() && success)
    {
        auto percent = static_cast<int>((file.pos() * 100) / fileSize);
        setProgress(percent);
        success = processToken(xsr.readNext());

        if(cancelled())
            return false;
    }

    setProgress(-1);

    if(!success)
        return false;

    if(xsr.hasError())
    {
        setFailureReason(QObject::tr("XML parse error: %1").arg(xsr.errorString()));
        return false;
    }

    return true;
}
