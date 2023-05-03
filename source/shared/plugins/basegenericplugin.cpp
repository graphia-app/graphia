/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

using namespace Qt::Literals::StringLiterals;

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

    if(urlTypeName == u"GML"_s)
        return std::make_unique<GmlFileParser>(userNodeData, userEdgeData);

    if(urlTypeName == u"GraphML"_s)
        return std::make_unique<GraphMLParser>(userNodeData, userEdgeData);

    if(urlTypeName == u"DOT"_s)
        return std::make_unique<DotFileParser>(userNodeData, userEdgeData);

    if(urlTypeName.startsWith(u"Pairwise"_s))
    {
        std::unique_ptr<IParser> parser;

        auto configurePairwiseParser = [&](auto pairwiseParser)
        {
            pairwiseParser->setFirstRowIsHeader(_pairwiseParameters._firstRowIsHeader);
            pairwiseParser->setColumnsConfiguration(_pairwiseParameters._columns);

            return pairwiseParser;
        };

        if(urlTypeName == u"PairwiseCSV"_s)
            parser = configurePairwiseParser(std::make_unique<PairwiseCSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == u"PairwiseSSV"_s)
            parser = configurePairwiseParser(std::make_unique<PairwiseSSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == u"PairwiseTSV"_s)
            parser = configurePairwiseParser(std::make_unique<PairwiseTSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == u"PairwiseTXT"_s)
            parser = configurePairwiseParser(std::make_unique<PairwiseTXTFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == u"PairwiseXLSX"_s)
            parser = configurePairwiseParser(std::make_unique<PairwiseXLSXFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));

        return parser;
    }

    if(urlTypeName.startsWith(u"Matrix"_s))
    {
        std::unique_ptr<IParser> parser;

        auto configureMatrixParser = [&](auto matrixParser)
        {
            if(_adjacencyMatrixParameters._filterEdges)
                matrixParser->setMinimumAbsEdgeWeight(_adjacencyMatrixParameters._minimumAbsEdgeWeight);

            matrixParser->setSkipDuplicates(_adjacencyMatrixParameters._skipDuplicates);

            return matrixParser;
        };

        if(urlTypeName == u"MatrixCSV"_s)
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixCSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == u"MatrixSSV"_s)
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixSSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == u"MatrixTSV"_s)
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixTSVFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == u"MatrixTXT"_s)
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixTXTFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == u"MatrixXLSX"_s)
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixXLSXFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));
        else if(urlTypeName == u"MatrixMatLab"_s)
            parser = configureMatrixParser(std::make_unique<AdjacencyMatrixMatLabFileParser>(userNodeData, userEdgeData, &_preloadedTabularData));

        return parser;
    }

    if(urlTypeName == u"BiopaxOWL"_s)
        return std::make_unique<BiopaxFileParser>(userNodeData);

    if(urlTypeName == u"JSONGraph"_s)
        return std::make_unique<JsonGraphParser>(userNodeData, userEdgeData);

    if(urlTypeName == u"CX"_s)
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
            static_cast<PairwiseColumnType>(info.value(u"type"_s).toInt()) : PairwiseColumnType::Unused;
        auto name = u::contains(info, "name") ? info.value(u"name"_s).toString() : QString();

        columns[column] = {type, name};
    }

    return columns;
}

void BaseGenericPluginInstance::applyParameter(const QString& name, const QVariant& value)
{
    if(name == u"firstRowIsHeader"_s)
        _pairwiseParameters._firstRowIsHeader = (value == u"true"_s);
    else if(name == u"columns"_s)
        _pairwiseParameters._columns = pairwiseColumns(value);
    else if(name == u"minimumThreshold"_s)
        _adjacencyMatrixParameters._minimumAbsEdgeWeight = value.toDouble();
    else if(name == u"initialThreshold"_s)
        _adjacencyMatrixParameters._initialAbsEdgeWeightThreshold = value.toDouble();
    else if(name == u"filterEdges"_s)
        _adjacencyMatrixParameters._filterEdges = (value == u"true"_s);
    else if(name == u"skipDuplicates"_s)
        _adjacencyMatrixParameters._skipDuplicates = (value == u"true"_s);
    else if(name == u"data"_s && value.canConvert<std::shared_ptr<TabularData>>())
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
            s += u", "_s;

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
    registerUrlType(u"GML"_s, QObject::tr("GML File"), QObject::tr("GML Files"), {"gml"});
    registerUrlType(u"GraphML"_s, QObject::tr("GraphML File"), QObject::tr("GraphML Files"), {"graphml"});
    registerUrlType(u"DOT"_s, QObject::tr("DOT File"), QObject::tr("DOT Files"), {"dot"});
    registerUrlType(u"PairwiseCSV"_s, QObject::tr("Pairwise CSV File"), QObject::tr("Pairwise CSV Files"), {"csv"});
    registerUrlType(u"PairwiseSSV"_s, QObject::tr("Pairwise SSV File"), QObject::tr("Pairwise SSV Files"), {"ssv"});
    registerUrlType(u"PairwiseTSV"_s, QObject::tr("Pairwise TSV File"), QObject::tr("Pairwise TSV Files"), {"tsv"});
    registerUrlType(u"PairwiseTXT"_s, QObject::tr("Pairwise Text File"), QObject::tr("Pairwise Text Files"), {"txt"});
    registerUrlType(u"PairwiseXLSX"_s, QObject::tr("Pairwise Excel File"), QObject::tr("Pairwise Excel Files"), {"xlsx"});
    registerUrlType(u"MatrixCSV"_s, QObject::tr("Adjacency Matrix CSV File"), QObject::tr("Adjacency Matrix CSV Files"), {"csv"});
    registerUrlType(u"MatrixSSV"_s, QObject::tr("Adjacency Matrix SSV File"), QObject::tr("Adjacency Matrix SSV Files"), {"ssv"});
    registerUrlType(u"MatrixTSV"_s, QObject::tr("Adjacency Matrix TSV File"), QObject::tr("Adjacency Matrix TSV Files"), {"tsv"});
    registerUrlType(u"MatrixTXT"_s, QObject::tr("Adjacency Matrix TXT File"), QObject::tr("Adjacency Matrix TXT Files"), {"txt"});
    registerUrlType(u"MatrixXLSX"_s, QObject::tr("Adjacency Matrix Excel File"), QObject::tr("Adjacency Matrix Excel Files"), {"xlsx"});
    registerUrlType(u"MatrixMatLab"_s, QObject::tr("Matlab Data File"), QObject::tr("Matlab Data Files"), {"mat"});
    registerUrlType(u"BiopaxOWL"_s, QObject::tr("Biopax OWL File"), QObject::tr("Biopax OWL Files"), {"owl"});
    registerUrlType(u"JSONGraph"_s, QObject::tr("JSON Graph File"), QObject::tr("JSON Graph Files"), {"json"});
    registerUrlType(u"CX"_s, QObject::tr("Cytoscape Exchange File"), QObject::tr("Cytoscape Exchange Files"), {"cx", "cx2"});
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
            (urlType == u"GML"_s && GmlFileParser::canLoad(url)) ||
            (urlType == u"GraphML"_s && GraphMLParser::canLoad(url)) ||
            (urlType == u"DOT"_s && DotFileParser::canLoad(url)) ||
            (urlType == u"PairwiseCSV"_s && PairwiseCSVFileParser::canLoad(url)) ||
            (urlType == u"PairwiseSSV"_s && PairwiseSSVFileParser::canLoad(url)) ||
            (urlType == u"PairwiseTSV"_s && PairwiseTSVFileParser::canLoad(url)) ||
            (urlType == u"PairwiseTXT"_s && PairwiseTXTFileParser::canLoad(url)) ||
            (urlType == u"PairwiseXLSX"_s && PairwiseXLSXFileParser::canLoad(url)) ||
            (urlType == u"MatrixCSV"_s && AdjacencyMatrixCSVFileParser::canLoad(url)) ||
            (urlType == u"MatrixSSV"_s && AdjacencyMatrixSSVFileParser::canLoad(url)) ||
            (urlType == u"MatrixTSV"_s && AdjacencyMatrixTSVFileParser::canLoad(url)) ||
            (urlType == u"MatrixTXT"_s && AdjacencyMatrixTXTFileParser::canLoad(url)) ||
            (urlType == u"MatrixXLSX"_s && AdjacencyMatrixXLSXFileParser::canLoad(url)) ||
            (urlType == u"MatrixMatLab"_s && AdjacencyMatrixMatLabFileParser::canLoad(url)) ||
            (urlType == u"BiopaxOWL"_s && BiopaxFileParser::canLoad(url)) ||
            (urlType == u"JSONGraph"_s && JsonGraphParser::canLoad(url)) ||
            (urlType == u"CX"_s && CxParser::canLoad(url));

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
            .arg(urlTypes.join(", "));
    }

    return {};
}

QString BaseGenericPlugin::parametersQmlPath(const QString& urlType) const
{
    if(urlType.startsWith(u"Matrix"_s))
        return u"qrc:///qml/MatrixParameters.qml"_s;

    if(urlType.startsWith(u"Pairwise"_s))
        return u"qrc:///qml/PairwiseParameters.qml"_s;

    return {};
}
