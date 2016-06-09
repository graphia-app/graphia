#include "genericplugin.h"

GenericPlugin::GenericPlugin() :
    _gmlFileParser(),
    _pairwiseTxtFileParser(this)
{
    registerUrlType("GML", QObject::tr("GML Files"), {"gml"});
    registerUrlType("PairwiseTXT", QObject::tr("Pairwise Text Files"), {"txt"});

    connect(this, &GenericPlugin::graphChanged, this, &GenericPlugin::onGraphChanged);
}

QStringList GenericPlugin::identifyUrl(const QUrl& url) const
{
    //FIXME actually look at the file contents
    return identifyByExtension(url);
}

IParser* GenericPlugin::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == "GML")
        return &_gmlFileParser;
    else if(urlTypeName == "PairwiseTXT")
        return &_pairwiseTxtFileParser;

    return nullptr;
}

void GenericPlugin::setNodeName(NodeId nodeId, const QString& name)
{
    graphModel()->setNodeName(nodeId, name);
}

void GenericPlugin::setEdgeWeight(EdgeId edgeId, float weight)
{
    if(_edgeWeights == nullptr)
    {
        _edgeWeights = std::make_unique<EdgeArray<float>>(graphModel()->mutableGraph());

        graphModel()->dataField(tr("Edge Weight"))
                .setFloatValueFn([this](EdgeId edgeId_) { return _edgeWeights->get(edgeId_); });
    }

    _edgeWeights->set(edgeId, weight);
}

void GenericPlugin::onGraphChanged()
{
    if(_edgeWeights != nullptr)
    {
        float min = *std::min_element(_edgeWeights->begin(), _edgeWeights->end());
        float max = *std::max_element(_edgeWeights->begin(), _edgeWeights->end());

        graphModel()->dataField(tr("Edge Weight")).setFloatMin(min).setFloatMax(max);
    }
}
