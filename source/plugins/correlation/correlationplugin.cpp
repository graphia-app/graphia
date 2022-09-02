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
}

void CorrelationPluginInstance::initialise(const IPlugin* plugin, IDocument* document,
                                           const IParserThread* parserThread)
{
    BasePluginInstance::initialise(plugin, document, parserThread);

    _graphModel = document->graphModel();
    _nodeAttributeTableModel.initialise(document, &_graphModel->userNodeData());

    _correlationValues = std::make_unique<EdgeArray<double>>(_graphModel->mutableGraph());

    const auto* modelQObject = dynamic_cast<const QObject*>(_graphModel);
    connect(modelQObject, SIGNAL(attributesChanged(QStringList,QStringList,QStringList)),
            this, SIGNAL(sharedValuesAttributeNamesChanged()));
    connect(modelQObject, SIGNAL(attributesChanged(QStringList,QStringList,QStringList)),
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
    size_t numColumns = _numContinuousColumns + _numDiscreteColumns;
    size_t left = dataRect.x();
    size_t right = dataRect.x() + dataRect.width();
    size_t top = dataRect.y();
    size_t bottom = dataRect.y() + dataRect.height();

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

            uint64_t rowOffset = static_cast<uint64_t>(rowIndex) * tabularData.numColumns();
            uint64_t dataPoint = columnIndex + rowOffset;
            parser.setProgress(static_cast<int>((dataPoint * 100) / numDataPoints));

            const auto& value = tabularData.valueAt(columnIndex, rowIndex);

            //FIXME: If there are continuous and discrete columns, dataColumnIndex will need to change
            size_t dataColumnIndex = columnIndex - dataRect.x();
            size_t dataRowIndex = rowIndex - dataRect.y();
            bool isColumnInDataRect = left <= columnIndex && columnIndex < right;
            bool isRowInDataRect = top <= rowIndex && rowIndex < bottom;
            bool isColumnAnnotation = rowIndex < top;
            bool isRowAttribute = columnIndex < left;

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
            else if(isColumnInDataRect)
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

    CorrelationFileParser::clipValues(_clippingType, _clippingValue, _continuousData);

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

    auto correlationDataType = NORMALISE_QML_ENUM(CorrelationDataType, _correlationDataType);
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

        auto continuousCorrelation = ContinuousCorrelation::create(NORMALISE_QML_ENUM(CorrelationType, _continuousCorrelationType));
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

        auto discreteCorrelation = DiscreteCorrelation::create(NORMALISE_QML_ENUM(CorrelationType, _discreteCorrelationType));
        _correlationAttributeName = discreteCorrelation->attributeName();
        correlationAttributeDescription = discreteCorrelation->attributeDescription();
        break;
    }
    }

    graphModel()->createAttribute(_correlationAttributeName)
        .setFloatValueFn([this](EdgeId edgeId) { return _correlationValues->get(edgeId); })
        .setFlag(AttributeFlag::AutoRange)
        .setDescription(correlationAttributeDescription);

    auto correlationPolarity = NORMALISE_QML_ENUM(CorrelationPolarity, _correlationPolarity);
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

EdgeList CorrelationPluginInstance::correlation(double minimumThreshold, IParser& parser)
{
    auto correlationDataType = NORMALISE_QML_ENUM(CorrelationDataType, _correlationDataType);
    switch(correlationDataType)
    {
    default:
    case CorrelationDataType::Continuous:
    {
        auto continuousCorrelation = ContinuousCorrelation::create(NORMALISE_QML_ENUM(CorrelationType, _continuousCorrelationType));
        return continuousCorrelation->edgeList(_continuousDataRows, minimumThreshold,
            NORMALISE_QML_ENUM(CorrelationPolarity, _correlationPolarity), &parser, &parser);
    }

    case CorrelationDataType::Discrete:
    {
        auto discreteCorrelation = DiscreteCorrelation::create(NORMALISE_QML_ENUM(CorrelationType, _discreteCorrelationType));
        return discreteCorrelation->edgeList(_discreteDataRows, minimumThreshold, _treatAsBinary, &parser, &parser);
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

void CorrelationPluginInstance::setDataColumnName(size_t column, const QString& name) // NOLINT readability-make-member-function-const
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
                auto newName = QStringLiteral("%1(%2)").arg(name).arg(i);
                setDataColumnName(clashingIndex, newName);
            }
        }

        dataColumnNameIndexes.clear();

        for(size_t i = 0; i < _dataColumnNames.size(); i++)
            dataColumnNameIndexes[_dataColumnNames.at(i)].push_back(i);
    }
}

// NOLINTNEXTLIME readability-make-member-function-const
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
    buildColumnAnnotations();
    _nodeAttributeTableModel.updateColumnNames();
}

void CorrelationPluginInstance::buildColumnAnnotations()
{
    _columnAnnotations.reserve(_userColumnData.numUserDataVectors());

    for(const auto& name : _userColumnData)
    {
        const auto* values = _userColumnData.vector(name);
        _columnAnnotations.emplace_back(name, values->begin(), values->end());
    }

    emit columnAnnotationNamesChanged();
}

// NOLINTNEXTLINE readability-make-member-function-const
void CorrelationPluginInstance::buildDiscreteDataValueIndex(Progressable& progressable)
{
    if(_correlationDataType != CorrelationDataType::Discrete)
        return;

    size_t dataValueIndex = 0;

    for(size_t columnIndex = 0; columnIndex < _numDiscreteColumns; columnIndex++)
    {
        std::set<QString> values;

        for(size_t rowIndex = 0; rowIndex < _numRows; rowIndex++)
            values.emplace(discreteDataAt(static_cast<int>(rowIndex), static_cast<int>(columnIndex)));

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
    std::vector<QString> urlTypes =
    {
        QStringLiteral("CorrelationCSV"),
        QStringLiteral("CorrelationTSV"),
        QStringLiteral("CorrelationSSV"),
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
    else if(name == QStringLiteral("correlationDataType"))
        _correlationDataType = NORMALISE_QML_ENUM(CorrelationDataType, value.toInt());
    else if(name == QStringLiteral("continuousCorrelationType"))
        _continuousCorrelationType = NORMALISE_QML_ENUM(CorrelationType, value.toInt());
    else if(name == QStringLiteral("discreteCorrelationType"))
        _discreteCorrelationType = NORMALISE_QML_ENUM(CorrelationType, value.toInt());
    else if(name == QStringLiteral("correlationPolarity"))
        _correlationPolarity = NORMALISE_QML_ENUM(CorrelationPolarity, value.toInt());
    else if(name == QStringLiteral("scaling"))
        _scalingType = NORMALISE_QML_ENUM(ScalingType, value.toInt());
    else if(name == QStringLiteral("normalise"))
        _normaliseType = NORMALISE_QML_ENUM(NormaliseType, value.toInt());
    else if(name == QStringLiteral("missingDataType"))
        _missingDataType = NORMALISE_QML_ENUM(MissingDataType, value.toInt());
    else if(name == QStringLiteral("missingDataValue"))
        _missingDataReplacementValue = value.toDouble();
    else if(name == QStringLiteral("clippingType"))
        _clippingType = NORMALISE_QML_ENUM(ClippingType, value.toInt());
    else if(name == QStringLiteral("clippingValue"))
        _clippingValue = value.toDouble();
    else if(name == QStringLiteral("treatAsBinary"))
        _treatAsBinary = value.toBool();
    else if(name == QStringLiteral("dataRect"))
        _dataRect = value.toRect();
    else if(name == QStringLiteral("clusteringType"))
        _clusteringType = NORMALISE_QML_ENUM(ClusteringType, value.toInt());
    else if(name == QStringLiteral("edgeReductionType"))
        _edgeReductionType = NORMALISE_QML_ENUM(EdgeReductionType, value.toInt());
    else if(name == QStringLiteral("data") && value.canConvert<std::shared_ptr<TabularData>>())
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

    auto correlationPolarity = NORMALISE_QML_ENUM(CorrelationPolarity, _correlationPolarity);
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

double CorrelationPluginInstance::continuousDataAt(int row, int column) const
{
    return _continuousData.at((row * _numContinuousColumns) + column);
}

QString CorrelationPluginInstance::discreteDataAt(int row, int column) const
{
    return _discreteData.at((row * _numDiscreteColumns) + column);
}

int CorrelationPluginInstance::discreteDataValueIndex(const QString& value) const
{
    if(!u::contains(_discreteDataValueIndex, value) || _discreteDataValueIndex.empty())
        return -1;

    return static_cast<int>(_discreteDataValueIndex.at(value));
}

QString CorrelationPluginInstance::rowName(int row) const
{
    return _graphModel->userNodeData().value(row,
        _graphModel->userNodeData().firstVectorName()).toString();
}

QString CorrelationPluginInstance::columnName(int column) const
{
    return _dataColumnNames.at(static_cast<size_t>(column));
}

QColor CorrelationPluginInstance::nodeColorForRow(int row) const
{
    auto nodeId = _graphModel->userNodeData().elementIdForIndex(row);

    if(nodeId.isNull())
        return {};

    return graphModel()->nodeVisual(nodeId).outerColor();
}

const ColumnAnnotation* CorrelationPluginInstance::columnAnnotationByName(const QString& name) const
{
    auto it = std::find_if(_columnAnnotations.begin(), _columnAnnotations.end(),
        [&name](const auto& v) { return v.name() == name; });

    if(it != _columnAnnotations.end())
        return &(*it);

    return nullptr;
}

QString CorrelationPluginInstance::attributeValueFor(const QString& attributeName, int row) const
{
    const auto* attribute = _graphModel->attributeByName(attributeName);
    auto nodeId = _graphModel->userNodeData().elementIdForIndex(row);

    if(attribute == nullptr || nodeId.isNull())
        return {};

    return attribute->stringValueOf(nodeId);
}

QByteArray CorrelationPluginInstance::save(IMutableGraph& graph, Progressable& progressable) const
{
    json jsonObject;

    jsonObject["numContinuousColumns"] = static_cast<int>(_numContinuousColumns);
    jsonObject["numDiscreteColumns"] = static_cast<int>(_numDiscreteColumns);
    jsonObject["numRows"] = static_cast<int>(_numRows);
    jsonObject["userColumnData"] =_userColumnData.save(progressable);
    jsonObject["dataColumnNames"] = jsonArrayFrom(_dataColumnNames, &progressable);

    graph.setPhase(QObject::tr("Data"));
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

        return array;
    }();

    graph.setPhase(QObject::tr("Correlation Values"));
    jsonObject["correlationValues"] = u::graphArrayAsJson(*_correlationValues, graph.edgeIds(), &progressable);

    jsonObject["minimumCorrelationValue"] = _minimumCorrelationValue;
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
        return false;

    if(dataVersion >= 7)
    {
        if(!u::contains(jsonObject, "numContinuousColumns") || !u::contains(jsonObject, "numDiscreteColumns"))
            return false;

        _numContinuousColumns = static_cast<size_t>(jsonObject["numContinuousColumns"].get<int>());
        _numDiscreteColumns = static_cast<size_t>(jsonObject["numDiscreteColumns"].get<int>());
    }
    else
    {
        if(!u::contains(jsonObject, "numColumns"))
            return false;

        _numContinuousColumns = static_cast<size_t>(jsonObject["numColumns"].get<int>());
    }

    if(!u::contains(jsonObject, "numRows"))
        return false;

    _numRows = static_cast<size_t>(jsonObject["numRows"].get<int>());

    if(!u::contains(jsonObject, "userColumnData"))
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

    graph.setPhase(QObject::tr("Data"));

    const char* continuousDataKey =
        dataVersion >= 7 ? "continuousData" : "data";

    if(!u::contains(jsonObject, continuousDataKey))
        return false;

    const auto& jsonContinuousData = jsonObject[continuousDataKey];
    for(const auto& value : jsonContinuousData)
    {
        _continuousData.emplace_back(value);
        parser.setProgress(static_cast<int>((i++ * 100) / jsonContinuousData.size()));
    }

    _continuousEpsilon = CorrelationFileParser::epsilonFor(_continuousData);

    if(dataVersion >= 7)
    {
        if(!u::contains(jsonObject, "discreteData"))
            return false;

        i = 0;
        const auto& jsonDiscreteData = jsonObject["discreteData"];
        for(const auto& value : jsonDiscreteData)
        {
            _discreteData.emplace_back(QString::fromStdString(value));
            parser.setProgress(static_cast<int>((i++ * 100) / jsonDiscreteData.size()));
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
            auto edgeId = static_cast<EdgeId>(i);
            if(graph.containsEdgeId(edgeId))
                _correlationValues->set(edgeId, correlationValue);

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
    _scalingType = NORMALISE_QML_ENUM(ScalingType, jsonObject["scaling"]);
    _normaliseType = NORMALISE_QML_ENUM(NormaliseType, jsonObject["normalisation"]);
    _missingDataType = NORMALISE_QML_ENUM(MissingDataType, jsonObject["missingDataType"]);
    _missingDataReplacementValue = jsonObject["missingDataReplacementValue"];

    if(dataVersion >= 10)
    {
        if(!u::containsAllOf(jsonObject, {"clippingType", "clippingValue"}))
            return false;

        _clippingType = NORMALISE_QML_ENUM(ClippingType, jsonObject["clippingType"]);
        _clippingValue = jsonObject["clippingValue"];
    }

    if(dataVersion >= 7)
    {
        if(!u::containsAllOf(jsonObject, {"correlationDataType", "continuousCorrelationType",
            "discreteCorrelationType", "correlationPolarity"}))
        {
            return false;
        }

        _correlationDataType = NORMALISE_QML_ENUM(CorrelationDataType, jsonObject["correlationDataType"]);
        _continuousCorrelationType = NORMALISE_QML_ENUM(CorrelationType, jsonObject["continuousCorrelationType"]);
        _discreteCorrelationType = NORMALISE_QML_ENUM(CorrelationType, jsonObject["discreteCorrelationType"]);
        _correlationPolarity = NORMALISE_QML_ENUM(CorrelationPolarity, jsonObject["correlationPolarity"]);
    }
    else if(dataVersion >= 3)
    {
        if(!u::contains(jsonObject, "correlationType") || !u::contains(jsonObject, "correlationPolarity"))
            return false;

        _correlationDataType = CorrelationDataType::Continuous;
        _continuousCorrelationType = NORMALISE_QML_ENUM(CorrelationType, jsonObject["correlationType"]);
        _discreteCorrelationType = CorrelationType::Jaccard;
        _correlationPolarity = NORMALISE_QML_ENUM(CorrelationPolarity, jsonObject["correlationPolarity"]);
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

    auto correlation = ContinuousCorrelation::create(
        NORMALISE_QML_ENUM(CorrelationType, _continuousCorrelationType));

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
        u::formatNumberScientific(_minimumCorrelationValue)));

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

CorrelationPlugin::CorrelationPlugin()
{
    registerUrlType(QStringLiteral("CorrelationCSV"), QObject::tr("Correlation CSV File"), QObject::tr("Correlation CSV Files"), {"csv"});
    registerUrlType(QStringLiteral("CorrelationTSV"), QObject::tr("Correlation TSV File"), QObject::tr("Correlation TSV Files"), {"tsv"});
    registerUrlType(QStringLiteral("CorrelationSSV"), QObject::tr("Correlation SSV File"), QObject::tr("Correlation SSV Files"), {"ssv"});
    registerUrlType(QStringLiteral("CorrelationXLSX"), QObject::tr("Correlation Excel File"), QObject::tr("Correlation Excel Files"), {"xlsx"});

    qmlRegisterType<CorrelationPluginInstance>("app.graphia", 1, 0, "CorrelationPluginInstance");
    qmlRegisterType<CorrelationPlotItem>("app.graphia", 1, 0, "CorrelationPlot");
    qmlRegisterType<GraphSizeEstimatePlotItem>("app.graphia", 1, 0, "GraphSizeEstimatePlot");
    qmlRegisterType<CorrelationTabularDataParser>("app.graphia", 1, 0, "CorrelationTabularDataParser");
}

QVariantMap CorrelationPlugin::correlationInfoFor(int correlationType) const
{
    auto makeVariantMap = [](const auto& correlation)
    {
        QVariantMap m;

        m.insert(QStringLiteral("name"), correlation->name());
        m.insert(QStringLiteral("description"), correlation->description());
        m.insert(QStringLiteral("attributeName"), correlation->attributeName());
        m.insert(QStringLiteral("attributeDescription"), correlation->attributeDescription());

        return m;
    };

    auto discreteCorrelation = DiscreteCorrelation::create(NORMALISE_QML_ENUM(CorrelationType, correlationType));
    if(discreteCorrelation != nullptr)
        return makeVariantMap(discreteCorrelation);

    auto continuousCorrelation = ContinuousCorrelation::create(NORMALISE_QML_ENUM(CorrelationType, correlationType));
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
