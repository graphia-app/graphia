/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include "correlationplugin.h"

#include "correlation.h"
#include "correlationplotitem.h"
#include "graphsizeestimateplotitem.h"

#include "shared/graph/grapharray_json.h"

#include "shared/utils/threadpool.h"
#include "shared/utils/iterator_range.h"
#include "shared/utils/container.h"
#include "shared/utils/random.h"
#include "shared/utils/string.h"
#include "shared/utils/redirects.h"

#include "shared/attributes/iattribute.h"

#include "shared/ui/visualisations/ielementvisual.h"

#include "shared/loading/xlsxtabulardataparser.h"

#include <json_helper.h>

#include <map>

CorrelationPluginInstance::CorrelationPluginInstance()
{
    connect(this, SIGNAL(loadSuccess()), this, SLOT(onLoadSuccess()));
    connect(this, SIGNAL(selectionChanged(const ISelectionManager*)),
            this, SLOT(onSelectionChanged(const ISelectionManager*)), Qt::DirectConnection);
    connect(this, SIGNAL(visualsChanged()), this, SIGNAL(nodeColorsChanged()));
}

void CorrelationPluginInstance::initialise(const IPlugin* plugin, IDocument* document,
                                           const IParserThread* parserThread)
{
    BasePluginInstance::initialise(plugin, document, parserThread);

    _graphModel = document->graphModel();
    _userNodeData.initialise(_graphModel->mutableGraph());
    _nodeAttributeTableModel.initialise(document, &_userNodeData);

    _correlationValues = std::make_unique<EdgeArray<double>>(_graphModel->mutableGraph());

    const auto* modelQObject = dynamic_cast<const QObject*>(_graphModel);
    connect(modelQObject, SIGNAL(attributesChanged(const QStringList&, const QStringList&)),
            this, SIGNAL(sharedValuesAttributeNamesChanged()));
    connect(modelQObject, SIGNAL(attributesChanged(const QStringList&, const QStringList&)),
            this, SIGNAL(numericalAttributeNamesChanged()));
}

bool CorrelationPluginInstance::loadUserData(const TabularData& tabularData,
    const QRect& dataRect, IParser& parser)
{
    if(dataRect.x() == 0 || dataRect.y() == 0)
    {
        qDebug() << "tabularData has no row or column names!";
        return false;
    }

    parser.setProgress(-1);

    uint64_t numDataPoints = static_cast<uint64_t>(tabularData.numColumns()) * tabularData.numRows();
    size_t left = dataRect.x();
    size_t right = dataRect.x() + dataRect.width();
    size_t top = dataRect.y();
    size_t bottom = dataRect.y() + dataRect.height();

    for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            if(parser.cancelled())
                return false;

            uint64_t rowOffset = static_cast<uint64_t>(rowIndex) * tabularData.numColumns();
            uint64_t dataPoint = columnIndex + rowOffset;
            parser.setProgress(static_cast<int>((dataPoint * 100) / numDataPoints));

            const auto& value = tabularData.valueAt(columnIndex, rowIndex);

            size_t dataColumnIndex = columnIndex - dataRect.x();
            size_t dataRowIndex = rowIndex - dataRect.y();
            bool isColumnInDataRect = left <= columnIndex && columnIndex < right;
            bool isRowInDataRect = top <= rowIndex && rowIndex < bottom;
            bool isColumnAnnotation = rowIndex < top;
            bool isRowAttribute = columnIndex < left;

            if((isColumnInDataRect && dataColumnIndex >= _numColumns) ||
                (isRowInDataRect && dataRowIndex >= _numRows))
            {
                qDebug() << QString("WARNING: Attempting to set data at coordinate (%1, %2) in "
                                    "dataRect of dimensions (%3, %4)")
                    .arg(dataColumnIndex).arg(dataRowIndex)
                    .arg(_numColumns).arg(_numRows);

                continue;
            }

            if(rowIndex == 0)
            {
                if(isColumnInDataRect)
                    setDataColumnName(dataColumnIndex, value);
                else if(isRowAttribute)
                    _userNodeData.add(value);
            }
            else if(isColumnAnnotation)
            {
                if(columnIndex == 0)
                    _userColumnData.add(value);
                else if(isColumnInDataRect)
                    _userColumnData.setValue(dataColumnIndex, tabularData.valueAt(0, rowIndex), value);
            }
            else if(isColumnInDataRect)
            {
                double transformedValue = 0.0;

                if(!value.isEmpty())
                {
                    bool success = false;
                    transformedValue = value.toDouble(&success);
                    Q_ASSERT(success);
                }
                else
                {
                    transformedValue = CorrelationFileParser::imputeValue(_missingDataType, _missingDataReplacementValue,
                        tabularData, dataRect, columnIndex, rowIndex);
                    _imputedValues = true;
                }

                transformedValue = CorrelationFileParser::scaleValue(_scalingType, transformedValue);

                setData(dataColumnIndex, dataRowIndex, transformedValue);
            }
            else if(isRowAttribute)
                _userNodeData.setValue(dataRowIndex, tabularData.valueAt(columnIndex, 0), value);
        }
    }

    makeDataColumnNamesUnique();
    setNodeAttributeTableModelDataColumns();
    parser.setProgress(-1);

    return true;
}

void CorrelationPluginInstance::normalise(IParser* parser)
{
    CorrelationFileParser::normalise(_normaliseType, _dataRows, parser);

    // Normalising obviously changes all the values in _dataRows, so we
    // must sync _data up so that it matches
    _data.clear();
    for(const auto& dataRow : _dataRows)
        _data.insert(_data.end(), dataRow.begin(), dataRow.end());
}

void CorrelationPluginInstance::finishDataRows()
{
    for(size_t row = 0; row < _numRows; row++)
        finishDataRow(row);
}

void CorrelationPluginInstance::createAttributes()
{
    size_t columnIndex = 0;
    std::map<QString, size_t> dataValueAttributeColumn;

    for(const auto& dataColumnName : _dataColumnNames)
        dataValueAttributeColumn[dataColumnName] = columnIndex++;

    graphModel()->createAttribute(tr("Data Value"))
        .setFloatValueFn([this, dataValueAttributeColumn](NodeId nodeId, const IAttribute& attribute)
        {
            auto columnName = attribute.parameterValue();
            if(columnName.isEmpty())
                return 0.0;

            auto column = dataValueAttributeColumn.at(columnName);
            return dataRowForNodeId(nodeId).valueAt(column);
        })
        .setFlag(AttributeFlag::AutoRange)
        .setUserDefined(true)
        .setDescription(tr("The Data Value is a parameterised attribute that permits referencing "
            "a specific column in the correlated data."))
        .setValidParameterValues(u::toQStringList(_dataColumnNames));

    graphModel()->createAttribute(tr("Strongest Data Column"))
        .setStringValueFn([this](NodeId nodeId)
        {
            const auto& dataRow = dataRowForNodeId(nodeId);
            return _dataColumnNames.at(dataRow.largestColumnIndex());
        })
        .setFlag(AttributeFlag::FindShared)
        .setFlag(AttributeFlag::Searchable)
        .setDescription(tr("The name of the data column which has the largest absolute value for "
            "the data row associated with the node."));

    graphModel()->createAttribute(tr("Mean Data Value"))
        .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId).mean(); })
        .setFlag(AttributeFlag::AutoRange)
        .setDescription(tr("The Mean Data Value is the mean of the values associated "
            "with the node."));

    graphModel()->createAttribute(tr("Minimum Data Value"))
        .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId).minValue(); })
        .setFlag(AttributeFlag::AutoRange)
        .setDescription(tr("The Minimum Data Value is the minimum value associated "
            "with the node."));

    graphModel()->createAttribute(tr("Maximum Data Value"))
        .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId).maxValue(); })
        .setFlag(AttributeFlag::AutoRange)
        .setDescription(tr("The Maximum Data Value is the maximum value associated "
            "with the node."));

    graphModel()->createAttribute(tr("Variance"))
        .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId).variance(); })
        .setFlag(AttributeFlag::AutoRange)
        .setDescription(tr("The %1 is a measure of the spread of the values associated with the node. "
            "It is defined as ∑(<i>x</i>-µ)², where <i>x</i> is the value and µ is the mean.")
            .arg(u::redirectLink("variance", tr("Variance"))));

    graphModel()->createAttribute(tr("Standard Deviation"))
        .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId).stddev(); })
        .setFlag(AttributeFlag::AutoRange)
        .setDescription(tr("The %1 is a measure of the spread of the values associated "
            "with the node. It is defined as √∑(<i>x</i>-µ)², where <i>x</i> is the value "
            "and µ is the mean.").arg(u::redirectLink("stddev", tr("Standard Deviation"))));

    graphModel()->createAttribute(tr("Coefficient of Variation"))
        .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId).coefVar(); })
        .setValueMissingFn([this](NodeId nodeId) { return std::isnan(dataRowForNodeId(nodeId).coefVar()); })
        .setFlag(AttributeFlag::AutoRange)
        .setDescription(tr("The %1 is a measure of the spread of the values associated "
            "with the node. It is defined as the standard deviation divided by the mean.")
            .arg(u::redirectLink("coef_variation", tr("Coefficient of Variation"))));

    auto correlation = Correlation::create(static_cast<CorrelationType>(_correlationType));
    _correlationAttributeName = correlation->attributeName();

    graphModel()->createAttribute(_correlationAttributeName)
        .setFloatValueFn([this](EdgeId edgeId) { return _correlationValues->get(edgeId); })
        .setFlag(AttributeFlag::AutoRange)
        .setDescription(correlation->attributeDescription());

    auto correlationPolarity = static_cast<CorrelationPolarity>(_correlationPolarity);
    switch(correlationPolarity)
    {
    case CorrelationPolarity::Negative:
    case CorrelationPolarity::Both:
        _correlationAbsAttributeName = tr("Absolute ") + _correlationAttributeName;

        graphModel()->createAttribute(_correlationAbsAttributeName)
            .setFloatValueFn([this](EdgeId edgeId) { return std::abs(_correlationValues->get(edgeId)); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(correlation->attributeDescription());
        break;

    default:
        break;
    }
}

void CorrelationPluginInstance::setHighlightedRows(const QVector<int>& highlightedRows)
{
    if(_highlightedRows.isEmpty() && highlightedRows.isEmpty())
        return;

    _highlightedRows = highlightedRows;

    NodeIdSet highlightedNodeIds;
    for(auto row : highlightedRows)
    {
        auto nodeId = _userNodeData.elementIdForIndex(static_cast<size_t>(row));
        highlightedNodeIds.insert(nodeId);
    }

    document()->highlightNodes(highlightedNodeIds);

    emit highlightedRowsChanged();
}

QStringList CorrelationPluginInstance::sharedValuesAttributeNames() const
{
    auto attributeNames = _graphModel->attributeNamesMatching(
    [](const IAttribute& attribute)
    {
        return !attribute.hasParameter() &&
            attribute.elementType() == ElementType::Node &&
            !attribute.sharedValues().empty();
    });

    return u::toQStringList(attributeNames);
}

QStringList CorrelationPluginInstance::numericalAttributeNames() const
{
    auto attributeNames = _graphModel->attributeNamesMatching(
    [](const IAttribute& attribute)
    {
        return !attribute.hasParameter() &&
            attribute.elementType() == ElementType::Node &&
            attribute.valueType() & ValueType::Numerical;
    });

    return u::toQStringList(attributeNames);
}

EdgeList CorrelationPluginInstance::correlation(double minimumThreshold, IParser& parser)
{
    auto correlation = Correlation::create(static_cast<CorrelationType>(_correlationType));
    return correlation->process(_dataRows, minimumThreshold,
        static_cast<CorrelationPolarity>(_correlationPolarity), &parser, &parser);
}

bool CorrelationPluginInstance::createEdges(const EdgeList& edges, IParser& parser)
{
    parser.setProgress(-1);
    for(auto edgeIt = edges.begin(); edgeIt != edges.end(); ++edgeIt)
    {
        if(parser.cancelled())
            return false;

        parser.setProgress(static_cast<int>(std::distance(edges.begin(), edgeIt)) * 100 /
            static_cast<int>(edges.size()));

        const auto& edge = *edgeIt;
        auto edgeId = graphModel()->mutableGraph().addEdge(edge._source, edge._target);
        _correlationValues->set(edgeId, edge._weight);
    }

    return true;
}

void CorrelationPluginInstance::setDimensions(size_t numColumns, size_t numRows)
{
    Q_ASSERT(_dataColumnNames.empty());
    Q_ASSERT(_userColumnData.empty());
    Q_ASSERT(_userNodeData.empty());

    _numColumns = numColumns;
    _numRows = numRows;

    _dataColumnNames.resize(numColumns);
    _data.resize(numColumns * numRows);
}

void CorrelationPluginInstance::setDataColumnName(size_t column, const QString& name)
{
    Q_ASSERT(column < _numColumns);
    _dataColumnNames.at(column) = name;
}

void CorrelationPluginInstance::makeDataColumnNamesUnique()
{
    std::map<QString, std::vector<size_t>> dataColumnNameIndexes;

    while(dataColumnNameIndexes.size() < _dataColumnNames.size())
    {
        for(const auto& [name, indexes] : dataColumnNameIndexes)
        {
            for(size_t i = 1; i < indexes.size(); i++)
            {
                auto clashingIndex = indexes.at(i);
                auto newName = QStringLiteral("%1(%2)").arg(name).arg(i);
                setDataColumnName(clashingIndex, newName);
            }
        }

        dataColumnNameIndexes.clear();

        for(size_t i = 0; i < _dataColumnNames.size(); i++)
            dataColumnNameIndexes[_dataColumnNames.at(i)].push_back(i);
    }
}

void CorrelationPluginInstance::setData(size_t column, size_t row, double value)
{
    auto index = (row * _numColumns) + column;
    Q_ASSERT(index < _data.size());
    _data.at(index) = value;
}

void CorrelationPluginInstance::finishDataRow(size_t row)
{
    Q_ASSERT(row < _numRows);

    auto nodeId = graphModel()->mutableGraph().addNode();
    auto computeCost = static_cast<uint64_t>(_numRows - row + 1);

    _dataRows.emplace_back(_data, row, _numColumns, nodeId, computeCost);
    _userNodeData.setElementIdForIndex(nodeId, row);

    auto nodeName = _userNodeData.valueBy(nodeId, _userNodeData.firstUserDataVectorName()).toString();
    graphModel()->setNodeName(nodeId, nodeName);
}

void CorrelationPluginInstance::setNodeAttributeTableModelDataColumns()
{
    // When there is a very large number of data columns (where very large is fairly arbitrary),
    // don't add them to the table as it can't cope with it performance wise
    if(_dataColumnNames.size() > 1000)
        return;

    _nodeAttributeTableModel.addDataColumns(_dataColumnNames, &_data);
}

QStringList CorrelationPluginInstance::columnAnnotationNames() const
{
    QStringList list;
    list.reserve(static_cast<int>(_columnAnnotations.size()));

    for(const auto& columnAnnotation : _columnAnnotations)
        list.append(columnAnnotation.name());

    return list;
}

void CorrelationPluginInstance::onLoadSuccess()
{
    _userNodeData.exposeAsAttributes(*graphModel());
    buildColumnAnnotations();
    _nodeAttributeTableModel.updateColumnNames();
}

QVector<double> CorrelationPluginInstance::rawData()
{
    return QVector<double>(_data.begin(), _data.end());
}

void CorrelationPluginInstance::buildColumnAnnotations()
{
    _columnAnnotations.reserve(_userColumnData.numUserDataVectors());

    for(const auto& [name, values] : _userColumnData)
    {
        auto numValues = values.numValues();
        auto numUniqueValues = values.numUniqueValues();

        // If the number of unique values is more than a half of the total
        // number of values, skip it, since a large number of unique values
        // potentially causes performance problems, and it's probably not a
        // useful annotation in the first place
        if((numValues > 300) && (numUniqueValues * 2 > numValues))
            continue;

        _columnAnnotations.emplace_back(name, values.begin(), values.end());
    }

    emit columnAnnotationNamesChanged();
}

const ContinuousDataRow& CorrelationPluginInstance::dataRowForNodeId(NodeId nodeId) const
{
    return _dataRows.at(_userNodeData.indexFor(nodeId));
}

void CorrelationPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    _nodeAttributeTableModel.onSelectionChanged();
}

std::unique_ptr<IParser> CorrelationPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    std::vector<QString> urlTypes =
    {
        QStringLiteral("CorrelationCSV"),
        QStringLiteral("CorrelationTSV"),
        QStringLiteral("CorrelationXLSX")
    };

    if(u::contains(urlTypes, urlTypeName))
        return std::make_unique<CorrelationFileParser>(this, urlTypeName, _tabularData, _dataRect);

    return nullptr;
}

void CorrelationPluginInstance::applyParameter(const QString& name, const QVariant& value)
{
    if(name == QStringLiteral("minimumCorrelation"))
        _minimumCorrelationValue = value.toDouble();
    else if(name == QStringLiteral("initialThreshold"))
        _initialCorrelationThreshold = value.toDouble();
    else if(name == QStringLiteral("transpose"))
        _transpose = (value == QStringLiteral("true"));
    else if(name == QStringLiteral("correlationType"))
        _correlationType = static_cast<CorrelationType>(value.toInt());
    else if(name == QStringLiteral("correlationPolarity"))
        _correlationPolarity = static_cast<CorrelationPolarity>(value.toInt());
    else if(name == QStringLiteral("scaling"))
        _scalingType = static_cast<ScalingType>(value.toInt());
    else if(name == QStringLiteral("normalise"))
        _normaliseType = static_cast<NormaliseType>(value.toInt());
    else if(name == QStringLiteral("missingDataType"))
        _missingDataType = static_cast<MissingDataType>(value.toInt());
    else if(name == QStringLiteral("missingDataValue"))
        _missingDataReplacementValue = value.toDouble();
    else if(name == QStringLiteral("dataFrame"))
        _dataRect = value.toRect();
    else if(name == QStringLiteral("clusteringType"))
        _clusteringType = static_cast<ClusteringType>(value.toInt());
    else if(name == QStringLiteral("edgeReductionType"))
        _edgeReductionType = static_cast<EdgeReductionType>(value.toInt());
    else if(name == QStringLiteral("data") && value.canConvert<std::shared_ptr<TabularData>>())
        _tabularData = std::move(*value.value<std::shared_ptr<TabularData>>());
}

QStringList CorrelationPluginInstance::defaultTransforms() const
{
    QStringList defaultTransforms =
    {
        R"([pinned] "Remove Components" where $"Component Size" <= 1)"
    };

    auto correlationPolarity = static_cast<CorrelationPolarity>(_correlationPolarity);
    switch(correlationPolarity)
    {
    default:
    case CorrelationPolarity::Positive:
        defaultTransforms.append(
            QStringLiteral(R"("Remove Edges" where $"%1" < %2)")
            .arg(_correlationAttributeName)
            .arg(_initialCorrelationThreshold));
        break;

    case CorrelationPolarity::Negative:
        defaultTransforms.append(
            QStringLiteral(R"("Remove Edges" where $"%1" > %2)")
            .arg(_correlationAttributeName)
            .arg(-_initialCorrelationThreshold));
        break;

    case CorrelationPolarity::Both:
        defaultTransforms.append(
            QStringLiteral(R"("Remove Edges" where $"%1" < %2)")
            .arg(_correlationAbsAttributeName)
            .arg(_initialCorrelationThreshold));
        break;
    }

    if(_edgeReductionType == EdgeReductionType::KNN)
    {
        defaultTransforms.append(QStringLiteral(R"("k-NN" using $"%1")")
            .arg(correlationPolarity == CorrelationPolarity::Positive ?
            _correlationAttributeName : _correlationAbsAttributeName));
    }
    else if(_edgeReductionType == EdgeReductionType::PercentNN)
    {
        defaultTransforms.append(QStringLiteral(R"("%-NN" using $"%1")")
            .arg(correlationPolarity == CorrelationPolarity::Positive ?
            _correlationAttributeName : _correlationAbsAttributeName));
    }

    if(_clusteringType == ClusteringType::MCL)
        defaultTransforms.append(QStringLiteral(R"("MCL Cluster")"));
    else if(_clusteringType == ClusteringType::Louvain)
        defaultTransforms.append(QStringLiteral(R"("Louvain Cluster")"));

    return defaultTransforms;
}

QStringList CorrelationPluginInstance::defaultVisualisations() const
{
    if(_clusteringType == ClusteringType::MCL)
        return { R"("MCL Cluster" "Colour")" };

    if(_clusteringType == ClusteringType::Louvain)
        return { R"("Louvain Cluster" "Colour")" };

    return {};
}

size_t CorrelationPluginInstance::numColumns() const
{
    return _numColumns;
}

double CorrelationPluginInstance::dataAt(int row, int column) const
{
    return _data.at((row * _numColumns) + column);
}

QString CorrelationPluginInstance::rowName(int row) const
{
    const auto& [name, firstColumn] = *_userNodeData.begin();
    return firstColumn.get(row);
}

QString CorrelationPluginInstance::columnName(int column) const
{
    return _dataColumnNames.at(static_cast<size_t>(column));
}

QColor CorrelationPluginInstance::nodeColorForRow(int row) const
{
    auto nodeId = _userNodeData.elementIdForIndex(row);

    if(nodeId.isNull())
        return {};

    return graphModel()->nodeVisual(nodeId).outerColor();
}

QString CorrelationPluginInstance::attributeValueFor(const QString& attributeName, int row) const
{
    const auto* attribute = _graphModel->attributeByName(attributeName);
    auto nodeId = _userNodeData.elementIdForIndex(row);

    if(attribute == nullptr || nodeId.isNull())
        return {};

    return attribute->stringValueOf(nodeId);
}

QByteArray CorrelationPluginInstance::save(IMutableGraph& graph, Progressable& progressable) const
{
    json jsonObject;

    jsonObject["numColumns"] = static_cast<int>(_numColumns);
    jsonObject["numRows"] = static_cast<int>(_numRows);
    jsonObject["userNodeData"] = _userNodeData.save(graph, graph.nodeIds(), progressable);
    jsonObject["userColumnData"] =_userColumnData.save(progressable);
    jsonObject["dataColumnNames"] = jsonArrayFrom(_dataColumnNames, &progressable);

    graph.setPhase(QObject::tr("Data"));
    jsonObject["data"] = [&]
    {
        json array;

        uint64_t i = 0;
        for(const auto& nodeId : graph.nodeIds())
        {
            const auto& dataRow = dataRowForNodeId(nodeId);
            std::copy(dataRow.begin(), dataRow.end(), std::back_inserter(array));

            progressable.setProgress(static_cast<int>((i++) * 100 / graph.nodeIds().size()));
        }

        progressable.setProgress(-1);

        return array;
    }();

    graph.setPhase(QObject::tr("Correlation Values"));
    jsonObject["correlationValues"] = u::graphArrayAsJson(*_correlationValues, graph.edgeIds(), &progressable);

    jsonObject["minimumCorrelationValue"] = _minimumCorrelationValue;
    jsonObject["transpose"] = _transpose;
    jsonObject["correlationType"] = static_cast<int>(_correlationType);
    jsonObject["correlationPolarity"] = static_cast<int>(_correlationPolarity);
    jsonObject["scaling"] = static_cast<int>(_scalingType);
    jsonObject["normalisation"] = static_cast<int>(_normaliseType);
    jsonObject["missingDataType"] = static_cast<int>(_missingDataType);
    jsonObject["missingDataReplacementValue"] = _missingDataReplacementValue;

    return QByteArray::fromStdString(jsonObject.dump());
}

bool CorrelationPluginInstance::load(const QByteArray& data, int dataVersion, IMutableGraph& graph,
                                     IParser& parser)
{
    json jsonObject = parseJsonFrom(data, &parser);

    if(parser.cancelled())
        return false;

    if(jsonObject.is_null() || !jsonObject.is_object())
        return false;

    if(!u::contains(jsonObject, "numColumns") || !u::contains(jsonObject, "numRows"))
        return false;

    _numColumns = static_cast<size_t>(jsonObject["numColumns"].get<int>());
    _numRows = static_cast<size_t>(jsonObject["numRows"].get<int>());

    if(!u::contains(jsonObject, "userNodeData") || !u::contains(jsonObject, "userColumnData"))
        return false;

    if(!_userNodeData.load(jsonObject["userNodeData"], parser))
        return false;

    if(!_userColumnData.load(jsonObject["userColumnData"], parser))
        return false;

    parser.setProgress(-1);

    if(!u::contains(jsonObject, "dataColumnNames"))
        return false;

    const auto& dataColumnNames = jsonObject["dataColumnNames"];
    std::transform(dataColumnNames.begin(), dataColumnNames.end(), std::back_inserter(_dataColumnNames),
    [](const auto& dataColumnName)
    {
        return QString::fromStdString(dataColumnName);
    });

    uint64_t i = 0;

    if(!u::contains(jsonObject, "data"))
        return false;

    graph.setPhase(QObject::tr("Data"));
    const auto& jsonData = jsonObject["data"];
    for(const auto& value : jsonData)
    {
        _data.emplace_back(value);
        parser.setProgress(static_cast<int>((i++ * 100) / jsonData.size()));
    }

    parser.setProgress(-1);

    for(size_t row = 0; row < _numRows; row++)
    {
        auto nodeId = _userNodeData.elementIdForIndex(row);

        if(!nodeId.isNull())
            _dataRows.emplace_back(_data, row, _numColumns, nodeId);

        parser.setProgress(static_cast<int>((row * 100) / _numRows));
    }

    parser.setProgress(-1);

    const char* correlationValuesKey =
        dataVersion >= 3 ? "correlationValues" : "pearsonValues";

    if(!u::contains(jsonObject, correlationValuesKey))
    {
        setFailureReason(tr("Plugin data is missing '%1' key.").arg(correlationValuesKey));
        return false;
    }

    const auto& jsonCorrelationValues = jsonObject[correlationValuesKey];
    graph.setPhase(QObject::tr("Correlation Values"));
    i = 0;

    if(dataVersion >= 2)
    {
        u::forEachJsonGraphArray(jsonCorrelationValues, [&](EdgeId edgeId, double correlationValue)
        {
            Q_ASSERT(graph.containsEdgeId(edgeId));
            _correlationValues->set(edgeId, correlationValue);

            parser.setProgress(static_cast<int>((i++ * 100) / jsonCorrelationValues.size()));
        });
    }
    else
    {
        for(const auto& correlationValue : jsonCorrelationValues)
        {
            if(graph.containsEdgeId(i))
                _correlationValues->set(i, correlationValue);

            parser.setProgress(static_cast<int>((i++ * 100) / jsonCorrelationValues.size()));
        }
    }

    parser.setProgress(-1);

    if(!u::containsAllOf(jsonObject, {"minimumCorrelationValue", "transpose", "scaling",
        "normalisation", "missingDataType", "missingDataReplacementValue"}))
    {
        return false;
    }

    _minimumCorrelationValue = jsonObject["minimumCorrelationValue"];
    _transpose = jsonObject["transpose"];
    _scalingType = static_cast<ScalingType>(jsonObject["scaling"]);
    _normaliseType = static_cast<NormaliseType>(jsonObject["normalisation"]);
    _missingDataType = static_cast<MissingDataType>(jsonObject["missingDataType"]);
    _missingDataReplacementValue = jsonObject["missingDataReplacementValue"];

    if(dataVersion >= 3)
    {
        if(!u::contains(jsonObject, "correlationType") || !u::contains(jsonObject, "correlationPolarity"))
            return false;

        _correlationType = static_cast<CorrelationType>(jsonObject["correlationType"]);
        _correlationPolarity = static_cast<CorrelationPolarity>(jsonObject["correlationPolarity"]);
    }

    createAttributes();
    makeDataColumnNamesUnique();
    setNodeAttributeTableModelDataColumns();

    return true;
}

QString CorrelationPluginInstance::log() const
{
    QString text;

    text.append(tr("Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]")
        .arg(_dataRect.x()).arg(_dataRect.y()).arg(_dataRect.width()).arg(_dataRect.height()));

    if(_transpose)
        text.append(tr("\nTransposed"));

    text.append(tr("\nCorrelation Metric: "));
    switch(_correlationType)
    {
    default:
    case CorrelationType::Pearson: text.append(tr("Pearson")); break;
    case CorrelationType::SpearmanRank: text.append(tr("Spearman Rank")); break;
    }

    text.append(tr("\nCorrelation Polarity: "));
    switch(_correlationPolarity)
    {
    default:
    case CorrelationPolarity::Positive: text.append(tr("Positive")); break;
    case CorrelationPolarity::Negative: text.append(tr("Negative")); break;
    case CorrelationPolarity::Both: text.append(tr("Both")); break;
    }

    text.append(tr("\nMinimum Correlation Value: %1").arg(
        u::formatNumberScientific(_minimumCorrelationValue)));

    switch(_scalingType)
    {
    default:
    case ScalingType::None: break;
    case ScalingType::Log2: text.append(tr("\nScaling: Log2(x + ε)")); break;
    case ScalingType::Log10: text.append(tr("\nScaling: Log10(x + ε)")); break;
    case ScalingType::AntiLog2: text.append(tr("\nScaling: AntiLog2(x)")); break;
    case ScalingType::AntiLog10: text.append(tr("\nScaling: AntiLog10(x)")); break;
    case ScalingType::ArcSin: text.append(tr("\nScaling: Arcsin(x)")); break;
    }

    switch(_normaliseType)
    {
    default:
    case NormaliseType::None: break;
    case NormaliseType::MinMax: text.append(tr("\nNormalisation: Min/Max")); break;
    case NormaliseType::Quantile: text.append(tr("\nNormalisation: Quantile")); break;
    case NormaliseType::Mean: text.append(tr("\nNormalisation: Mean")); break;
    case NormaliseType::Standarisation: text.append(tr("\nNormalisation: Standarisation")); break;
    case NormaliseType::UnitScaling: text.append(tr("\nNormalisation: Unit Scaling")); break;
    }

    if(_imputedValues)
    {
        text.append(tr("\nImputation: "));
        switch(_missingDataType)
        {
        default:
        case MissingDataType::Constant: text.append(tr("Constant (%1)")
            .arg(u::formatNumberScientific(_missingDataReplacementValue))); break;
        case MissingDataType::ColumnAverage: text.append(tr("Column Mean")); break;
        case MissingDataType::RowInterpolation: text.append(tr("Row Interpolate")); break;
        }
    }

    return text;
}

CorrelationPlugin::CorrelationPlugin()
{
    registerUrlType(QStringLiteral("CorrelationCSV"), QObject::tr("Correlation CSV File"), QObject::tr("Correlation CSV Files"), {"csv"});
    registerUrlType(QStringLiteral("CorrelationTSV"), QObject::tr("Correlation TSV File"), QObject::tr("Correlation TSV Files"), {"tsv"});
    registerUrlType(QStringLiteral("CorrelationXLSX"), QObject::tr("Correlation Excel File"), QObject::tr("Correlation Excel Files"), {"xlsx"});

    qmlRegisterType<CorrelationPluginInstance>("app.graphia", 1, 0, "CorrelationPluginInstance");
    qmlRegisterType<CorrelationPlotItem>("app.graphia", 1, 0, "CorrelationPlot");
    qmlRegisterType<GraphSizeEstimatePlotItem>("app.graphia", 1, 0, "GraphSizeEstimatePlot");
    qmlRegisterType<CorrelationTabularDataParser>("app.graphia", 1, 0, "CorrelationTabularDataParser");
    qmlRegisterType<DataRectTableModel>("app.graphia", 1, 0, "DataRectTableModel");
}

static QString contentIdentityOf(const QUrl& url)
{
    if(XlsxTabularDataParser::canLoad(url))
        return QStringLiteral("CorrelationXLSX");

    QString identity;

    std::ifstream file(url.toLocalFile().toStdString());

    if(!file)
        return identity;

    const int maxLines = 50;
    int numLinesScanned = 0;

    std::istream* is = nullptr;
    do
    {
        std::string line;

        is = &u::getline(file, line);

        size_t numCommas = 0;
        size_t numTabs = 0;
        bool inQuotes = false;

        for(auto character : line)
        {
            switch(character)
            {
            case '"': inQuotes = !inQuotes; break;
            case ',': if(!inQuotes) { numCommas++; } break;
            case '\t': if(!inQuotes) { numTabs++; } break;
            default: break;
            }
        }

        if(numTabs > numCommas)
            identity = QStringLiteral("CorrelationTSV");
        else if(numCommas > numTabs)
            identity = QStringLiteral("CorrelationCSV");

        numLinesScanned++;
    } while(identity.isEmpty() &&
        !is->fail() && !is->eof() &&
        numLinesScanned < maxLines);

    return identity;
}

QStringList CorrelationPlugin::identifyUrl(const QUrl& url) const
{
    auto urlTypes = identifyByExtension(url);

    if(urlTypes.isEmpty() || contentIdentityOf(url) != urlTypes.first())
        return {};

    return urlTypes;
}

QString CorrelationPlugin::failureReason(const QUrl& url) const
{
    auto urlTypes = identifyByExtension(url);

    if(urlTypes.isEmpty())
        return {};

    auto extensionIdentity = urlTypes.first();
    auto contentIdentity = contentIdentityOf(url);

    if(contentIdentity.isEmpty())
    {
        return tr("%1 has an extension that indicates it is a '%2', "
            "however its content could not be determined.").arg(url.fileName(),
            individualDescriptionForUrlTypeName(extensionIdentity));
    }

    if(extensionIdentity != contentIdentity)
    {
        return tr("%1 has an extension that indicates it is a '%2', "
            "however its content resembles a '%3'.").arg(url.fileName(),
            individualDescriptionForUrlTypeName(extensionIdentity),
            individualDescriptionForUrlTypeName(contentIdentity));
    }

    return {};
}
