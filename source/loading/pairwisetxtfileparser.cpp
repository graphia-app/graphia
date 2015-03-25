#include "pairwisetxtfileparser.h"

#include "../graph/graph.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>

#include <QDebug>

#include <map>
#include <vector>

bool PairwiseTxtFileParser::parse(Graph& graph)
{
    QFileInfo info(_filename);
    auto fileSize = info.size();

    QFile file(_filename);
    if(file.open(QIODevice::ReadOnly))
    {
        std::map<QString, NodeId> nodeIdHash;
        std::vector<std::pair<NodeId, NodeId>> edges; //HACK

        int percentComplete = 0;
        QTextStream textStream(&file);

        while(!textStream.atEnd())
        {
            QString line = textStream.readLine();
            QRegExp re("\"([^\"]+)\"|([^\\s]+)");
            QStringList list;
            int pos = 0;

            while((pos = re.indexIn(line, pos)) != -1)
            {
                list << (!re.cap(1).isEmpty() ? re.cap(1) : re.cap(2));
                pos += re.matchedLength();
            }

            if(list.size() >= 2)
            {
                auto first = list.at(0);
                auto second = list.at(1);

                NodeId firstNodeId;
                NodeId secondNodeId;

                if(nodeIdHash.find(first) == nodeIdHash.end())
                {
                    firstNodeId = graph.addNode();
                    nodeIdHash.emplace(first, firstNodeId);
                }
                else
                    firstNodeId = nodeIdHash[first];

                if(nodeIdHash.find(second) == nodeIdHash.end())
                {
                    secondNodeId = graph.addNode();
                    nodeIdHash.emplace(second, secondNodeId);
                }
                else
                    secondNodeId = nodeIdHash[second];

                edges.emplace_back(firstNodeId, secondNodeId); //HACK
            }

            int newPercentComplete = static_cast<int>(file.pos() * 100 / fileSize);

            if(newPercentComplete > percentComplete)
            {
                percentComplete = newPercentComplete;
                emit progress(newPercentComplete);
            }
        }

        //HACK
        for(auto edge : edges)
            graph.addEdge(edge.first, edge.second);

        file.close();
    }

    return true;
}
