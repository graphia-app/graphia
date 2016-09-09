#include "basegenericplugin.h"

#include "shared/loading/gmlfileparser.h"
#include "shared/loading/pairwisetxtfileparser.h"
#include "shared/loading/graphmlparser.h"

BaseGenericPluginInstance::BaseGenericPluginInstance()
{
    connect(this, &BaseGenericPluginInstance::graphChanged,
            this, &BaseGenericPluginInstance::onGraphChanged);
    connect(this, &BaseGenericPluginInstance::selectionChanged,
            this, &BaseGenericPluginInstance::onSelectionChanged);
}

std::unique_ptr<IParser> BaseGenericPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == "GML")
        return std::make_unique<GmlFileParser>(this);
    else if(urlTypeName == "PairwiseTXT")
        return std::make_unique<PairwiseTxtFileParser>(this);
    else if(urlTypeName == "GraphML")
        return std::make_unique<GraphMLParser>(this);

    return nullptr;
}

void BaseGenericPluginInstance::setNodeName(NodeId nodeId, const QString& name)
{
    graphModel()->setNodeName(nodeId, name);
}

void BaseGenericPluginInstance::setEdgeWeight(EdgeId edgeId, float weight)
{
    if(_edgeWeights == nullptr)
    {
        _edgeWeights = std::make_unique<EdgeArray<float>>(graphModel()->mutableGraph());

        graphModel()->dataField(tr("Edge Weight"))
                .setFloatValueFn([this](EdgeId edgeId_) { return _edgeWeights->get(edgeId_); });
    }

    _edgeWeights->set(edgeId, weight);
}

QString BaseGenericPluginInstance::selectedNodeNames() const
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

float BaseGenericPluginInstance::selectedNodeMeanDegree() const
{
    if(selectionManager()->selectedNodes().empty())
        return 0.0f;

    float d = 0.0f;

    for(auto nodeId : selectionManager()->selectedNodes())
        d += static_cast<float>(graphModel()->graph().nodeById(nodeId).degree());

    return d / selectionManager()->selectedNodes().size();
}

void BaseGenericPluginInstance::onGraphChanged()
{
    if(_edgeWeights != nullptr)
    {
        float min = *std::min_element(_edgeWeights->begin(), _edgeWeights->end());
        float max = *std::max_element(_edgeWeights->begin(), _edgeWeights->end());

        graphModel()->dataField(tr("Edge Weight")).setFloatMin(min).setFloatMax(max);
    }
}

void BaseGenericPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    emit selectedNodeNamesChanged();
    emit selectedNodeMeanDegreeChanged();
}

BaseGenericPlugin::BaseGenericPlugin()
{
    registerUrlType("GML", QObject::tr("GML File"), QObject::tr("GML Files"), {"gml"});
    registerUrlType("PairwiseTXT", QObject::tr("Pairwise Text File"), QObject::tr("Pairwise Text Files"), {"txt", "layout"});
    registerUrlType("GraphML", QObject::tr("GraphML File"), QObject::tr("GraphML Files"), {"graphml"});

}

QStringList BaseGenericPlugin::identifyUrl(const QUrl& url) const
{
    //FIXME actually look at the file contents
    return identifyByExtension(url);
}
