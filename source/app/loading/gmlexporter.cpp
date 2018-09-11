#include "gmlexporter.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

bool GMLExporter::save(const QUrl& url, IGraphModel* graphModel)
{
    QFile file(url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    int level = 0;

    size_t fileCount = graphModel->attributeNames().size() +
                       static_cast<size_t>(graphModel->graph().numNodes()) +
                       static_cast<size_t>(graphModel->graph().numEdges());
    size_t runningCount = 0;

    std::map<QString, QString> alphanumAttributeNames;
    for(const auto& nodeAttributeName : graphModel->attributeNames())
    {
        auto cleanName = nodeAttributeName;
        cleanName.remove(QRegularExpression("[^a-zA-Z\\d]"));
        if(cleanName.isEmpty())
            cleanName = "Attribute";

        int suffix = 0;
        // Duplicate attributenames can occur when removing non alphanum chars, append a number.
        if(alphanumAttributeNames.find(cleanName) != alphanumAttributeNames.end())
        {
            do
            {
                auto uniqueCleanName = cleanName;
                uniqueCleanName.append(QString::number(suffix++));
                if(alphanumAttributeNames.find(uniqueCleanName) == alphanumAttributeNames.end())
                {
                    cleanName = uniqueCleanName;
                    break;
                }
            } while(true);
        }

        alphanumAttributeNames[nodeAttributeName] = cleanName;

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / fileCount));
    }

    QTextStream stream(&file);
    stream << "graph" << endl << "[" << endl;
    level++;

    for(auto nodeId : graphModel->graph().nodeIds())
    {
        stream << indent(level) << "node" << endl;
        stream << indent(level) << "[" << endl;
        level++;
        stream << indent(level) << "id " << static_cast<int>(nodeId) << endl;
        stream << indent(level) << "label " << QStringLiteral("\"%1\"").arg(graphModel->nodeName(nodeId))
               << endl;

        for(const auto& nodeAttributeName : graphModel->attributeNames(ElementType::Node))
        {
            const auto& attribute = graphModel->attributeByName(nodeAttributeName);
            if(attribute->valueType() == ValueType::String)
            {
                stream << indent(level) << alphanumAttributeNames[nodeAttributeName] << " "
                       << QStringLiteral("\"%1\"").arg(attribute->stringValueOf(nodeId)) << endl;
            }
            else
            {
                stream << indent(level) << alphanumAttributeNames[nodeAttributeName] << " "
                       << attribute->stringValueOf(nodeId) << endl;
            }
        }
        stream << indent(--level) << "]" << endl; // node

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / fileCount));
    }

    for(auto edgeId : graphModel->graph().edgeIds())
    {
        auto& edge = graphModel->graph().edgeById(edgeId);

        stream << indent(level) << "edge" << endl;
        stream << indent(level) << "[" << endl;
        level++;
        stream << indent(level) << "source " << static_cast<int>(edge.sourceId()) << endl;
        stream << indent(level) << "target " << static_cast<int>(edge.targetId()) << endl;

        for(const auto& edgeAttributeName : graphModel->attributeNames(ElementType::Edge))
        {
            const auto& attribute = graphModel->attributeByName(edgeAttributeName);
            if(attribute->valueType() == ValueType::String)
            {
                stream << indent(level) << alphanumAttributeNames[edgeAttributeName] << " "
                       << QStringLiteral("\"%1\"").arg(attribute->stringValueOf(edgeId)) << endl;
            }
            else
            {
                stream << indent(level) << alphanumAttributeNames[edgeAttributeName] << " "
                       << attribute->stringValueOf(edgeId) << endl;
            }
        }
        stream << indent(--level) << "]" << endl; // edge

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / fileCount));
    }

    stream << indent(--level) << "]" << endl; // graph

    return true;
}
