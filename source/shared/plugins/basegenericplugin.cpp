#include "basegenericplugin.h"

#include "shared/loading/gmlfileparser.h"
#include "shared/loading/pairwisetxtfileparser.h"
#include "shared/loading/graphmlparser.h"

#include "shared/attributes/iattribute.h"

#include "shared/utils/container.h"
#include "shared/utils/iterator_range.h"

#include "thirdparty/json/json_helper.h"

BaseGenericPluginInstance::BaseGenericPluginInstance()
{
    connect(this, SIGNAL(loadSuccess()), this, SLOT(onLoadSuccess()));
    connect(this, SIGNAL(selectionChanged(const ISelectionManager*)),
            this, SLOT(onSelectionChanged(const ISelectionManager*)), Qt::DirectConnection);
}

void BaseGenericPluginInstance::initialise(const IPlugin* plugin, IDocument* document,
                                           const IParserThread* parserThread)
{
    BasePluginInstance::initialise(plugin, document, parserThread);

    auto graphModel = document->graphModel();

    _userNodeData.initialise(graphModel->mutableGraph());
    _nodeAttributeTableModel.initialise(document, &_userNodeData);

    _userEdgeData.initialise(graphModel->mutableGraph());
}

std::unique_ptr<IParser> BaseGenericPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == QLatin1String("GML"))
        return std::make_unique<GmlFileParser>(&_userNodeData, &_userEdgeData);

    if(urlTypeName == QLatin1String("PairwiseTXT"))
        return std::make_unique<PairwiseTxtFileParser>(&_userNodeData, &_userEdgeData);

    if(urlTypeName == QLatin1String("GraphML"))
        return std::make_unique<GraphMLParser>(&_userNodeData);

    return nullptr;
}

QByteArray BaseGenericPluginInstance::save(IMutableGraph& graph, Progressable& progressable) const
{
    json jsonObject;

    progressable.setProgress(-1);

    jsonObject["userNodeData"] = _userNodeData.save(graph, progressable);
    jsonObject["userEdgeData"] = _userEdgeData.save(graph, progressable);

    return QByteArray::fromStdString(jsonObject.dump());
}

bool BaseGenericPluginInstance::load(const QByteArray& data, int dataVersion,
                                     IMutableGraph& graph, IParser& parser)
{
    if(dataVersion != plugin()->dataVersion())
        return false;

    json jsonObject = parseJsonFrom(data, parser);

    if(parser.cancelled())
        return false;

    if(jsonObject.is_null() || !jsonObject.is_object())
        return false;

    parser.setProgress(-1);

    if(!u::contains(jsonObject, "userNodeData") || !jsonObject["userNodeData"].is_object())
        return false;

    graph.setPhase(QObject::tr("Node Data"));
    if(!_userNodeData.load(jsonObject["userNodeData"], parser))
        return false;

    if(!u::contains(jsonObject, "userEdgeData") || !jsonObject["userEdgeData"].is_object())
        return false;

    graph.setPhase(QObject::tr("Edge Data"));
    if(!_userEdgeData.load(jsonObject["userEdgeData"], parser))
        return false; // NOLINT

    return true;
}

QString BaseGenericPluginInstance::selectedNodeNames() const
{
    QString s;

    for(auto nodeId : selectionManager()->selectedNodes())
    {
        if(!s.isEmpty())
            s += QLatin1String(", ");

        s += graphModel()->nodeName(nodeId);
    }

    return s;
}

void BaseGenericPluginInstance::setHighlightedRows(const QVector<int>& highlightedRows)
{
    if(_highlightedRows.isEmpty() && highlightedRows.isEmpty())
        return;

    _highlightedRows = highlightedRows;

    NodeIdSet highlightedNodeIds;
    for(auto row : highlightedRows)
    {
        auto nodeId = _userNodeData.elementIdForRowIndex(static_cast<size_t>(row));
        highlightedNodeIds.insert(nodeId);
    }

    document()->highlightNodes(highlightedNodeIds);

    emit highlightedRowsChanged();
}

void BaseGenericPluginInstance::onLoadSuccess()
{
    _userNodeData.exposeAsAttributes(*graphModel());
    _userEdgeData.exposeAsAttributes(*graphModel());
    _nodeAttributeTableModel.updateRoleNames();
}

void BaseGenericPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    emit selectedNodeNamesChanged();
    _nodeAttributeTableModel.onSelectionChanged();
}

BaseGenericPlugin::BaseGenericPlugin()
{
    registerUrlType(QStringLiteral("GML"), QObject::tr("GML File"), QObject::tr("GML Files"), {"gml"});
    registerUrlType(QStringLiteral("PairwiseTXT"), QObject::tr("Pairwise Text File"), QObject::tr("Pairwise Text Files"), {"txt", "layout"});
    registerUrlType(QStringLiteral("GraphML"), QObject::tr("GraphML File"), QObject::tr("GraphML Files"), {"graphml"});

}

QStringList BaseGenericPlugin::identifyUrl(const QUrl& url) const
{
    //FIXME actually look at the file contents
    return identifyByExtension(url);
}
