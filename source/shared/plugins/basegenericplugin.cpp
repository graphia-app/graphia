#include "basegenericplugin.h"

#include "shared/loading/biopaxfileparser.h"
#include "shared/loading/gmlfileparser.h"
#include "shared/loading/pairwisetxtfileparser.h"
#include "shared/loading/graphmlparser.h"
#include "shared/loading/matrixfileparser.h"
#include "shared/loading/matfileparser.h"
#include "shared/loading/jsongraphparser.h"

#include "shared/attributes/iattribute.h"

#include "shared/utils/container.h"
#include "shared/utils/iterator_range.h"

#include <json_helper.h>

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

    if(urlTypeName == QLatin1String("MatrixCSV"))
        return std::make_unique<MatrixFileCSVParser>(&_userNodeData, &_userEdgeData);

    if(urlTypeName == QLatin1String("MatrixSSV"))
        return std::make_unique<MatrixFileSSVParser>(&_userNodeData, &_userEdgeData);

    if(urlTypeName == QLatin1String("MatrixTSV"))
        return std::make_unique<MatrixFileTSVParser>(&_userNodeData, &_userEdgeData);

    if(urlTypeName == QLatin1String("BiopaxOWL"))
        return std::make_unique<BiopaxFileParser>(&_userNodeData);
    
    if(urlTypeName == QLatin1String("MatFile"))
        return std::make_unique<MatFileParser>(&_userNodeData, &_userEdgeData);

    if(urlTypeName == QLatin1String("JSONGraph"))
        return std::make_unique<JsonGraphParser>(&_userNodeData, &_userEdgeData);

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

    json jsonObject = parseJsonFrom(data, &parser);

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
    registerUrlType(QStringLiteral("MatrixCSV"), QObject::tr("Matrix CSV File"), QObject::tr("Matrix CSV Files"), {"csv", "matrix"});
    registerUrlType(QStringLiteral("MatrixSSV"), QObject::tr("Matrix SSV File"), QObject::tr("Matrix SSV Files"), {"csv", "matrix"});
    registerUrlType(QStringLiteral("MatrixTSV"), QObject::tr("Matrix File"), QObject::tr("Matrix Files"), {"tsv", "matrix"});
    registerUrlType(QStringLiteral("BiopaxOWL"), QObject::tr("Biopax OWL File"), QObject::tr("Biopax OWL Files"), {"owl"});
    registerUrlType(QStringLiteral("MatFile"), QObject::tr("Matlab Data File"), QObject::tr("Matlab Data Files"), {"mat"});
    registerUrlType(QStringLiteral("JSONGraph"), QObject::tr("JSON Graph File"), QObject::tr("JSON Graph Files"), {"json"});
}

QStringList BaseGenericPlugin::identifyUrl(const QUrl& url) const
{
    auto urlTypes = identifyByExtension(url);

    if(urlTypes.isEmpty())
        return {};

    QStringList result;

    for(const auto& urlType : urlTypes)
    {
        if(urlType == QStringLiteral("GML") && GmlFileParser::canLoad(url))
            result.push_back(urlType);
        else if(urlType == QStringLiteral("PairwiseTXT") && PairwiseTxtFileParser::canLoad(url))
            result.push_back(urlType);
        else if(urlType == QStringLiteral("GraphML") && GraphMLParser::canLoad(url))
            result.push_back(urlType);
        else if(urlType == QStringLiteral("MatrixCSV") && MatrixFileCSVParser::canLoad(url))
            result.push_back(urlType);
        else if(urlType == QStringLiteral("MatrixSSV") && MatrixFileSSVParser::canLoad(url))
            result.push_back(urlType);
        else if(urlType == QStringLiteral("MatrixTSV") && MatrixFileTSVParser::canLoad(url))
            result.push_back(urlType);
        else if(urlType == QStringLiteral("BiopaxOWL") && BiopaxFileParser::canLoad(url))
            result.push_back(urlType);
        else if(urlType == QStringLiteral("MatFile") && MatFileParser::canLoad(url))
            result.push_back(urlType);
        else if(urlType == QStringLiteral("JSONGraph") && JsonGraphParser::canLoad(url))
            result.push_back(urlType);
    }

    return result;
}

QString BaseGenericPlugin::failureReason(const QUrl& url) const
{
    auto urlTypes = identifyByExtension(url);
    if(!urlTypes.isEmpty())
    {
        return tr("The file's contents do not match its filename extension. Extension: %1")
            .arg(urlTypes.join(','));
    }

    return {};
}
