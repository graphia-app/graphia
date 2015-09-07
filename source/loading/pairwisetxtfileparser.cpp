#include "pairwisetxtfileparser.h"

#include "../utils/utils.h"
#include "../graph/graph.h"
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
                auto first = list.at(0);
                auto second = list.at(1);

                NodeId firstNodeId;
                NodeId secondNodeId;

                if(!u::contains(nodeIdHash, first))
                {
                    firstNodeId = graph.addNode();
                    nodeIdHash.emplace(first, firstNodeId);
                    _graphModel->nodeNames()[firstNodeId] = first;
                }
                else
                    firstNodeId = nodeIdHash[first];

                if(!u::contains(nodeIdHash, second))
                {
                    secondNodeId = graph.addNode();
                    nodeIdHash.emplace(second, secondNodeId);
                    _graphModel->nodeNames()[secondNodeId] = second;
                }
                else
                    secondNodeId = nodeIdHash[second];

                auto edgeId = graph.addEdge(firstNodeId, secondNodeId);

                if(list.size() >= 3)
                {
                    // We have an edge weight too
                    auto third = list.at(2);
                    _graphModel->edgeWeights()[edgeId] = third.toFloat();
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
