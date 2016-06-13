#include "genericplugin.h"

#include "loading/gmlfileparser.h"
#include "loading/pairwisetxtfileparser.h"

GenericPluginInstance::GenericPluginInstance()
{
    connect(this, &GenericPluginInstance::graphChanged, this, &GenericPluginInstance::onGraphChanged);
}

std::unique_ptr<IParser> GenericPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == "GML")
        return std::make_unique<GmlFileParser>();
    else if(urlTypeName == "PairwiseTXT")
        return std::make_unique<PairwiseTxtFileParser>(this);

    return nullptr;
}

void GenericPluginInstance::setNodeName(NodeId nodeId, const QString& name)
{
    graphModel()->setNodeName(nodeId, name);
}

void GenericPluginInstance::setEdgeWeight(EdgeId edgeId, float weight)
{
    if(_edgeWeights == nullptr)
    {
        _edgeWeights = std::make_unique<EdgeArray<float>>(graphModel()->mutableGraph());

        graphModel()->dataField(tr("Edge Weight"))
                .setFloatValueFn([this](EdgeId edgeId_) { return _edgeWeights->get(edgeId_); });
    }

    _edgeWeights->set(edgeId, weight);
}

void GenericPluginInstance::onGraphChanged()
{
    if(_edgeWeights != nullptr)
    {
        float min = *std::min_element(_edgeWeights->begin(), _edgeWeights->end());
        float max = *std::max_element(_edgeWeights->begin(), _edgeWeights->end());

        graphModel()->dataField(tr("Edge Weight")).setFloatMin(min).setFloatMax(max);
    }
}

GenericPlugin::GenericPlugin()
{
    registerUrlType("GML", QObject::tr("GML Files"), {"gml"});
    registerUrlType("PairwiseTXT", QObject::tr("Pairwise Text Files"), {"txt"});
}

QStringList GenericPlugin::identifyUrl(const QUrl& url) const
{
    //FIXME actually look at the file contents
    return identifyByExtension(url);
}

std::unique_ptr<IPluginInstance> GenericPlugin::createInstance()
{
    return std::make_unique<GenericPluginInstance>();
}
