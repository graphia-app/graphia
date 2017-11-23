#include "pairwisetxtfileparser.h"

#include "shared/utils/container.h"
#include "shared/utils/string.h"
#include "shared/graph/imutablegraph.h"
#include "shared/plugins/basegenericplugin.h"
#include "shared/plugins/usernodedata.h"

#include "thirdparty/utfcpp/utf8.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>
#include <QUrl>
#include <QDebug>

#include <unordered_map>
#include <vector>

#include <string>
#include <iostream>
#include <fstream>
#include <cctype>

PairwiseTxtFileParser::PairwiseTxtFileParser(BaseGenericPluginInstance* genericPluginInstance,
                                             UserNodeData* userNodeData) :
    _genericPluginInstance(genericPluginInstance), _userNodeData(userNodeData)
{
    if(_userNodeData != nullptr)
        _userNodeData->add(QObject::tr("Node Name"));
}

bool PairwiseTxtFileParser::parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progressFn)
{
    std::ifstream file(url.toLocalFile().toStdString());
    if(!file)
        return false;

    auto fileSize = file.tellg();
    file.seekg(0, std::ios::end);
    fileSize = file.tellg() - fileSize;

    std::unordered_map<std::string, NodeId> nodeIdMap;

    std::string line;
    std::string token;
    std::vector<std::string> tokens;

    progressFn(-1);

    file.seekg(0, std::ios::beg);
    while(u::getline(file, line))
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

            if(codePoint == '\"')
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
                bool space = (codePoint < 0xFF) && (std::isspace(codePoint) != 0);
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
                {
                    _userNodeData->add(attributeName);
                    _userNodeData->setValueByNodeId(nodeId, attributeName, value);
                }
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
                firstNodeId = graph.addNode();
                nodeIdMap.emplace(firstToken, firstNodeId);

                if(_userNodeData != nullptr)
                {
                    _userNodeData->addNodeId(firstNodeId);
                    _userNodeData->setValueByNodeId(firstNodeId, QObject::tr("Node Name"),
                                                      QString::fromStdString(firstToken));
                }
            }
            else
                firstNodeId = nodeIdMap[firstToken];

            if(!u::contains(nodeIdMap, secondToken))
            {
                secondNodeId = graph.addNode();
                nodeIdMap.emplace(secondToken, secondNodeId);

                if(_userNodeData != nullptr)
                {
                    _userNodeData->addNodeId(secondNodeId);
                    _userNodeData->setValueByNodeId(secondNodeId, QObject::tr("Node Name"),
                                                      QString::fromStdString(secondToken));
                }
            }
            else
                secondNodeId = nodeIdMap[secondToken];

            auto edgeId = graph.addEdge(firstNodeId, secondNodeId);

            if(tokens.size() >= 3)
            {
                // We have an edge weight too
                auto& thirdToken = tokens.at(2);

                if(u::isNumeric(thirdToken))
                {
                    float edgeWeight = std::stof(thirdToken);

                    if(std::isnan(edgeWeight) || !std::isfinite(edgeWeight))
                        edgeWeight = 1.0f;

                    _genericPluginInstance->setEdgeWeight(edgeId, edgeWeight);
                }
            }
        }

        auto filePosition = file.tellg();
        if(filePosition >= 0)
            progressFn(static_cast<int>(filePosition * 100 / fileSize));
    }

    return true;
}
