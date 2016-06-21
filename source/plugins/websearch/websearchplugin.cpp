#include "websearchplugin.h"

#include "shared/loading/pairwisetxtfileparser.h"

WebSearchPluginInstance::WebSearchPluginInstance()
{
    connect(this, &WebSearchPluginInstance::graphChanged, this, &WebSearchPluginInstance::onGraphChanged);
    connect(this, &WebSearchPluginInstance::selectionChanged, this, &WebSearchPluginInstance::onSelectionChanged);
}

std::unique_ptr<IParser> WebSearchPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == "PairwiseTXT")
        return std::make_unique<PairwiseTxtFileParser>(this);

    return nullptr;
}

void WebSearchPluginInstance::setNodeName(NodeId nodeId, const QString& name)
{
    graphModel()->setNodeName(nodeId, name);
}

void WebSearchPluginInstance::setEdgeWeight(EdgeId edgeId, float weight)
{
    if(_edgeWeights == nullptr)
    {
        _edgeWeights = std::make_unique<EdgeArray<float>>(graphModel()->mutableGraph());

        graphModel()->dataField(tr("Edge Weight"))
                .setFloatValueFn([this](EdgeId edgeId_) { return _edgeWeights->get(edgeId_); });
    }

    _edgeWeights->set(edgeId, weight);
}

QString WebSearchPluginInstance::selectedNodeNames() const
{
    QString s;

    for(auto nodeId : selectionManager()->selectedNodes())
    {
        if(!s.isEmpty())
            s += ", ";

        s += graphModel()->nodeName(nodeId);
    }

    return s;
}

void WebSearchPluginInstance::onSelectionChanged(const ISelectionManager*)
{
}

void WebSearchPluginInstance::onGraphChanged()
{
    if(_edgeWeights != nullptr)
    {
        float min = *std::min_element(_edgeWeights->begin(), _edgeWeights->end());
        float max = *std::max_element(_edgeWeights->begin(), _edgeWeights->end());

        graphModel()->dataField(tr("Edge Weight")).setFloatMin(min).setFloatMax(max);
    }
}

WebSearchPlugin::WebSearchPlugin()
{
    registerUrlType("PairwiseTXT", QObject::tr("Pairwise Text File"), QObject::tr("Pairwise Text Files"), {"txt"});
}

QStringList WebSearchPlugin::identifyUrl(const QUrl& url) const
{
    return identifyByExtension(url);
}

std::unique_ptr<IPluginInstance> WebSearchPlugin::createInstance()
{
    return std::make_unique<WebSearchPluginInstance>();
}
