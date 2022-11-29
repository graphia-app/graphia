/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "basegenericplugin.h"

#include "shared/loading/biopaxfileparser.h"
#include "shared/loading/gmlfileparser.h"
#include "shared/loading/dotfileparser.h"
#include "shared/loading/graphmlparser.h"
#include "shared/loading/adjacencymatrixfileparser.h"
#include "shared/loading/pairwisefileparser.h"
#include "shared/loading/jsongraphparser.h"
#include "shared/loading/cxparser.h"

#include "shared/attributes/iattribute.h"

#include "shared/utils/container.h"

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

    _graphModel = document->graphModel();
    _nodeAttributeTableModel.initialise(document, &_graphModel->userNodeData());
}

std::unique_ptr<IParser> BaseGenericPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    auto* userNodeData = &_graphModel->userNodeData();
    auto* userEdgeData = &_graphModel->userEdgeData();

    if(urlTypeName == QStringLiteral("GML"))
        return std::make_unique<GmlFileParser>(userNodeData, userEdgeData);

    if(urlTypeName == QStringLiteral("GraphML"))
        return std::make_unique<GraphMLParser>(userNodeData, userEdgeData);

    if(urlTypeName == QStringLiteral("DOT"))
        return std::make_unique<DotFileParser>(userNodeData, userEdgeData);

    if(urlTypeName.startsWith(QStringLiteral("Pairwise")))
    {
        std::unique_ptr<IParser> parser;

        auto configurePairwiseParser = [&](auto pairwiseParser)
        {
            pairwiseParser->setFirstRowIsHeader(_pairwiseParameters._firstRowIsHeader);
            pairwiseParser->setColumnsConfiguration(_pairwiseParameters._columns);

            return pairwiseParser;
        };

        if(urlTypeName == QStringLiteral("PairwiseCSV"))
            parser = configurePairwiseParser(std::make_unique<PairwiseCSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == QStringLiteral("PairwiseSSV"))
            parser = configurePairwiseParser(std::make_unique<PairwiseSSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == QStringLiteral("PairwiseTSV"))
            parser = configurePairwiseParser(std::make_unique<PairwiseTSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == QStringLiteral("PairwiseTXT"))
            parser = configurePairwiseParser(std::make_unique<PairwiseTXTFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == QStringLiteral("PairwiseXLSX"))
            parser = configurePairwiseParser(std::make_unique<PairwiseXLSXFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));

        return parser;
    }

    if(urlTypeName.startsWith(QStringLiteral("Matrix")))
    {
        std::unique_ptr<IParser> parser;

        auto configureMatrixParser = [&](auto matrixParser)
        {
            if(_adjacencyMatrixParameters._filterEdges)
                matrixParser->setMinimumAbsEdgeWeight(_adjacencyMatrixParameters._minimumAbsEdgeWeight);

            matrixParser->setSkipDuplicates(_adjacencyMatrixParameters._skipDuplicates);

            return matrixParser;
        };

        if(urlTypeName == QStringLiteral("MatrixCSV"))
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixCSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == QStringLiteral("MatrixSSV"))
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixSSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == QStringLiteral("MatrixTSV"))
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixTSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == QStringLiteral("MatrixXLSX"))
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixXLSXFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == QStringLiteral("MatrixMatLab"))
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixMatLabFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));

        return parser;
    }

    if(urlTypeName == QStringLiteral("BiopaxOWL"))
        return std::make_unique<BiopaxFileParser>(userNodeData);

    if(urlTypeName == QStringLiteral("JSONGraph"))
        return std::make_unique<JsonGraphParser>(userNodeData, userEdgeData);

    if(urlTypeName == QStringLiteral("CX"))
        return std::make_unique<CxParser>(userNodeData, userEdgeData);

    return nullptr;
}

static auto pairwiseColumns(const QVariant& value)
{
    PairwiseColumnsConfiguration columns;

    const auto map = value.toMap();

    const auto& keys = map.keys();
    for(const auto& key : keys)
    {
        Q_ASSERT(u::isInteger(key));

        auto column = static_cast<size_t>(key.toInt());
        auto info = map.value(key).toMap();
        auto type = u::contains(info, "type") ?
            static_cast<PairwiseColumnType>(info.value(QStringLiteral("type")).toInt()) : PairwiseColumnType::Unused;
        auto name = u::contains(info, "name") ? info.value(QStringLiteral("name")).toString() : QString();

        columns[column] = {type, name};
    }

    return columns;
}

void BaseGenericPluginInstance::applyParameter(const QString& name, const QVariant& value)
{
    if(name == QStringLiteral("firstRowIsHeader"))
        _pairwiseParameters._firstRowIsHeader = (value == QStringLiteral("true"));
    else if(name == QStringLiteral("columns"))
        _pairwiseParameters._columns = pairwiseColumns(value);
    else if(name == QStringLiteral("minimumThreshold"))
        _adjacencyMatrixParameters._minimumAbsEdgeWeight = value.toDouble();
    else if(name == QStringLiteral("initialThreshold"))
        _adjacencyMatrixParameters._initialAbsEdgeWeightThreshold = value.toDouble();
    else if(name == QStringLiteral("filterEdges"))
        _adjacencyMatrixParameters._filterEdges = (value == QStringLiteral("true"));
    else if(name == QStringLiteral("skipDuplicates"))
        _adjacencyMatrixParameters._skipDuplicates = (value == QStringLiteral("true"));
    else if(name == QStringLiteral("data") && value.canConvert<std::shared_ptr<TabularData>>())
        _preloadedTabularData = std::move(*value.value<std::shared_ptr<TabularData>>());
    else
        qDebug() << "BaseGenericPluginInstance::applyParameter unknown parameter" << name << value;
}

QStringList BaseGenericPluginInstance::defaultTransforms() const
{
    QStringList defaultTransforms;

    if(_adjacencyMatrixParameters._filterEdges)
    {
        defaultTransforms.append(
            QStringLiteral(R"("Remove Edges" where $"Absolute Edge Weight" < %2)")
            .arg(_adjacencyMatrixParameters._initialAbsEdgeWeightThreshold));
    }

    return defaultTransforms;
}

QString BaseGenericPluginInstance::selectedNodeNames() const
{
    QString s;

    for(auto nodeId : selectionManager()->selectedNodes())
    {
        if(!s.isEmpty())
            s += QStringLiteral(", ");

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
        auto nodeId = _graphModel->userNodeData().elementIdForIndex(static_cast<size_t>(row));
        highlightedNodeIds.insert(nodeId);
    }

    document()->highlightNodes(highlightedNodeIds);

    emit highlightedRowsChanged();
}

void BaseGenericPluginInstance::onLoadSuccess()
{
    _nodeAttributeTableModel.updateColumnNames();
}

void BaseGenericPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    emit selectedNodeNamesChanged();
    _nodeAttributeTableModel.onSelectionChanged();
}

// NOLINTNEXTLINE modernize-use-equals-default
BaseGenericPlugin::BaseGenericPlugin()
{
    registerUrlType(QStringLiteral("GML"), QObject::tr("GML File"), QObject::tr("GML Files"), {"gml"});
    registerUrlType(QStringLiteral("GraphML"), QObject::tr("GraphML File"), QObject::tr("GraphML Files"), {"graphml"});
    registerUrlType(QStringLiteral("DOT"), QObject::tr("DOT File"), QObject::tr("DOT Files"), {"dot"});
    registerUrlType(QStringLiteral("PairwiseCSV"), QObject::tr("Pairwise CSV File"), QObject::tr("Pairwise CSV Files"), {"csv"});
    registerUrlType(QStringLiteral("PairwiseSSV"), QObject::tr("Pairwise SSV File"), QObject::tr("Pairwise SSV Files"), {"ssv"});
    registerUrlType(QStringLiteral("PairwiseTSV"), QObject::tr("Pairwise TSV File"), QObject::tr("Pairwise TSV Files"), {"tsv"});
    registerUrlType(QStringLiteral("PairwiseTXT"), QObject::tr("Pairwise Text File"), QObject::tr("Pairwise Text Files"), {"txt"});
    registerUrlType(QStringLiteral("PairwiseXLSX"), QObject::tr("Pairwise Excel File"), QObject::tr("Pairwise Excel Files"), {"xlsx"});
    registerUrlType(QStringLiteral("MatrixCSV"), QObject::tr("Adjacency Matrix CSV File"), QObject::tr("Adjacency Matrix CSV Files"), {"csv"});
    registerUrlType(QStringLiteral("MatrixSSV"), QObject::tr("Adjacency Matrix SSV File"), QObject::tr("Adjacency Matrix SSV Files"), {"ssv"});
    registerUrlType(QStringLiteral("MatrixTSV"), QObject::tr("Adjacency Matrix File"), QObject::tr("Adjacency Matrix Files"), {"tsv"});
    registerUrlType(QStringLiteral("MatrixXLSX"), QObject::tr("Adjacency Matrix Excel File"), QObject::tr("Adjacency Matrix Excel Files"), {"xlsx"});
    registerUrlType(QStringLiteral("MatrixMatLab"), QObject::tr("Matlab Data File"), QObject::tr("Matlab Data Files"), {"mat"});
    registerUrlType(QStringLiteral("BiopaxOWL"), QObject::tr("Biopax OWL File"), QObject::tr("Biopax OWL Files"), {"owl"});
    registerUrlType(QStringLiteral("JSONGraph"), QObject::tr("JSON Graph File"), QObject::tr("JSON Graph Files"), {"json"});
    registerUrlType(QStringLiteral("CX"), QObject::tr("Cytoscape Exchange File"), QObject::tr("Cytoscape Exchange Files"), {"cx", "cx2"});
}

QStringList BaseGenericPlugin::identifyUrl(const QUrl& url) const
{
    if(!url.isLocalFile())
        return {};

    auto urlTypes = identifyByExtension(url);

    if(urlTypes.isEmpty())
        return {};

    QStringList result;
    result.reserve(urlTypes.size());

    for(const auto& urlType : urlTypes)
    {
        const bool canLoad =
            (urlType == QStringLiteral("GML") && GmlFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("GraphML") && GraphMLParser::canLoad(url)) ||
            (urlType == QStringLiteral("DOT") && DotFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("PairwiseCSV") && PairwiseCSVFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("PairwiseSSV") && PairwiseSSVFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("PairwiseTSV") && PairwiseTSVFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("PairwiseTXT") && PairwiseTXTFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("PairwiseXLSX") && PairwiseXLSXFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("MatrixCSV") && AdjacencyMatrixCSVFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("MatrixSSV") && AdjacencyMatrixSSVFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("MatrixTSV") && AdjacencyMatrixTSVFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("MatrixXLSX") && AdjacencyMatrixXLSXFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("MatrixMatLab") && AdjacencyMatrixMatLabFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("BiopaxOWL") && BiopaxFileParser::canLoad(url)) ||
            (urlType == QStringLiteral("JSONGraph") && JsonGraphParser::canLoad(url)) ||
            (urlType == QStringLiteral("CX") && CxParser::canLoad(url));

        if(canLoad)
            result.push_back(urlType);
    }

    return result;
}

QString BaseGenericPlugin::failureReason(const QUrl& url) const
{
    auto urlTypes = identifyByExtension(url);
    if(!urlTypes.isEmpty())
    {
        return tr("The file cannot be loaded. The file extension "
            "was used to determine the file's possible type(s): %1. "
            "Please check its contents are of this type.")
            .arg(urlTypes.join(','));
    }

    return {};
}

QString BaseGenericPlugin::parametersQmlPath(const QString& urlType) const
{
    if(urlType.startsWith(QStringLiteral("Matrix")))
        return QStringLiteral("qrc:///qml/MatrixParameters.qml");

    if(urlType.startsWith(QStringLiteral("Pairwise")))
        return QStringLiteral("qrc:///qml/PairwiseParameters.qml");

    return {};
}
