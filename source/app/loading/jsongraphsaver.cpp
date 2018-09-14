#include "jsongraphsaver.h"

#include "json_helper.h"
#include "nativesaver.h"
#include "shared/attributes/iattribute.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "ui/document.h"

#include <QDebug>
#include <QFile>

bool JSONGraphSaver::save()
{
    json fileObject;
    fileObject["graph"] = NativeSaver::graphAsJson(_graphModel->graph(), *this);
    setProgress(-1);

    int fileSize = _graphModel->graph().numNodes() + _graphModel->graph().numEdges();
    int runningCount = 0;

    // Node Attributes
    _graphModel->mutableGraph().setPhase(QObject::tr("Node Attributes"));
    for(auto& node : fileObject["graph"]["nodes"])
    {
        NodeId nodeId = std::stoi(node["id"].get<std::string>());
        for(const auto& nodeAttributeName : _graphModel->attributeNames(ElementType::Node))
        {
            const auto& attribute = _graphModel->attributeByName(nodeAttributeName);

            auto byteArray = nodeAttributeName.toUtf8();
            auto name = byteArray.constData();

            if(attribute->valueType() == ValueType::String)
                node["metadata"][name] = attribute->stringValueOf(nodeId);
            else if(attribute->valueType() == ValueType::Int)
                node["metadata"][name] = attribute->intValueOf(nodeId);
            else if(attribute->valueType() == ValueType::Float)
                node["metadata"][name] = attribute->floatValueOf(nodeId);
        }

        runningCount++;
        setProgress(runningCount * 100 / fileSize);
    }

    // Edge Attributes
    _graphModel->mutableGraph().setPhase(QObject::tr("Edge Attributes"));
    for(auto& edge : fileObject["graph"]["edges"])
    {
        EdgeId edgeId = std::stoi(edge["id"].get<std::string>());
        for(const auto& edgeAttributeName : _graphModel->attributeNames(ElementType::Edge))
        {
            const auto& attribute = _graphModel->attributeByName(edgeAttributeName);

            auto byteArray = edgeAttributeName.toUtf8();
            auto name = byteArray.constData();

            if(attribute->valueType() == ValueType::String)
                edge["metadata"][name] = attribute->stringValueOf(edgeId);
            else if(attribute->valueType() == ValueType::Int)
                edge["metadata"][name] = attribute->intValueOf(edgeId);
            else if(attribute->valueType() == ValueType::Float)
                edge["metadata"][name] = attribute->floatValueOf(edgeId);
        }

        runningCount++;
        setProgress(runningCount * 100 / fileSize);
    }

    QFile file(_url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    file.write(QByteArray::fromStdString(fileObject.dump()));
    file.close();
    return true;
}
