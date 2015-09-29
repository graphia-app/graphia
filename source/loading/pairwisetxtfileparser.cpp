#include "pairwisetxtfileparser.h"

#include "../utils/utils.h"
#include "../graph/mutablegraph.h"
#include "../graph/weightededgegraphmodel.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>

#include <QDebug>

#include <map>
#include <vector>

bool PairwiseTxtFileParser::parse(MutableGraph& graph)
{
    QFileInfo info(_filename);

    if(!info.exists())
        return false;

    auto fileSize = info.size();

    QFile file(_filename);
    if(file.open(QIODevice::ReadOnly))
    {
        std::map<QString, NodeId> nodeIdHash;

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
                auto firstToken = list.at(0);
                auto secondToken = list.at(1);

                NodeId firstNodeId;
                NodeId secondNodeId;

                if(!u::contains(nodeIdHash, firstToken))
                {
                    firstNodeId = graph.addNode();
                    nodeIdHash.emplace(firstToken, firstNodeId);
                    _graphModel->setNodeName(firstNodeId, firstToken);
                }
                else
                    firstNodeId = nodeIdHash[firstToken];

                if(!u::contains(nodeIdHash, secondToken))
                {
                    secondNodeId = graph.addNode();
                    nodeIdHash.emplace(secondToken, secondNodeId);
                    _graphModel->setNodeName(secondNodeId, secondToken);
                }
                else
                    secondNodeId = nodeIdHash[secondToken];

                auto edgeId = graph.addEdge(firstNodeId, secondNodeId);

                if(list.size() >= 3)
                {
                    // We have an edge weight too
                    auto thirdToken = list.at(2);
                    _graphModel->setEdgeWeight(edgeId, thirdToken.toFloat());
                }
            }

            int newPercentComplete = static_cast<int>(file.pos() * 100 / fileSize);

            if(newPercentComplete > percentComplete)
            {
                percentComplete = newPercentComplete;
                emit progress(newPercentComplete);
            }
        }

        file.close();
    }

    return true;
}
