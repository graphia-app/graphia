/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "pairwisetxtfileparser.h"

#include "shared/utils/container.h"
#include "shared/utils/string.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/loading/userelementdata.h"

#include <utfcpp/utf8.h>

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QUrl>
#include <QDebug>

#include <unordered_map>
#include <vector>

#include <string>
#include <iostream>
#include <fstream>
#include <cctype>

PairwiseTxtFileParser::PairwiseTxtFileParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData) :
    _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
}

bool PairwiseTxtFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    Q_ASSERT(graphModel != nullptr);

    std::ifstream file(url.toLocalFile().toStdString());
    if(file.fail() || graphModel == nullptr)
        return false;

    auto fileSize = file.tellg();
    file.seekg(0, std::ios::end);
    fileSize = file.tellg() - fileSize;

    if(fileSize == 0)
    {
        setFailureReason(QObject::tr("File is empty."));
        return false;
    }

    std::unordered_map<std::string, NodeId> nodeIdMap;

    std::string line;
    std::string token;
    std::vector<std::string> tokens;

    setProgress(-1);

    file.seekg(0, std::ios::beg);
    while(!u::getline(file, line).eof())
    {
        if(cancelled())
            return false;

        tokens.clear();

        bool inQuotes = false;
        bool isComment = false;

        std::string validatedLine;
        utf8::replace_invalid(line.begin(), line.end(), std::back_inserter(validatedLine));
        auto it = validatedLine.begin();
        auto end = validatedLine.end();
        while(it < end)
        {
            uint32_t codePoint = utf8::next(it, end);

            if(it < end && it < (end - 1) &&
               codePoint == '/' && utf8::peek_next(it, end) == '/')
            {
                isComment = true;

                // Skip the second /
                utf8::advance(it, 1, end);
                codePoint = utf8::next(it, end);
            }

            if(codePoint == '"')
            {
                if(inQuotes)
                {
                    tokens.emplace_back(std::move(token));
                    token.clear();
                }

                inQuotes = !inQuotes;
            }
            else
            {
                bool space = (codePoint < 0xFF) && (std::isspace(static_cast<int>(codePoint)) != 0);
                bool trailingSpace = space && !token.empty();

                if(trailingSpace && !inQuotes)
                {
                    tokens.emplace_back(std::move(token));
                    token.clear();
                }
                else if(!space || inQuotes)
                    utf8::unchecked::append(codePoint, std::back_inserter(token));
            }
        }

        if(!token.empty())
        {
            tokens.emplace_back(std::move(token));
            token.clear();
        }

        if(isComment)
        {
            if(!tokens.empty() && _userNodeData != nullptr)
            {
                NodeId nodeId;
                QString attributeName;
                QString value;

                const std::string NODE("NODE");
                auto& firstToken = tokens.at(0);

                if(firstToken.compare(0, NODE.length(), NODE) == 0)
                {
                    auto& secondToken = tokens.at(1);
                    if(u::contains(nodeIdMap, secondToken))
                        nodeId = nodeIdMap.at(secondToken);

                    std::string property = firstToken.substr(NODE.length(), std::string::npos);

                    if(tokens.size() == 4 && property == "CLASS")
                    {
                        attributeName = QString::fromStdString(tokens.at(3));
                        value = QString::fromStdString(tokens.at(2));
                    }
                    else if(tokens.size() == 3 && property == "SIZE")
                    {
                        attributeName = QObject::tr("BioLayout Node Size");
                        value = QString::fromStdString(tokens.at(2));
                    }
                    else if(tokens.size() == 4 && property == "SHAPE")
                    {
                        attributeName = QObject::tr("BioLayout Node Shape");
                        value = QString::fromStdString(tokens.at(3));
                    }
                    else if(tokens.size() == 3 && property == "ALPHA")
                    {
                        attributeName = QObject::tr("BioLayout Node Opacity");
                        value = QString::fromStdString(tokens.at(2));
                    }
                    else if(tokens.size() == 3 && property == "COLOR")
                    {
                        attributeName = QObject::tr("BioLayout Node Colour");
                        value = QString::fromStdString(tokens.at(2));
                    }
                    else if(tokens.size() == 3 && property == "DESC")
                    {
                        attributeName = QObject::tr("BioLayout Node Description");
                        value = QString::fromStdString(tokens.at(2));
                    }
                    else if(tokens.size() == 3 && property == "URL")
                    {
                        attributeName = QObject::tr("BioLayout Node URL");
                        value = QString::fromStdString(tokens.at(2));
                    }
                }

                if(!nodeId.isNull())
                    _userNodeData->setValueBy(nodeId, attributeName, value);
            }
        }
        else if(tokens.size() >= 2)
        {
            auto& firstToken = tokens.at(0);
            auto& secondToken = tokens.at(1);

            NodeId firstNodeId;
            NodeId secondNodeId;

            if(!u::contains(nodeIdMap, firstToken))
            {
                firstNodeId = graphModel->mutableGraph().addNode();
                nodeIdMap.emplace(firstToken, firstNodeId);

                if(_userNodeData != nullptr)
                {
                    auto nodeName = QString::fromStdString(firstToken);
                    _userNodeData->setValueBy(firstNodeId, QObject::tr("Node Name"), nodeName);
                    graphModel->setNodeName(firstNodeId, nodeName);
                }
            }
            else
                firstNodeId = nodeIdMap[firstToken];

            if(!u::contains(nodeIdMap, secondToken))
            {
                secondNodeId = graphModel->mutableGraph().addNode();
                nodeIdMap.emplace(secondToken, secondNodeId);

                if(_userNodeData != nullptr)
                {
                    auto nodeName = QString::fromStdString(secondToken);
                    _userNodeData->setValueBy(secondNodeId, QObject::tr("Node Name"), nodeName);
                    graphModel->setNodeName(secondNodeId, nodeName);
                }
            }
            else
                secondNodeId = nodeIdMap[secondToken];

            auto edgeId = graphModel->mutableGraph().addEdge(firstNodeId, secondNodeId);

            if(tokens.size() >= 3)
            {
                // We have an edge weight too
                auto& thirdToken = tokens.at(2);

                if(u::isNumeric(thirdToken))
                {
                    auto edgeWeight = u::toNumber(thirdToken);

                    if(std::isnan(edgeWeight) || !std::isfinite(edgeWeight))
                        edgeWeight = 1.0;

                    _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"), QString::number(edgeWeight));
                }
            }
        }

        auto filePosition = file.tellg();
        if(filePosition >= 0)
            setProgress(static_cast<int>(filePosition * 100 / fileSize));
    }

    return true;
}
