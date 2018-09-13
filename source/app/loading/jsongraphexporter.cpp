#include "jsongraphexporter.h"

#include "saver.h"
#include "json_helper.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/attributes/iattribute.h"

#include <QFile>
#include <QDebug>

bool JSONGraphExporter::save(const QUrl &url, IGraphModel *graphModel)
{
    json fileObject;
    fileObject["graph"] = Saver::graphAsJson(graphModel->graph(), *this);
    setProgress(-1);

    int fileSize = graphModel->graph().numNodes() + graphModel->graph().numEdges();
    int runningCount = 0;

    // Node Attributes
    graphModel->mutableGraph().setPhase(QObject::tr("Node Attributes"));
    for(auto& node : fileObject["graph"]["nodes"])
    {
        NodeId nodeId = std::stoi(node["id"].get<std::string>());
        for(const auto& nodeAttributeName : graphModel->attributeNames(ElementType::Node))
        {
            const auto& attribute = graphModel->attributeByName(nodeAttributeName);

            auto byteArray = nodeAttributeName.toUtf8();
            auto name = byteArray.constData();

            if(attribute->valueType() == ValueType::String)
                node["metadata"][name] = attribute->stringValueOf(nodeId);
            else if (attribute->valueType() == ValueType::Int)
                node["metadata"][name] = attribute->intValueOf(nodeId);
            else if (attribute->valueType() == ValueType::Float)
                node["metadata"][name] = attribute->floatValueOf(nodeId);
        }

        runningCount++;
        setProgress(runningCount * 100 / fileSize);
    }

    // Edge Attributes
    graphModel->mutableGraph().setPhase(QObject::tr("Edge Attributes"));
    for(auto& edge : fileObject["graph"]["edges"])
    {
        EdgeId edgeId = std::stoi(edge["id"].get<std::string>());
        for(const auto& edgeAttributeName : graphModel->attributeNames(ElementType::Edge))
        {
            const auto& attribute = graphModel->attributeByName(edgeAttributeName);

            auto byteArray = edgeAttributeName.toUtf8();
            auto name = byteArray.constData();

            if(attribute->valueType() == ValueType::String)
                edge["metadata"][name] = attribute->stringValueOf(edgeId);
            else if (attribute->valueType() == ValueType::Int)
                edge["metadata"][name] = attribute->intValueOf(edgeId);
            else if (attribute->valueType() == ValueType::Float)
                edge["metadata"][name] = attribute->floatValueOf(edgeId);
        }

        runningCount++;
        setProgress(runningCount * 100 / fileSize);
    }

    QFile file(url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    file.write(QByteArray::fromStdString(fileObject.dump()));
    file.close();
    return true;
}

QString JSONGraphExporter::name() const
{
    return QStringLiteral("JSON Graph");
}

QString JSONGraphExporter::extension() const
{
    return QStringLiteral(".json");
}
