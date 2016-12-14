#include "basegenericplugin.h"

#include "shared/loading/gmlfileparser.h"
#include "shared/loading/pairwisetxtfileparser.h"
#include "shared/loading/graphmlparser.h"

BaseGenericPluginInstance::BaseGenericPluginInstance() :
    _nodeAttributesTableModel(&_nodeAttributes)
{
    connect(this, SIGNAL(loadComplete()), this, SLOT(onLoadComplete()));
    connect(this, SIGNAL(graphChanged()), this, SLOT(onGraphChanged()));
    connect(this, SIGNAL(selectionChanged(const ISelectionManager*)),
            this, SLOT(onSelectionChanged(const ISelectionManager*)));
}

void BaseGenericPluginInstance::initialise(IGraphModel* graphModel, ISelectionManager* selectionManager, const IParserThread* parserThread)
{
    BasePluginInstance::initialise(graphModel, selectionManager, parserThread);

    _nodeAttributes.initialise(graphModel->mutableGraph());
}

std::unique_ptr<IParser> BaseGenericPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == "GML")
        return std::make_unique<GmlFileParser>(&_nodeAttributes);
    else if(urlTypeName == "PairwiseTXT")
        return std::make_unique<PairwiseTxtFileParser>(this, &_nodeAttributes);
    else if(urlTypeName == "GraphML")
        return std::make_unique<GraphMLParser>(&_nodeAttributes);

    return nullptr;
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

void BaseGenericPluginInstance::onLoadComplete()
{
    _nodeAttributes.setNodeNamesToFirstAttribute(*graphModel());
    _nodeAttributes.exposeToGraphModel(*graphModel());
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

void BaseGenericPluginInstance::onSelectionChanged(const ISelectionManager* selectionManager)
{
    emit selectedNodeNamesChanged();
    _nodeAttributesTableModel.setSelectedNodes(selectionManager->selectedNodes());
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
