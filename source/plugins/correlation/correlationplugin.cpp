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

#include "correlationplugin.h"

#include "correlation.h"
#include "correlationplotitem.h"
#include "graphsizeestimateplotitem.h"

#include "importannotationscommand.h"
#include "importannotationskeydetection.h"

#include "hierarchicalclusteringcommand.h"

#include "shared/graph/grapharray_json.h"

#include "shared/utils/threadpool.h"
#include "shared/utils/iterator_range.h"
#include "shared/utils/container.h"
#include "shared/utils/random.h"
#include "shared/utils/string.h"
#include "shared/utils/redirects.h"
#include "shared/utils/source_location.h"

#include "shared/attributes/iattribute.h"

#include "shared/ui/visualisations/ielementvisual.h"

#include "shared/loading/xlsxtabulardataparser.h"

#include <json_helper.h>

#include <map>

using namespace Qt::Literals::StringLiterals;

CorrelationPluginInstance::CorrelationPluginInstance()
{
    connect(this, SIGNAL(loadSuccess()), this, SLOT(onLoadSuccess()));
    connect(this, SIGNAL(selectionChanged(const ISelectionManager*)),
        this, SLOT(onSelectionChanged(const ISelectionManager*)), Qt::DirectConnection);
}

void CorrelationPluginInstance::initialise(const IPlugin* plugin, IDocument* document,
                                           const IParserThread* parserThread)
{
    BasePluginInstance::initialise(plugin, document, parserThread);

    _graphModel = document->graphModel();
    _nodeAttributeTableModel.initialise(document, &_graphModel->userNodeData());

    _correlationValues = std::make_unique<EdgeArray<double>>(_graphModel->mutableGraph());

    connect(this, &BasePluginInstance::attributesChanged,
        this, &CorrelationPluginInstance::sharedValuesAttributeNamesChanged);
    connect(this, &BasePluginInstance::attributesChanged,
        this, &CorrelationPluginInstance::numericalAttributeNamesChanged);
}

bool CorrelationPluginInstance::loadUserData(const TabularData& tabularData,
    const QRect& dataRect, IParser& parser)
{
    if(dataRect.x() == 0 || dataRect.y() == 0)
    {
        qDebug() << "tabularData has no row or column names!";
        return false;
    }

    // The parser may have updated the dataRect, so update
    // ours too so that the provenance log is consistent
    _dataRect = dataRect;

    parser.setProgress(-1);

    const uint64_t numDataPoints = static_cast<uint64_t>(tabularData.numColumns()) * tabularData.numRows();
    const size_t numColumns = _numContinuousColumns + _numDiscreteColumns;
    auto left = static_cast<size_t>(dataRect.x());
    auto right = static_cast<size_t>(dataRect.x()) + static_cast<size_t>(dataRect.width());
    auto top = static_cast<size_t>(dataRect.y());
    auto bottom = static_cast<size_t>(dataRect.y()) + static_cast<size_t>(dataRect.height());

    // Do a pass over the non-data column names, to ensure they're unique
    std::vector<QString> rowAttributeColumnNames(left);
    for(size_t columnIndex = 0; columnIndex < left; columnIndex++)
    {
        auto columnName = tabularData.valueAt(columnIndex, 0);
        columnName = u::findUniqueName(rowAttributeColumnNames, columnName);
        rowAttributeColumnNames[columnIndex] = columnName;
    }

    for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            if(parser.cancelled())
                return false;

            const uint64_t rowOffset = static_cast<uint64_t>(rowIndex) * tabularData.numColumns();
            const uint64_t dataPoint = columnIndex + rowOffset;
            parser.setProgress(static_cast<int>((dataPoint * 100) / numDataPoints));

            const auto& value = tabularData.valueAt(columnIndex, rowIndex);

            //FIXME: If there are continuous and discrete columns, dataColumnIndex will need to change
            const size_t dataColumnIndex = columnIndex - static_cast<size_t>(dataRect.x());
            const size_t dataRowIndex = rowIndex - static_cast<size_t>(dataRect.y());
            const bool isColumnInDataRect = left <= columnIndex && columnIndex < right;
            const bool isRowInDataRect = top <= rowIndex && rowIndex < bottom;
            const bool isColumnAnnotation = rowIndex < top;
            const bool isRowAttribute = columnIndex < left;

            if((isColumnInDataRect && dataColumnIndex >= numColumns) ||
                (isRowInDataRect && dataRowIndex >= _numRows))
            {
                qDebug() << QString("WARNING: Attempting to set data at coordinate (%1, %2) in "
                                    "dataRect of dimensions (%3, %4)")
                    .arg(dataColumnIndex).arg(dataRowIndex)
                    .arg(numColumns).arg(_numRows);

                continue;
            }

            if(rowIndex == 0)
            {
                if(isColumnInDataRect)
                    setDataColumnName(dataColumnIndex, value);
                else if(isRowAttribute)
                    _graphModel->userNodeData().add(value);
            }
            else if(isColumnAnnotation)
            {
                if(columnIndex == 0)
                    _userColumnData.add(value);
                else if(isColumnInDataRect)
                    _userColumnData.setValue(dataColumnIndex, tabularData.valueAt(0, rowIndex), value);
            }
            else if(isColumnInDataRect && isRowInDataRect)
            {
                switch(_correlationDataType)
                {
                default:
                case CorrelationDataType::Continuous:
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
                        _valuesWereImputed = true;
                    }

                    auto index = (dataRowIndex * _numContinuousColumns) + dataColumnIndex;
                    Q_ASSERT(index < _continuousData.size());
                    _continuousData.at(index) = transformedValue;
                    break;
                }

                case CorrelationDataType::Discrete:
                {
                    auto index = (dataRowIndex * _numDiscreteColumns) + dataColumnIndex;
                    Q_ASSERT(index < _discreteData.size());
                    _discreteData.at(index) = value;
                    break;
                }
                }
            }
            else if(isRowAttribute)
                _graphModel->userNodeData().setValue(dataRowIndex, rowAttributeColumnNames.at(columnIndex), value);
        }
    }

    parser.setProgress(-1);

    CorrelationFileParser::clipValues(_clippingType, _clippingValue,
        static_cast<size_t>(dataRect.width()), _continuousData);

    _continuousEpsilon = CorrelationFileParser::epsilonFor(_continuousData);
    std::transform(_continuousData.begin(), _continuousData.end(), _continuousData.begin(),
    [this](double value)
    {
        return CorrelationFileParser::scaleValue(_scalingType, value, _continuousEpsilon);
    });

    buildDiscreteDataValueIndex(parser);
    makeDataColumnNamesUnique();
    setNodeAttributeTableModelDataColumns();

    return true;
}

void CorrelationPluginInstance::normalise(IParser* parser)
{
    CorrelationFileParser::normalise(_normaliseType, _continuousDataRows, parser);

    // Normalising obviously changes all the values in _continuousDataRows, so we
    // must sync _data up so that it matches
    _continuousData.clear();
    for(const auto& dataRow : _continuousDataRows)
        _continuousData.insert(_continuousData.end(), dataRow.begin(), dataRow.end());
}

void CorrelationPluginInstance::finishDataRows()
{
    for(size_t row = 0; row < _numRows; row++)
        finishDataRow(row);

    for(auto& continuousDataRow : _continuousDataRows)
        continuousDataRow.update();

    for(auto& discreteDataRow : _discreteDataRows)
        discreteDataRow.update();
}

void CorrelationPluginInstance::createAttributes()
{
    size_t columnIndex = 0;
    std::map<QString, size_t> dataValueAttributeColumn;

    for(const auto& dataColumnName : _dataColumnNames)
        dataValueAttributeColumn[dataColumnName] = columnIndex++;

    auto& dataValueAttribute = graphModel()->createAttribute(tr("Data Value"))
        .setUserDefined(true)
        .setDescription(tr("The Data Value is a parameterised attribute that permits referencing "
            "a specific column in the correlated data."))
        .setValidParameterValues(u::toQStringList(_dataColumnNames));

    QString correlationAttributeDescription;

    auto correlationDataType = normaliseQmlEnum<CorrelationDataType>(_correlationDataType);
    switch(correlationDataType)
    {
    default:
    case CorrelationDataType::Continuous:
    {
        dataValueAttribute
            .setFloatValueFn([this, dataValueAttributeColumn](NodeId nodeId, const IAttribute& attribute)
            {
                auto columnName = attribute.parameterValue();
                if(columnName.isEmpty())
                    return 0.0;

                auto column = dataValueAttributeColumn.at(columnName);
                return continuousDataRowForNodeId(nodeId).valueAt(column);
            })
            .setFlag(AttributeFlag::AutoRange);

        graphModel()->createAttribute(tr("Strongest Data Column"))
            .setStringValueFn([this](NodeId nodeId)
            {
                const auto& dataRow = continuousDataRowForNodeId(nodeId);
                return _dataColumnNames.at(dataRow.largestColumnIndex());
            })
            .setFlag(AttributeFlag::FindShared)
            .setFlag(AttributeFlag::Searchable)
            .setDescription(tr("The name of the data column which has the largest absolute value for "
                "the data row associated with the node."));

        graphModel()->createAttribute(tr("Mean Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return continuousDataRowForNodeId(nodeId).mean(); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr("The Mean Data Value is the mean of the values associated "
                "with the node."));

        graphModel()->createAttribute(tr("Minimum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return continuousDataRowForNodeId(nodeId).minValue(); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr("The Minimum Data Value is the minimum value associated "
                "with the node."));

        graphModel()->createAttribute(tr("Maximum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return continuousDataRowForNodeId(nodeId).maxValue(); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr("The Maximum Data Value is the maximum value associated "
                "with the node."));

        graphModel()->createAttribute(tr("Variance"))
            .setFloatValueFn([this](NodeId nodeId) { return continuousDataRowForNodeId(nodeId).variance(); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr("The %1 is a measure of the spread of the values associated with the node. "
                "It is defined as ∑(<i>x</i>-µ)², where <i>x</i> is the value and µ is the mean.")
                .arg(u::redirectLink("variance", tr("Variance"))));

        graphModel()->createAttribute(tr("Standard Deviation"))
            .setFloatValueFn([this](NodeId nodeId) { return continuousDataRowForNodeId(nodeId).stddev(); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr("The %1 is a measure of the spread of the values associated "
                "with the node. It is defined as √∑(<i>x</i>-µ)², where <i>x</i> is the value "
                "and µ is the mean.").arg(u::redirectLink("stddev", tr("Standard Deviation"))));

        graphModel()->createAttribute(tr("Coefficient of Variation"))
            .setFloatValueFn([this](NodeId nodeId) { return continuousDataRowForNodeId(nodeId).coefVar(); })
            .setValueMissingFn([this](NodeId nodeId) { return std::isnan(continuousDataRowForNodeId(nodeId).coefVar()); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr("The %1 is a measure of the spread of the values associated "
                "with the node. It is defined as the standard deviation divided by the mean.")
                .arg(u::redirectLink("coef_variation", tr("Coefficient of Variation"))));

        auto continuousCorrelation = ContinuousCorrelation::create(_continuousCorrelationType, _correlationFilterType);
        _correlationAttributeName = continuousCorrelation->attributeName();
        correlationAttributeDescription = continuousCorrelation->attributeDescription();
        break;
    }

    case CorrelationDataType::Discrete:
    {
        dataValueAttribute
            .setStringValueFn([this, dataValueAttributeColumn](NodeId nodeId, const IAttribute& attribute)
            {
                auto columnName = attribute.parameterValue();
                if(columnName.isEmpty())
                    return QString();

                auto column = dataValueAttributeColumn.at(columnName);
                return discreteDataRowForNodeId(nodeId).valueAt(column);
            });

        auto discreteCorrelation = DiscreteCorrelation::create(_discreteCorrelationType, _correlationFilterType);
        _correlationAttributeName = discreteCorrelation->attributeName();
        correlationAttributeDescription = discreteCorrelation->attributeDescription();
        break;
    }
    }

    graphModel()->createAttribute(_correlationAttributeName)
        .setFloatValueFn([this](EdgeId edgeId) { return _correlationValues->get(edgeId); })
        .setFlag(AttributeFlag::AutoRange)
        .setDescription(correlationAttributeDescription);

    auto correlationPolarity = normaliseQmlEnum<CorrelationPolarity>(_correlationPolarity);
    switch(correlationPolarity)
    {
    case CorrelationPolarity::Negative:
    case CorrelationPolarity::Both:
        _correlationAbsAttributeName = tr("Absolute ") + _correlationAttributeName;

        graphModel()->createAttribute(_correlationAbsAttributeName)
            .setFloatValueFn([this](EdgeId edgeId) { return std::abs(_correlationValues->get(edgeId)); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(correlationAttributeDescription);
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
        auto nodeId = _graphModel->userNodeData().elementIdForIndex(static_cast<size_t>(row));
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

EdgeList CorrelationPluginInstance::correlation(IParser& parser)
{
    auto correlationDataType = normaliseQmlEnum<CorrelationDataType>(_correlationDataType);
    switch(correlationDataType)
    {
    default:
    case CorrelationDataType::Continuous:
    {
        auto continuousCorrelation = ContinuousCorrelation::create(_continuousCorrelationType, _correlationFilterType);
        return continuousCorrelation->edgeList(_continuousDataRows,
            {
                {u"minimumThreshold"_s, _minimumThreshold},
                {u"maximumK"_s, static_cast<uint>(_maximumK)},
                {u"correlationPolarity"_s, static_cast<int>(_correlationPolarity)}
            }, &parser, &parser);
    }

    case CorrelationDataType::Discrete:
    {
        auto discreteCorrelation = DiscreteCorrelation::create(_discreteCorrelationType, _correlationFilterType);
        return discreteCorrelation->edgeList(_discreteDataRows,
            {
                {u"minimumThreshold"_s, _minimumThreshold},
                {u"maximumK"_s, static_cast<uint>(_maximumK)},
                {u"treatAsBinary"_s, _treatAsBinary}
            }, &parser, &parser);
    }
    }

    return {};
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

void CorrelationPluginInstance::setDimensions(size_t numContinuousColumns, size_t numDiscreteColumns, size_t numRows)
{
    Q_ASSERT(_dataColumnNames.empty());
    Q_ASSERT(_userColumnData.empty());
    Q_ASSERT(_graphModel->userNodeData().numUserDataVectors() == 0);

    _numContinuousColumns = numContinuousColumns;
    _numDiscreteColumns = numDiscreteColumns;
    _numRows = numRows;

    emit numColumnsChanged();

    _dataColumnNames.resize(numContinuousColumns + numDiscreteColumns);
    _continuousData.resize(numContinuousColumns * numRows);
    _discreteData.resize(numDiscreteColumns * numRows);
}

void CorrelationPluginInstance::setDataColumnName(size_t column, const QString& name)
{
    Q_ASSERT(column < (_numContinuousColumns + _numDiscreteColumns));
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
                auto newName = u"%1(%2)"_s.arg(name).arg(i);
                setDataColumnName(clashingIndex, newName);
            }
        }

        dataColumnNameIndexes.clear();

        for(size_t i = 0; i < _dataColumnNames.size(); i++)
            dataColumnNameIndexes[_dataColumnNames.at(i)].push_back(i);
    }
}

void CorrelationPluginInstance::finishDataRow(size_t row)
{
    Q_ASSERT(row < _numRows);

    auto nodeId = graphModel()->mutableGraph().addNode();
    auto computeCost = static_cast<uint64_t>(_numRows - row + 1);

    _continuousDataRows.emplace_back(_continuousData, row, _numContinuousColumns, nodeId, computeCost);
    _discreteDataRows.emplace_back(_discreteData, row, _numDiscreteColumns, nodeId, computeCost);

    _graphModel->userNodeData().setElementIdForIndex(nodeId, row);

    auto nodeName = _graphModel->userNodeData().valueBy(nodeId,
        _graphModel->userNodeData().firstVectorName()).toString();
    graphModel()->setNodeName(nodeId, nodeName);
}

void CorrelationPluginInstance::setNodeAttributeTableModelDataColumns()
{
    // When there is a very large number of data columns (where very large is fairly arbitrary),
    // don't add them to the table as it can't cope with it performance wise
    if(_dataColumnNames.size() > 1000)
        return;

    switch(_correlationDataType)
    {
    default:
    case CorrelationDataType::Continuous:
        _nodeAttributeTableModel.addContinuousDataColumns(_dataColumnNames, &_continuousData);
        break;

    case CorrelationDataType::Discrete:
        _nodeAttributeTableModel.addDiscreteDataColumns(_dataColumnNames, &_discreteData);
        break;
    }
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
    rebuildColumnAnnotations();
    _nodeAttributeTableModel.updateColumnNames();
}

void CorrelationPluginInstance::rebuildColumnAnnotations()
{
    std::vector<ColumnAnnotation> columnAnnotations;
    columnAnnotations.reserve(_userColumnData.numUserDataVectors());

    for(const auto& name : _userColumnData)
    {
        const auto* values = _userColumnData.vector(name);
        columnAnnotations.emplace_back(name, values->begin(), values->end());
    }

    auto changed = columnAnnotations != _columnAnnotations;

    auto before = columnAnnotationNames();
    _columnAnnotations = columnAnnotations;
    auto after = columnAnnotationNames();

    const QStringList addedNames = u::toQStringList(u::setDifference(after, before));
    const QStringList removedNames = u::toQStringList(u::setDifference(before, after));

    emit columnAnnotationNamesChanged(addedNames, removedNames);

    if(changed)
        emit columnAnnotationValuesChanged();
}

void CorrelationPluginInstance::buildDiscreteDataValueIndex(Progressable& progressable)
{
    if(_correlationDataType != CorrelationDataType::Discrete)
        return;

    size_t dataValueIndex = 0;

    for(size_t columnIndex = 0; columnIndex < _numDiscreteColumns; columnIndex++)
    {
        std::set<QString> values;

        for(size_t rowIndex = 0; rowIndex < _numRows; rowIndex++)
            values.emplace(discreteDataAt(rowIndex, columnIndex));

        std::vector<QString> sortedValues;
        sortedValues.reserve(values.size());
        std::copy(values.begin(), values.end(), std::back_inserter(sortedValues));
        std::sort(sortedValues.begin(), sortedValues.end());

        for(const auto& sortedValue : sortedValues)
        {
            if(!u::contains(_discreteDataValueIndex, sortedValue))
                _discreteDataValueIndex[sortedValue] = dataValueIndex++;
        }

        progressable.setProgress(static_cast<int>((columnIndex * 100) / _numDiscreteColumns));
    }
}

const ContinuousDataVector& CorrelationPluginInstance::continuousDataRowForNodeId(NodeId nodeId) const
{
    return _continuousDataRows.at(_graphModel->userNodeData().indexFor(nodeId));
}

const DiscreteDataVector& CorrelationPluginInstance::discreteDataRowForNodeId(NodeId nodeId) const
{
    return _discreteDataRows.at(_graphModel->userNodeData().indexFor(nodeId));
}

void CorrelationPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    _nodeAttributeTableModel.onSelectionChanged();
}

std::unique_ptr<IParser> CorrelationPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    const std::vector<QString> urlTypes =
    {
        u"CorrelationCSV"_s,
        u"CorrelationTSV"_s,
        u"CorrelationSSV"_s,
        u"CorrelationXLSX"_s
    };

    if(u::contains(urlTypes, urlTypeName))
        return std::make_unique<CorrelationFileParser>(this, urlTypeName, _tabularData, _dataRect);

    return nullptr;
}

void CorrelationPluginInstance::applyParameter(const QString& name, const QVariant& value)
{
    if(name == u"minimumThreshold"_s)
        _minimumThreshold = value.toDouble();
    else if(name == u"initialThreshold"_s)
        _initialThreshold = value.toDouble();
    else if(name == u"maximumK"_s)
        _maximumK = static_cast<size_t>(value.toUInt());
    else if(name == u"initialK"_s)
        _initialK = static_cast<size_t>(value.toUInt());
    else if(name == u"transpose"_s)
        _transpose = (value == u"true"_s);
    else if(name == u"correlationFilterType"_s)
        _correlationFilterType = qmlEnumFor<CorrelationFilterType>(value);
    else if(name == u"correlationDataType"_s)
        _correlationDataType = qmlEnumFor<CorrelationDataType>(value);
    else if(name == u"continuousCorrelationType"_s)
        _continuousCorrelationType = qmlEnumFor<CorrelationType>(value);
    else if(name == u"discreteCorrelationType"_s)
        _discreteCorrelationType = qmlEnumFor<CorrelationType>(value);
    else if(name == u"correlationPolarity"_s)
        _correlationPolarity = qmlEnumFor<CorrelationPolarity>(value);
    else if(name == u"scaling"_s)
        _scalingType = qmlEnumFor<ScalingType>(value);
    else if(name == u"normalise"_s)
        _normaliseType = qmlEnumFor<NormaliseType>(value);
    else if(name == u"missingDataType"_s)
        _missingDataType = qmlEnumFor<MissingDataType>(value);
    else if(name == u"missingDataValue"_s)
        _missingDataReplacementValue = value.toDouble();
    else if(name == u"clippingType"_s)
        _clippingType = qmlEnumFor<ClippingType>(value);
    else if(name == u"clippingValue"_s)
        _clippingValue = value.toDouble();
    else if(name == u"treatAsBinary"_s)
        _treatAsBinary = value.toBool();
    else if(name == u"dataRect"_s)
    {
        if(value.canConvert<QVariantMap>())
        {
            // This happens when the parameters are specified in headless mode
            auto m = value.value<QVariantMap>();

            if(m.contains(u"x"_s))         _dataRect.setX(m.value(u"x"_s).toInt());
            if(m.contains(u"left"_s))      _dataRect.setLeft(m.value(u"left"_s).toInt());
            if(m.contains(u"right"_s))     _dataRect.setRight(m.value(u"right"_s).toInt());
            if(m.contains(u"y"_s))         _dataRect.setY(m.value(u"y"_s).toInt());
            if(m.contains(u"top"_s))       _dataRect.setTop(m.value(u"top"_s).toInt());
            if(m.contains(u"bottom"_s))    _dataRect.setBottom(m.value(u"bottom"_s).toInt());
            if(m.contains(u"width"_s))     _dataRect.setWidth(m.value(u"width"_s).toInt());
            if(m.contains(u"height"_s))    _dataRect.setHeight(m.value(u"height"_s).toInt());
        }
        else
            _dataRect = value.toRect();
    }
    else if(name == u"additionalTransforms"_s)
        _additionalTransforms = value.toStringList();
    else if(name == u"additionalVisualisations"_s)
        _additionalVisualisations = value.toStringList();
    else if(name == u"data"_s && value.canConvert<std::shared_ptr<TabularData>>())
        _tabularData = std::move(*value.value<std::shared_ptr<TabularData>>());
    else
        qDebug() << "CorrelationPluginInstance::applyParameter unknown parameter" << name << value;
}

QStringList CorrelationPluginInstance::defaultTransforms() const
{
    QStringList defaultTransforms =
    {
        R"([pinned] "Remove Components" where $"Component Size" <= 1)"
    };

    auto correlationFilterType = normaliseQmlEnum<CorrelationFilterType>(_correlationFilterType);
    auto correlationPolarity = normaliseQmlEnum<CorrelationPolarity>(_correlationPolarity);

    const char* filterOperator = nullptr;
    const QString* filterAttribute = nullptr;

    switch(normaliseQmlEnum<CorrelationPolarity>(_correlationPolarity))
    {
    default:
    case CorrelationPolarity::Positive:
        filterOperator = "<";
        filterAttribute = &_correlationAttributeName;
        break;

    case CorrelationPolarity::Negative:
        filterOperator = ">";
        filterAttribute = &_correlationAttributeName;
        break;

    case CorrelationPolarity::Both:
        filterOperator = "<";
        filterAttribute = &_correlationAbsAttributeName;
        break;
    }

    switch(correlationFilterType)
    {
    case CorrelationFilterType::Threshold:
    {
        defaultTransforms.append(
            QStringLiteral(R"("Remove Edges" where $"%1" %2 %3)")
            .arg(*filterAttribute, filterOperator)
            .arg(_initialThreshold));

        break;
    }

    case CorrelationFilterType::Knn:
    {
        defaultTransforms.append(
            QStringLiteral(R"("k-NN" using $"%1" with "k" = %2)")
            .arg(correlationPolarity == CorrelationPolarity::Positive ?
            _correlationAttributeName : _correlationAbsAttributeName)
            .arg(_initialK));

        defaultTransforms.append(
            QStringLiteral(R"("Remove Edges" where $"%1" %2 %3)")
            .arg(*filterAttribute, filterOperator)
            .arg(_minimumThreshold));

        break;
    }

    default: break;
    }

    defaultTransforms.reserve(defaultTransforms.size() + _additionalTransforms.size());

    for(const auto& additionalTransform : _additionalTransforms)
        defaultTransforms.append(additionalTransform);

    return defaultTransforms;
}

QStringList CorrelationPluginInstance::defaultVisualisations() const
{
    QStringList defaultVisualisations;
    defaultVisualisations.reserve(_additionalVisualisations.size());

    for(const auto& additionalVisualisation : _additionalVisualisations)
        defaultVisualisations.append(additionalVisualisation);

    return defaultVisualisations;
}

double CorrelationPluginInstance::continuousDataAt(size_t row, size_t column) const
{
    return _continuousData.at((row * _numContinuousColumns) + column);
}

QString CorrelationPluginInstance::discreteDataAt(size_t row, size_t column) const
{
    return _discreteData.at((row * _numDiscreteColumns) + column);
}

int CorrelationPluginInstance::discreteDataValueIndex(const QString& value) const
{
    if(!u::contains(_discreteDataValueIndex, value) || _discreteDataValueIndex.empty())
        return -1;

    return static_cast<int>(_discreteDataValueIndex.at(value));
}

QString CorrelationPluginInstance::rowName(size_t row) const
{
    return _graphModel->userNodeData().value(row,
        _graphModel->userNodeData().firstVectorName()).toString();
}

QString CorrelationPluginInstance::columnName(size_t column) const
{
    return _dataColumnNames.at(static_cast<size_t>(column));
}

QColor CorrelationPluginInstance::nodeColorForRow(size_t row) const
{
    auto nodeId = _graphModel->userNodeData().elementIdForIndex(row);

    if(nodeId.isNull())
        return {};

    return graphModel()->nodeVisual(nodeId).outerColor();
}

QColor CorrelationPluginInstance::nodeColorForRows(const std::vector<size_t>& rows) const
{
    if(rows.empty())
        return {};

    auto color = nodeColorForRow(rows.at(0));

    auto colorsInconsistent = std::any_of(rows.begin(), rows.end(),
    [this, &color](auto row)
    {
        return nodeColorForRow(row) != color;
    });

    if(colorsInconsistent)
    {
        // The colours are not consistent, so just use black
        color = QCustomPlotColorProvider::penColor();
    }

    return color;
}

const ColumnAnnotation* CorrelationPluginInstance::columnAnnotationByName(const QString& name) const
{
    auto it = std::find_if(_columnAnnotations.begin(), _columnAnnotations.end(),
        [&name](const auto& v) { return v.name() == name; });

    if(it != _columnAnnotations.end())
        return &(*it);

    return nullptr;
}

std::vector<size_t> CorrelationPluginInstance::rowsForGraph() const
{
    std::vector<size_t> rows;
    rows.reserve(_graphModel->graph().numNodes());

    for(auto nodeId : _graphModel->graph().nodeIds())
        rows.push_back(_graphModel->userNodeData().indexFor(nodeId));

    return rows;
}

QString CorrelationPluginInstance::attributeValueFor(const QString& attributeName, size_t row) const
{
    const auto* attribute = _graphModel->attributeByName(attributeName);
    auto nodeId = _graphModel->userNodeData().elementIdForIndex(row);

    if(attribute == nullptr || nodeId.isNull())
        return {};

    return attribute->stringValueOf(nodeId);
}

void CorrelationPluginInstance::computeHierarchicalClustering()
{
    if(_continuousHcOrder.empty())
    {
        commandManager()->execute(ExecutePolicy::Once, std::make_unique<HierarchicalClusteringCommand>(
            _continuousData, _numContinuousColumns, _numRows, *this));
    }
    else
        emit hierarchicalClusteringComplete();
}

void CorrelationPluginInstance::setHcOrdering(const std::vector<size_t>& ordering)
{
    _continuousHcOrder = ordering;
    emit hierarchicalClusteringComplete();
}

size_t CorrelationPluginInstance::hcColumn(size_t column) const
{
    if(column >= _continuousHcOrder.size())
        return 0;

    return _continuousHcOrder.at(column);
}

std::vector<int> CorrelationPluginInstance::rowsOfInterestByColumns(const std::vector<int>& columns,
    const std::vector<int>& rows, int percentile, double weight)
{
    if(rows.empty())
        return {};

    const auto positiveWeight = weight > 0.0 ? (1.0 + weight) : 1.0;
    const auto negativeWeight = weight < 0.0 ? (1.0 - weight) : 1.0;

    struct RowScore
    {
        int _row = 0;
        double _value = 0.0;
    };

    std::vector<RowScore> rowScores;
    rowScores.reserve(rows.size());

    for(auto row : rows)
    {
        const auto& dataRow = _continuousDataRows.at(static_cast<size_t>(row));

        double columnsSum = 0.0;
        for(auto column : columns)
            columnsSum += dataRow.valueAt(static_cast<size_t>(column));

        const double otherColumnsSum = dataRow.sum() - columnsSum;

        const RowScore rowScore
        {
            row,
            (columnsSum * positiveWeight) - (otherColumnsSum * negativeWeight)
        };

        rowScores.push_back(rowScore);
    }

    std::sort(rowScores.begin(), rowScores.end(), [](const auto& a, const auto& b)
    {
        return a._value > b._value;
    });

    auto rowsPercentile = std::max(static_cast<size_t>(1),
        (static_cast<size_t>(percentile) * rows.size()) / 100);

    rowScores.resize(rowsPercentile);

    std::vector<int> outRows;
    outRows.reserve(rowScores.size());
    for(const auto& rowMean : rowScores)
        outRows.push_back(rowMean._row);

    return outRows;
}

QByteArray CorrelationPluginInstance::save(IMutableGraph& graph, Progressable& progressable) const
{
    json jsonObject;

    jsonObject["numContinuousColumns"] = static_cast<int>(_numContinuousColumns);
    jsonObject["numDiscreteColumns"] = static_cast<int>(_numDiscreteColumns);
    jsonObject["userColumnData"] =_userColumnData.save(progressable);
    jsonObject["dataColumnNames"] = jsonArrayFrom(_dataColumnNames, &progressable);

    progressable.setPhase(QObject::tr("Data"));
    jsonObject["continuousData"] = [&]
    {
        json array;

        uint64_t i = 0;
        for(const auto& nodeId : graph.nodeIds())
        {
            const auto& dataRow = continuousDataRowForNodeId(nodeId);
            std::copy(dataRow.begin(), dataRow.end(), std::back_inserter(array));

            progressable.setProgress(static_cast<int>((i++) * 100 / graph.nodeIds().size()));
        }

        progressable.setProgress(-1);
        Q_ASSERT(_numContinuousColumns == 0 || (array.size() % _numContinuousColumns) == 0);

        return array;
    }();

    jsonObject["discreteData"] = [&]
    {
        json array;

        uint64_t i = 0;
        for(const auto& nodeId : graph.nodeIds())
        {
            const auto& dataRow = discreteDataRowForNodeId(nodeId);
            std::copy(dataRow.begin(), dataRow.end(), std::back_inserter(array));

            progressable.setProgress(static_cast<int>((i++) * 100 / graph.nodeIds().size()));
        }

        progressable.setProgress(-1);
        Q_ASSERT(_numDiscreteColumns == 0 || (array.size() % _numDiscreteColumns) == 0);

        return array;
    }();

    jsonObject["hcOrdering"] = [&]
    {
        json array = json::array();

        uint64_t i = 0;
        for(const auto& index : _continuousHcOrder)
        {
            array.push_back(index);
            progressable.setProgress(static_cast<int>((i++) * 100 / _continuousHcOrder.size()));
        }

        progressable.setProgress(-1);

        return array;
    }();

    progressable.setPhase(QObject::tr("Correlation Values"));
    jsonObject["correlationValues"] = u::graphArrayAsJson(*_correlationValues, graph.edgeIds(), &progressable);

    jsonObject["minimumThreshold"] = _minimumThreshold;
    jsonObject["maximumK"] = _maximumK;
    jsonObject["transpose"] = _transpose;
    jsonObject["correlationDataType"] = static_cast<int>(_correlationDataType);
    jsonObject["continuousCorrelationType"] = static_cast<int>(_continuousCorrelationType);
    jsonObject["discreteCorrelationType"] = static_cast<int>(_discreteCorrelationType);
    jsonObject["correlationPolarity"] = static_cast<int>(_correlationPolarity);
    jsonObject["scaling"] = static_cast<int>(_scalingType);
    jsonObject["normalisation"] = static_cast<int>(_normaliseType);
    jsonObject["missingDataType"] = static_cast<int>(_missingDataType);
    jsonObject["missingDataReplacementValue"] = _missingDataReplacementValue;
    jsonObject["clippingType"] = static_cast<int>(_clippingType);
    jsonObject["clippingValue"] = _clippingValue;

    return QByteArray::fromStdString(jsonObject.dump());
}

bool CorrelationPluginInstance::load(const QByteArray& data, int dataVersion, IMutableGraph& graph, IParser& parser)
{
    json jsonObject = parseJsonFrom(data, &parser);

    if(parser.cancelled())
        return false;

    if(jsonObject.is_null() || !jsonObject.is_object())
    {
        setGenericFailureReason(CURRENT_SOURCE_LOCATION);
        return false;
    }

    if(dataVersion >= 7)
    {
        if(!u::contains(jsonObject, "numContinuousColumns") || !u::contains(jsonObject, "numDiscreteColumns"))
        {
            setGenericFailureReason(CURRENT_SOURCE_LOCATION);
            return false;
        }

        _numContinuousColumns = static_cast<size_t>(jsonObject["numContinuousColumns"].get<int>());
        _numDiscreteColumns = static_cast<size_t>(jsonObject["numDiscreteColumns"].get<int>());
    }
    else
    {
        if(!u::contains(jsonObject, "numColumns"))
        {
            setGenericFailureReason(CURRENT_SOURCE_LOCATION);
            return false;
        }

        _numContinuousColumns = static_cast<size_t>(jsonObject["numColumns"].get<int>());
    }

    if(!u::contains(jsonObject, "userColumnData"))
    {
        setGenericFailureReason(CURRENT_SOURCE_LOCATION);
        return false;
    }

    if(!_userColumnData.load(jsonObject["userColumnData"], parser))
    {
        setGenericFailureReason(CURRENT_SOURCE_LOCATION);
        return false;
    }

    parser.setProgress(-1);

    if(!u::contains(jsonObject, "dataColumnNames"))
    {
        setGenericFailureReason(CURRENT_SOURCE_LOCATION);
        return false;
    }

    const auto& dataColumnNames = jsonObject["dataColumnNames"];
    std::transform(dataColumnNames.begin(), dataColumnNames.end(), std::back_inserter(_dataColumnNames),
    [](const auto& dataColumnName)
    {
        return QString::fromStdString(dataColumnName);
    });

    uint64_t i = 0;

    parser.setPhase(QObject::tr("Data"));

    const char* continuousDataKey =
        dataVersion >= 7 ? "continuousData" : "data";

    if(!u::contains(jsonObject, continuousDataKey))
    {
        setGenericFailureReason(CURRENT_SOURCE_LOCATION);
        return false;
    }

    const auto& jsonContinuousData = jsonObject[continuousDataKey];
    for(const auto& value : jsonContinuousData)
    {
        _continuousData.emplace_back(value);
        parser.setProgress(static_cast<int>((i++ * 100) / jsonContinuousData.size()));
    }

    if(_numContinuousColumns > 0)
    {
        Q_ASSERT((_continuousData.size() % _numContinuousColumns) == 0);
        if((_continuousData.size() % _numContinuousColumns) != 0)
        {
            setGenericFailureReason(CURRENT_SOURCE_LOCATION);
            return false;
        }

        _numRows = _continuousData.size() / _numContinuousColumns;
    }

    _continuousEpsilon = CorrelationFileParser::epsilonFor(_continuousData);

    if(dataVersion >= 7)
    {
        if(!u::contains(jsonObject, "discreteData"))
        {
            setGenericFailureReason(CURRENT_SOURCE_LOCATION);
            return false;
        }

        i = 0;
        const auto& jsonDiscreteData = jsonObject["discreteData"];
        for(const auto& value : jsonDiscreteData)
        {
            _discreteData.emplace_back(QString::fromStdString(value));
            parser.setProgress(static_cast<int>((i++ * 100) / jsonDiscreteData.size()));
        }

        if(_numDiscreteColumns > 0)
        {
            Q_ASSERT((_discreteData.size() % _numDiscreteColumns) == 0);
            if((_discreteData.size() % _numDiscreteColumns) != 0)
            {
                setGenericFailureReason(CURRENT_SOURCE_LOCATION);
                return false;
            }

            _numRows = _discreteData.size() / _numDiscreteColumns;
        }
    }

    Q_ASSERT(_numRows > 0);
    if(_numRows == 0)
    {
        setGenericFailureReason(CURRENT_SOURCE_LOCATION);
        return false;
    }

    if(dataVersion >= 11)
    {
        if(!u::contains(jsonObject, "hcOrdering"))
        {
            setGenericFailureReason(CURRENT_SOURCE_LOCATION);
            return false;
        }

        i = 0;
        const auto& jsonHcOrdering = jsonObject["hcOrdering"];
        for(const auto& value : jsonHcOrdering)
        {
            _continuousHcOrder.emplace_back(value);
            parser.setProgress(static_cast<int>((i++ * 100) / jsonHcOrdering.size()));
        }
    }

    parser.setProgress(-1);

    for(size_t row = 0; row < _numRows; row++)
    {
        auto nodeId = _graphModel->userNodeData().elementIdForIndex(row);

        if(!nodeId.isNull())
        {
            _continuousDataRows.emplace_back(_continuousData, row, _numContinuousColumns, nodeId).update();
            _discreteDataRows.emplace_back(_discreteData, row, _numDiscreteColumns, nodeId).update();
        }

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
    parser.setPhase(QObject::tr("Correlation Values"));
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
            auto edgeId = static_cast<EdgeId>(i);
            if(graph.containsEdgeId(edgeId))
                _correlationValues->set(edgeId, correlationValue);

            parser.setProgress(static_cast<int>((i++ * 100) / jsonCorrelationValues.size()));
        }
    }

    parser.setProgress(-1);

    const char* correlationThresholdKey =
        dataVersion >= 14 ? "minimumThreshold" :
        dataVersion >= 12 ? "threshold" :
        "minimumCorrelationValue";

    if(!u::containsAllOf(jsonObject, {correlationThresholdKey, "transpose", "scaling",
        "normalisation", "missingDataType", "missingDataReplacementValue"}))
    {
        setGenericFailureReason(CURRENT_SOURCE_LOCATION);
        return false;
    }

    _minimumThreshold = jsonObject[correlationThresholdKey];

    if(dataVersion >= 14)
        _maximumK = jsonObject["maximumK"];

    _transpose = jsonObject["transpose"];
    _scalingType = normaliseQmlEnum<ScalingType>(jsonObject["scaling"]);
    _normaliseType = normaliseQmlEnum<NormaliseType>(jsonObject["normalisation"]);
    _missingDataType = normaliseQmlEnum<MissingDataType>(jsonObject["missingDataType"]);
    _missingDataReplacementValue = jsonObject["missingDataReplacementValue"];

    if(dataVersion >= 10)
    {
        if(!u::containsAllOf(jsonObject, {"clippingType", "clippingValue"}))
        {
            setGenericFailureReason(CURRENT_SOURCE_LOCATION);
            return false;
        }

        _clippingType = normaliseQmlEnum<ClippingType>(jsonObject["clippingType"]);
        _clippingValue = jsonObject["clippingValue"];
    }

    if(dataVersion >= 7)
    {
        if(!u::containsAllOf(jsonObject, {"correlationDataType", "continuousCorrelationType",
            "discreteCorrelationType", "correlationPolarity"}))
        {
            setGenericFailureReason(CURRENT_SOURCE_LOCATION);
            return false;
        }

        _correlationDataType = normaliseQmlEnum<CorrelationDataType>(jsonObject["correlationDataType"]);
        _continuousCorrelationType = normaliseQmlEnum<CorrelationType>(jsonObject["continuousCorrelationType"]);
        _discreteCorrelationType = normaliseQmlEnum<CorrelationType>(jsonObject["discreteCorrelationType"]);
        _correlationPolarity = normaliseQmlEnum<CorrelationPolarity>(jsonObject["correlationPolarity"]);
    }
    else if(dataVersion >= 3)
    {
        if(!u::contains(jsonObject, "correlationType") || !u::contains(jsonObject, "correlationPolarity"))
        {
            setGenericFailureReason(CURRENT_SOURCE_LOCATION);
            return false;
        }

        _correlationDataType = CorrelationDataType::Continuous;
        _continuousCorrelationType = normaliseQmlEnum<CorrelationType>(jsonObject["correlationType"]);
        _discreteCorrelationType = CorrelationType::Jaccard;
        _correlationPolarity = normaliseQmlEnum<CorrelationPolarity>(jsonObject["correlationPolarity"]);
    }

    createAttributes();
    buildDiscreteDataValueIndex(parser);
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

    auto correlation = ContinuousCorrelation::create(_continuousCorrelationType, _correlationFilterType);

    switch(_correlationDataType)
    {
    default:
    case CorrelationDataType::Continuous:
        text.append(tr("\nContinuous Correlation Metric: %1").arg(correlation->name()));

        text.append(tr("\nCorrelation Polarity: "));
        switch(_correlationPolarity)
        {
        default:
        case CorrelationPolarity::Positive: text.append(tr("Positive")); break;
        case CorrelationPolarity::Negative: text.append(tr("Negative")); break;
        case CorrelationPolarity::Both: text.append(tr("Both")); break;
        }
        break;

    case CorrelationDataType::Discrete:
        text.append(tr("\nDiscrete Correlation Metric: %1").arg(correlation->name()));
        break;
    }

    text.append(tr("\nMinimum Correlation Value: %1").arg(
        u::formatNumberScientific(_minimumThreshold)));

    if(_correlationFilterType == CorrelationFilterType::Knn)
        text.append(tr("\nMaximum k Value: %1").arg(_maximumK));

    if(_valuesWereImputed)
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

    switch(_clippingType)
    {
    default:
    case ClippingType::None: break;
    case ClippingType::Constant: text.append(tr("\nClipping: Constant (%1)")
        .arg(u::formatNumberScientific(_clippingValue))); break;
    case ClippingType::Winsorization: text.append(tr("\nClipping: Winsorization (%1 percentile)")
        .arg(static_cast<int>(_clippingValue))); break;
    }

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

    return text;
}

void CorrelationPluginInstance::importAnnotationsFromTable(
    std::shared_ptr<TabularData> data, // NOLINT performance-unnecessary-value-param
    int keyRowIndex, const std::vector<int>& importRowIndices, bool replace)
{
    commandManager()->execute(ExecutePolicy::Add, std::make_unique<ImportAnnotationsCommand>(
        this, &(*data), keyRowIndex, importRowIndices, replace));
}

CorrelationPlugin::CorrelationPlugin()
{
    registerUrlType(u"CorrelationCSV"_s, QObject::tr("Correlation CSV File"), QObject::tr("Correlation CSV Files"), {"csv"});
    registerUrlType(u"CorrelationTSV"_s, QObject::tr("Correlation TSV File"), QObject::tr("Correlation TSV Files"), {"tsv"});
    registerUrlType(u"CorrelationSSV"_s, QObject::tr("Correlation SSV File"), QObject::tr("Correlation SSV Files"), {"ssv"});
    registerUrlType(u"CorrelationXLSX"_s, QObject::tr("Correlation Excel File"), QObject::tr("Correlation Excel Files"), {"xlsx"});

    qmlRegisterType<CorrelationPluginInstance>("app.graphia", 1, 0, "CorrelationPluginInstance");
    qmlRegisterType<CorrelationPlotItem>("app.graphia", 1, 0, "CorrelationPlot");
    qmlRegisterType<GraphSizeEstimatePlotItem>("app.graphia", 1, 0, "GraphSizeEstimatePlot");
    qmlRegisterType<CorrelationTabularDataParser>("app.graphia", 1, 0, "CorrelationTabularDataParser");
    qmlRegisterType<ImportAnnotationsKeyDetection>("app.graphia", 1, 0, "ImportAnnotationsKeyDetection");
}

QVariantMap CorrelationPlugin::correlationInfoFor(int correlationType) const
{
    auto makeVariantMap = [](const auto& correlation)
    {
        QVariantMap m;

        m.insert(u"name"_s, correlation->name());
        m.insert(u"description"_s, correlation->description());
        m.insert(u"attributeName"_s, correlation->attributeName());
        m.insert(u"attributeDescription"_s, correlation->attributeDescription());

        return m;
    };

    auto discreteCorrelation = DiscreteCorrelation::create(
        normaliseQmlEnum<CorrelationType>(correlationType),
        CorrelationFilterType::Threshold);
    if(discreteCorrelation != nullptr)
        return makeVariantMap(discreteCorrelation);

    auto continuousCorrelation = ContinuousCorrelation::create(
        normaliseQmlEnum<CorrelationType>(correlationType),
        CorrelationFilterType::Threshold);
    if(continuousCorrelation != nullptr)
        return makeVariantMap(continuousCorrelation);

    return {};
}

QStringList CorrelationPlugin::identifyUrl(const QUrl& url) const
{
    if(!url.isLocalFile())
        return {};

    auto urlTypes = identifyByExtension(url);
    auto contentType = TabularData::contentIdentityOf(url);

    if(urlTypes.isEmpty() || !urlTypes.first().endsWith(contentType))
        return {};

    return urlTypes;
}

QString CorrelationPlugin::failureReason(const QUrl& url) const
{
    auto urlTypes = identifyByExtension(url);

    if(urlTypes.isEmpty())
        return {};

    auto extensionIdentity = urlTypes.first();
    auto contentIdentity = TabularData::contentIdentityOf(url);

    if(contentIdentity.isEmpty())
    {
        return tr("%1 has an extension that indicates it is a '%2', "
            "however its content could not be determined.").arg(url.fileName(),
            individualDescriptionForUrlTypeName(extensionIdentity));
    }

    if(!extensionIdentity.endsWith(contentIdentity))
    {
        return tr("%1 has an extension that indicates it is a '%2', "
            "however its content resembles a '%3' file.").arg(url.fileName(),
            individualDescriptionForUrlTypeName(extensionIdentity), contentIdentity);
    }

    return {};
}
