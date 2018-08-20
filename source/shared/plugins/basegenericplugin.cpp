#include "basegenericplugin.h"

#include "shared/loading/gmlfileparser.h"
#include "shared/loading/pairwisetxtfileparser.h"
#include "shared/loading/graphmlparser.h"
#include "shared/loading/matrixfileparser.h"

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

    if(urlTypeName == QLatin1String("MatrixCSV"))
        return std::make_unique<MatrixFileParser>(&_userNodeData, &_userEdgeData);

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
    registerUrlType(QStringLiteral("MatrixCSV"), QObject::tr("Matrix CSV File"), QObject::tr("Matrix CSV Files"), {"csv"});
    registerUrlType(QStringLiteral("Matrix"), QObject::tr("Matrix File"), QObject::tr("Matrix Files"), {"matrix"});
}


static QStringList contentIdentityOf(const QUrl& url)
{
    QStringList result;

    std::ifstream file(url.toLocalFile().toStdString());
    std::string line;

    QFile qFile(url.toLocalFile());
    qFile.open(QFile::ReadOnly | QIODevice::Truncate);
    QTextStream stream(&qFile);

    //Matrix Scanning
    QString qline = stream.readLine().trimmed();
    QStringList potentialColumnHeaders = qline.split(QRegExp("(\\s|,)"));
    qDebug() << potentialColumnHeaders;
    const int LINE_SCAN_COUNT = 5;
    bool isMatrix = true;

    // Check if row and column headers match, if so it's very likely a matrix
    if(!potentialColumnHeaders.isEmpty())
    {
        bool headerIsDouble = false;
        for(QString header : potentialColumnHeaders)
        {
            bool success = false;
            header.toDouble(&success);
            if(success && !header.isEmpty())
            {
                headerIsDouble = true;
                break;
            }
        }
        for(int i = 0; i < LINE_SCAN_COUNT && !stream.atEnd(); ++i)
        {
            QString line = stream.readLine();
            QStringList splitline = line.split(QRegExp("(\\s|,)"));
            qDebug() << splitline;
            if(!splitline.isEmpty())
            {
                if(splitline.first() != potentialColumnHeaders[i])
                {
                    isMatrix = false;
                    qDebug() << "Didn't match!";
                }
            }
        }
    }
    qDebug() << "Is Matrix" << isMatrix;


    if(file && u::getline(file, line))
    {
        size_t numCommas = 0;
        size_t numTabs = 0;
        size_t numSpaces = 0;
        bool inQuotes = false;

        for(auto character : line)
        {
            switch(character)
            {
            case '"': inQuotes = !inQuotes; break;
            case ',': if(!inQuotes) { numCommas++; } break;
            case '\t': if(!inQuotes) { numTabs++; } break;
            case ' ': if(!inQuotes) { numSpaces++; } break;
            default: break;
            }
        }

        auto max = std::max({numCommas, numTabs, numSpaces});

        if(numTabs == max)
            result.append(QStringLiteral("CorrelationTSV"));
        if(numCommas == max)
            result.append(QStringLiteral("CorrelationCSV"));
        if(numSpaces == max)
            result.append(QStringLiteral("PairwiseTXT"));
    }

    return result;
}


QStringList BaseGenericPlugin::identifyUrl(const QUrl& url) const
{
    auto urlTypes = identifyByExtension(url);
    contentIdentityOf(url);

    if(urlTypes.isEmpty())
        return {};

    return urlTypes;
}
