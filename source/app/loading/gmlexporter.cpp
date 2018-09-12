#include "gmlexporter.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

static QString escape(QString unescaped)
{
    QString escaped = unescaped;
    escaped.replace("\"", "\\\"");
    return escaped;
}

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
        QString nodeName = escape(graphModel->nodeName(nodeId));
        stream << indent(level) << "node" << endl;
        stream << indent(level) << "[" << endl;
        level++;
        stream << indent(level) << "id " << static_cast<int>(nodeId) << endl;
        QString labelString = QStringLiteral("\"%1\"").arg(nodeName);
        stream << indent(level) << "label " << labelString << endl;

        // Attributes
        for(const auto& nodeAttributeName : graphModel->attributeNames(ElementType::Node))
        {
            const auto& attribute = graphModel->attributeByName(nodeAttributeName);
            QString escapedValue = escape(attribute->stringValueOf(nodeId));
            if(attribute->valueType() == ValueType::String)
            {
                stream << indent(level) << alphanumAttributeNames[nodeAttributeName] << " "
                       << QStringLiteral("\"%1\"").arg(escapedValue) << endl;
            }
            else
            {
                stream << indent(level) << alphanumAttributeNames[nodeAttributeName] << " "
                       << escapedValue << endl;
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

        // Attributes
        for(const auto& edgeAttributeName : graphModel->attributeNames(ElementType::Edge))
        {
            const auto& attribute = graphModel->attributeByName(edgeAttributeName);
            auto escapedValue = escape(attribute->stringValueOf(edgeId));
            if(attribute->valueType() == ValueType::String)
            {
                stream << indent(level) << alphanumAttributeNames[edgeAttributeName] << " "
                       << QStringLiteral("\"%1\"").arg(escapedValue) << endl;
            }
            else
            {
                stream << indent(level) << alphanumAttributeNames[edgeAttributeName] << " "
                       << escapedValue << endl;
            }
        }
        stream << indent(--level) << "]" << endl; // edge

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / fileCount));
    }

    stream << indent(--level) << "]" << endl; // graph

    return true;
}
