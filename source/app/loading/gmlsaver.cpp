#include "gmlsaver.h"

#include "shared/graph/imutablegraph.h"
#include "ui/document.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

static QString escape(const QString& string)
{
    return string.toHtmlEscaped();
}

bool GMLSaver::save()
{
    QFile file(_url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    int level = 0;

    size_t numElements = _graphModel->attributeNames().size() +
                       static_cast<size_t>(_graphModel->graph().numNodes()) +
                       static_cast<size_t>(_graphModel->graph().numEdges());
    size_t runningCount = 0;

    std::map<QString, QString> alphanumAttributeNames;
    _graphModel->mutableGraph().setPhase(QObject::tr("Attributes"));
    for(const auto& nodeAttributeName : _graphModel->attributeNames())
    {
        auto cleanName = nodeAttributeName;
        cleanName.remove(QRegularExpression(QStringLiteral("[^a-zA-Z\\d]")));
        if(cleanName.isEmpty())
            cleanName = QStringLiteral("Attribute");

        // Duplicate attributenames can occur when removing non alphanum chars, append a number.
        if(alphanumAttributeNames.find(cleanName) != alphanumAttributeNames.end())
        {
            int suffix = 1;
            while(true)
            {
                auto uniqueCleanName = cleanName;
                uniqueCleanName.append(QString::number(suffix++));
                if(alphanumAttributeNames.find(uniqueCleanName) == alphanumAttributeNames.end())
                {
                    cleanName = uniqueCleanName;
                    break;
                }
            }
        }

        alphanumAttributeNames[nodeAttributeName] = cleanName;

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    QTextStream stream(&file);
    stream << "graph" << endl << "[" << endl;
    level++;

    _graphModel->mutableGraph().setPhase(QObject::tr("Nodes"));
    for(auto nodeId : _graphModel->graph().nodeIds())
    {
        QString nodeName = escape(_graphModel->nodeName(nodeId));
        stream << indent(level) << "node" << endl;
        stream << indent(level) << "[" << endl;
        level++;
        stream << indent(level) << "id " << static_cast<int>(nodeId) << endl;
        QString labelString = QStringLiteral("\"%1\"").arg(nodeName);
        stream << indent(level) << "label " << labelString << endl;

        // Attributes
        for(const auto& nodeAttributeName : _graphModel->attributeNames(ElementType::Node))
        {
            const auto& attribute = _graphModel->attributeByName(nodeAttributeName);
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
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    _graphModel->mutableGraph().setPhase(QObject::tr("Edges"));
    for(auto edgeId : _graphModel->graph().edgeIds())
    {
        auto& edge = _graphModel->graph().edgeById(edgeId);

        stream << indent(level) << "edge" << endl;
        stream << indent(level) << "[" << endl;
        level++;
        stream << indent(level) << "source " << static_cast<int>(edge.sourceId()) << endl;
        stream << indent(level) << "target " << static_cast<int>(edge.targetId()) << endl;

        // Attributes
        for(const auto& edgeAttributeName : _graphModel->attributeNames(ElementType::Edge))
        {
            const auto& attribute = _graphModel->attributeByName(edgeAttributeName);
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
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    stream << indent(--level) << "]" << endl; // graph

    return true;
}
