#include "pairwisetxtfileparser.h"

#include "shared/utils/utils.h"
#include "shared/graph/imutablegraph.h"
#include "shared/plugins/basegenericplugin.h"

#include "thirdparty/utf8/utf8.h"

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

PairwiseTxtFileParser::PairwiseTxtFileParser(BaseGenericPluginInstance* genericPluginInstance) :
    _genericPluginInstance(genericPluginInstance)
{}

bool PairwiseTxtFileParser::parse(const QUrl& url, IMutableGraph& graph, const IParser::ProgressFn& progress)
{
    std::ifstream file(url.toLocalFile().toStdString());
    if(!file)
        return false;

    auto fileSize = file.tellg();
    file.seekg(0, std::ios::end);
    fileSize = file.tellg() - fileSize;

    std::unordered_map<std::string, NodeId> nodeIdHash;

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

        auto it = line.begin();
        auto end = line.end();
        do
        {
            uint32_t codePoint = utf8::next(it, end);

            if((it + 1) < end &&
               codePoint == '/' && utf8::peek_next(it, end) == '/')
            {
                // Ignore C++ style comments
                break;
            }
            else if(codePoint == '\"')
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
                bool space = std::isspace(codePoint);
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
        while(it < end);

        if(!token.empty())
        {
            tokens.emplace_back(std::move(token));
            token.clear();
        }

        if(tokens.size() >= 2)
        {
            auto& firstToken = tokens.at(0);
            auto& secondToken = tokens.at(1);

            NodeId firstNodeId;
            NodeId secondNodeId;

            if(!u::contains(nodeIdHash, firstToken))
            {
                firstNodeId = graph.addNode();
                nodeIdHash.emplace(firstToken, firstNodeId);
                _genericPluginInstance->setNodeName(firstNodeId, QString::fromStdString(firstToken));
            }
            else
                firstNodeId = nodeIdHash[firstToken];

            if(!u::contains(nodeIdHash, secondToken))
            {
                secondNodeId = graph.addNode();
                nodeIdHash.emplace(secondToken, secondNodeId);
                _genericPluginInstance->setNodeName(secondNodeId, QString::fromStdString(secondToken));
            }
            else
                secondNodeId = nodeIdHash[secondToken];

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
