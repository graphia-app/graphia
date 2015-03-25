#include "pairwisetxtfileparser.h"

#include "../graph/graph.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>

#include <QDebug>

bool PairwiseTxtFileParser::parse(Graph& graph)
{
    QFileInfo info(_filename);
    auto fileSize = info.size();

    QFile file(_filename);
    if(file.open(QIODevice::ReadOnly))
    {
        QHash<QString, NodeId> nodeIdHash;

        int percentComplete = 0;
        QTextStream textStream(&file);

        while(!textStream.atEnd())
        {
            QString line = textStream.readLine();
            QRegExp re("(\"[^\"]+\")|([^\\s]+)");
            QStringList list;
            int pos = 0;

            while((pos = re.indexIn(line, pos)) != -1)
            {
                list << re.cap(0);
                pos += re.matchedLength();
            }

            if(list.size() >= 2)
            {
                auto first = list.at(0);
                auto second = list.at(1);

                if(!nodeIdHash.contains(first))
                    nodeIdHash.insert(first, graph.addNode());

                if(!nodeIdHash.contains(second))
                    nodeIdHash.insert(second, graph.addNode());

                graph.addEdge(nodeIdHash.value(first), nodeIdHash.value(second));
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
