#include "pairwisetxtfileparser.h"

#include "shared/utils/utils.h"
#include "shared/graph/imutablegraph.h"
#include "shared/plugins/basegenericplugin.h"
#include "shared/plugins/nodeattributes.h"

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
                                             NodeAttributes* nodeAttributes) :
    _genericPluginInstance(genericPluginInstance), _nodeAttributes(nodeAttributes)
{
    if(_nodeAttributes != nullptr)
        _nodeAttributes->add(QObject::tr("Node Name"));
}

bool PairwiseTxtFileParser::parse(const QUrl& url, IMutableGraph& graph, const IParser::ProgressFn& progress)
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

    file.seekg(0, std::ios::beg);
    while(std::getline(file, line))
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
                bool space = codePoint < 0xFF && std::isspace(codePoint);
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
            if(tokens.size() >= 4 && _nodeAttributes != nullptr)
            {
                auto& firstToken = tokens.at(0);

                if(firstToken.compare("NODECLASS") == 0)
                {
                    auto& secondToken = tokens.at(1);
                    auto& thirdToken = tokens.at(2);
                    auto& fourthToken = tokens.at(3);

                    if(u::contains(nodeIdMap, secondToken))
                    {
                        QString nodeClass(QString::fromStdString(fourthToken));
                        QString value(QString::fromStdString(thirdToken));

                        auto nodeId = nodeIdMap.at(secondToken);

                        _nodeAttributes->add(nodeClass);
                        _nodeAttributes->setValueByNodeId(nodeId, nodeClass, value);
                    }
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

                if(_nodeAttributes != nullptr)
                {
                    _nodeAttributes->addNodeId(firstNodeId);
                    _nodeAttributes->setValueByNodeId(firstNodeId, QObject::tr("Node Name"),
                                                      QString::fromStdString(firstToken));
                }
            }
            else
                firstNodeId = nodeIdMap[firstToken];

            if(!u::contains(nodeIdMap, secondToken))
            {
                secondNodeId = graph.addNode();
                nodeIdMap.emplace(secondToken, secondNodeId);

                if(_nodeAttributes != nullptr)
                {
                    _nodeAttributes->addNodeId(secondNodeId);
                    _nodeAttributes->setValueByNodeId(secondNodeId, QObject::tr("Node Name"),
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
                _genericPluginInstance->setEdgeWeight(edgeId, std::atof(thirdToken.c_str()));
            }
        }

        progress(static_cast<int>(file.tellg() * 100 / fileSize));
    }

    return true;
}
