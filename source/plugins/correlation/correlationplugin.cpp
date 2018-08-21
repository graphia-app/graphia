#include "correlationplugin.h"

#include "correlationplotitem.h"
#include "loading/correlationfileparser.h"
#include "shared/utils/threadpool.h"
#include "shared/utils/iterator_range.h"
#include "shared/utils/container.h"
#include "shared/utils/random.h"
#include "shared/utils/string.h"
#include "shared/attributes/iattribute.h"
#include "shared/ui/visualisations/ielementvisual.h"

#include "thirdparty/json/json_helper.h"

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

    auto graphModel = document->graphModel();
    _userNodeData.initialise(graphModel->mutableGraph());

    if(_transpose)
    {
        // Don't include data columns in the table model when transposing as this is likely to
        // result in a very large number of columns in the table model, which hurt performance
        _nodeAttributeTableModel.initialise(document, &_userNodeData);
    }
    else
        _nodeAttributeTableModel.initialise(document, &_userNodeData, &_dataColumnNames, &_data);


    _pearsonValues = std::make_unique<EdgeArray<double>>(graphModel->mutableGraph());

    graphModel->createAttribute(tr("Pearson Correlation Value"))
            .setFloatValueFn([this](EdgeId edgeId) { return _pearsonValues->get(edgeId); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Pearson_correlation_coefficient">)"
                               "Pearson Correlation Coefficient</a> is an indication of "
                               "the linear relationship between two variables."));
}

bool CorrelationPluginInstance::loadUserData(const TabularData& tabularData,
    size_t firstDataColumn, size_t firstDataRow, IParser& parser)
{
    if(firstDataColumn == 0 || firstDataRow == 0)
    {
        qDebug() << "tabularData has no row or column names!";
        return false;
    }

    parser.setProgress(-1);

    uint64_t numDataPoints = static_cast<uint64_t>(tabularData.numColumns()) * tabularData.numRows();

    for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            if(parser.cancelled())
                return false;

            uint64_t rowOffset = static_cast<uint64_t>(rowIndex) * tabularData.numColumns();
            uint64_t dataPoint = columnIndex + rowOffset;
            parser.setProgress(static_cast<int>((dataPoint * 100) / numDataPoints));

            QString value = tabularData.valueAsQString(columnIndex, rowIndex);

            size_t dataColumnIndex = columnIndex - firstDataColumn;
            size_t dataRowIndex = rowIndex - firstDataRow;
            bool isRowInDataRect = firstDataRow <= rowIndex;
            bool isColumnInDataRect = firstDataColumn <= columnIndex;

            if((isColumnInDataRect && dataColumnIndex >= _numColumns) ||
                    (isRowInDataRect && dataRowIndex >= _numRows))
            {
                qDebug() << QString("WARNING: Attempting to set data at coordinate (%1, %2) in "
                                    "dataRect of dimensions (%3, %4)")
                            .arg(dataColumnIndex)
                            .arg(dataRowIndex)
                            .arg(_numColumns)
                            .arg(_numRows);
                continue;
            }

            if(rowIndex == 0)
            {
                if(!isColumnInDataRect)
                    _userNodeData.add(value);
                else
                {
                    setDataColumnName(dataColumnIndex, value);
                }
            }
            else if(!isRowInDataRect)
            {
                if(columnIndex == 0)
                    _userColumnData.add(value);
                else if(isColumnInDataRect)
                    _userColumnData.setValue(dataColumnIndex, tabularData.valueAsQString(0, rowIndex), value);
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
                    transformedValue = imputeValue(tabularData, firstDataColumn, firstDataRow, columnIndex, rowIndex);

                transformedValue = scaleValue(transformedValue);

                setData(dataColumnIndex, dataRowIndex, transformedValue);
            }
            else // Not in data rect, not first row, put in to the userNodeData
                _userNodeData.setValue(dataRowIndex, tabularData.valueAsQString(columnIndex, 0), value);
        }
    }

    parser.setProgress(-1);

    return true;
}

bool CorrelationPluginInstance::normalise(IParser& parser)
{
    switch(_normalisation)
    {
    case NormaliseType::MinMax:
    {
        MinMaxNormaliser normaliser;
        return normaliser.process(_data, _numColumns, _numRows, parser);
    }
    case NormaliseType::Quantile:
    {
        QuantileNormaliser normaliser;
        return normaliser.process(_data, _numColumns, _numRows, parser);
    }
    default:
        break;
    }

    return true;
}

void CorrelationPluginInstance::finishDataRows()
{
    for(size_t row = 0; row < _numRows; row++)
        finishDataRow(row);
}

void CorrelationPluginInstance::createAttributes()
{
    graphModel()->createAttribute(tr("Mean Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._mean; })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr("The Mean Data Value is the mean of the values associated "
                               "with the node."));

    graphModel()->createAttribute(tr("Minimum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._minValue; })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr("The Minimum Data Value is the minimum value associated "
                               "with the node."));

    graphModel()->createAttribute(tr("Maximum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._maxValue; })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr("The Maximum Data Value is the maximum value associated "
                               "with the node."));

    graphModel()->createAttribute(tr("Variance"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._variance; })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Variance">Variance</a> )"
                               "is a measure of the spread of the values associated "
                               "with the node. It is defined as ∑(<i>x</i>-µ)², where <i>x</i> is the value "
                               "and µ is the mean."));

    graphModel()->createAttribute(tr("Standard Deviation"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._stddev; })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Standard_deviation">)"
                               "Standard Deviation</a> is a measure of the spread of the values associated "
                               "with the node. It is defined as √∑(<i>x</i>-µ)², where <i>x</i> is the value "
                               "and µ is the mean."));

    graphModel()->createAttribute(tr("Coefficient of Variation"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._coefVar; })
            .setValueMissingFn([this](NodeId nodeId) { return std::isnan(dataRowForNodeId(nodeId)._coefVar); })
            .setFlag(AttributeFlag::AutoRange)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Coefficient_of_variation">)"
                               "Coefficient of Variation</a> "
                               "is a measure of the spread of the values associated "
                               "with the node. It is defined as the standard deviation "
                               "divided by the mean."));
}

std::vector<CorrelationPluginInstance::CorrelationEdge> CorrelationPluginInstance::pearsonCorrelation(
        std::vector<DataRow>::const_iterator begin, std::vector<DataRow>::const_iterator end,
        double minimumThreshold, IParser* parser)
{
    if(parser != nullptr)
        parser->setProgress(-1);

    uint64_t totalCost = 0;
    for(auto& row : _dataRows)
        totalCost += row.computeCostHint();

    std::atomic<uint64_t> cost(0);

    auto results = ThreadPool(QStringLiteral("PearsonCor")).concurrent_for(begin, end,
    [&](std::vector<DataRow>::const_iterator rowAIt)
    {
        const auto& rowA = *rowAIt;
        std::vector<CorrelationEdge> edges;

        if(parser != nullptr && parser->cancelled())
            return edges;

        for(const auto& rowB : make_iterator_range(rowAIt + 1, end))
        {
            double productSum = std::inner_product(rowA.cbegin(), rowA.cend(), rowB.cbegin(), 0.0);
            double numerator = (_numColumns * productSum) - (rowA._sum * rowB._sum);
            double denominator = rowA._variability * rowB._variability;

            double r = numerator / denominator;

            if(std::isfinite(r) && r >= minimumThreshold)
                edges.push_back({rowA._nodeId, rowB._nodeId, r});
        }

        cost += rowA.computeCostHint();

        if(parser != nullptr)
            parser->setProgress((cost * 100) / totalCost);

        return edges;
    });

    if(parser != nullptr)
    {
        // Returning the results might take time
        parser->setProgress(-1);
    }

    std::vector<CorrelationEdge> edges;
    edges.reserve(std::distance(results.begin(), results.end()));
    edges.insert(edges.end(), std::make_move_iterator(results.begin()), std::make_move_iterator(results.end()));

    return edges;
}

void CorrelationPluginInstance::setHighlightedRows(const QVector<int>& highlightedRows)
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

std::vector<CorrelationPluginInstance::CorrelationEdge> CorrelationPluginInstance::pearsonCorrelation(
    const QString& fileName, double minimumThreshold, IParser& parser)
{
    // Perform a preliminary correlation on a small random sample of the input data, so we can
    // tell if the user is trying to create an absurdly large graph and then give them the
    // option to cancel incase they made a mistake
    const int percent = 1;
    const auto numSampleRows = (_dataRows.size() * percent) / 100;
    const auto sample = u::randomSample(_dataRows, numSampleRows);
    const auto sampleEdges = pearsonCorrelation(sample.cbegin(), sample.cend(), minimumThreshold);
    const auto numEdgesEstimate = (sampleEdges.size() * 10000) / (percent * percent);

    const auto numNodes = _dataRows.size();
    const auto warningThreshold = static_cast<double>(5e6);

    if(numNodes > warningThreshold || numEdgesEstimate > warningThreshold)
    {
        auto warningResult = document()->messageBox(MessageBoxIcon::Warning,
            QObject::tr("Correlation"), QString(QObject::tr(
                "Loading '%1' at a minimum threshold of %2 will result in a very large "
                "graph (%3 nodes, approx. %4 edges). This has the potential to exhaust "
                "system resources and lead to instability or freezes. Are you sure you "
                "wish to continue?"))
                .arg(fileName, QString::number(minimumThreshold),
                     u::formatUsingSIPostfix(numNodes),
                     u::formatUsingSIPostfix(numEdgesEstimate)),
            {MessageBoxButton::Yes, MessageBoxButton::No});

        if(warningResult == MessageBoxButton::No)
        {
            parser.cancel();
            return {};
        }
    }

    return pearsonCorrelation(_dataRows.cbegin(), _dataRows.cend(), minimumThreshold, &parser);
}

bool CorrelationPluginInstance::createEdges(const std::vector<CorrelationPluginInstance::CorrelationEdge>& edges,
                                            IParser& parser)
{
    parser.setProgress(-1);
    for(auto edgeIt = edges.begin(); edgeIt != edges.end(); ++edgeIt)
    {
        if(parser.cancelled())
            return false;

        parser.setProgress(std::distance(edges.begin(), edgeIt) * 100 / static_cast<int>(edges.size()));

        auto& edge = *edgeIt;
        auto edgeId = graphModel()->mutableGraph().addEdge(edge._source, edge._target);
        _pearsonValues->set(edgeId, edge._r);
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

void CorrelationPluginInstance::setData(size_t column, size_t row, double value)
{
    auto index = (row * _numColumns) + column;
    Q_ASSERT(index < _data.size());
    _data.at(index) = value;
}

void CorrelationPluginInstance::finishDataRow(size_t row)
{
    Q_ASSERT(row < _numRows);

    auto dataStartIndex = row * _numColumns;
    auto dataEndIndex = dataStartIndex + _numColumns;

    auto begin =_data.cbegin() + dataStartIndex;
    auto end = _data.cbegin() + dataEndIndex;
    auto nodeId = graphModel()->mutableGraph().addNode();
    auto computeCost = static_cast<int>(_numRows - row + 1);

    _dataRows.emplace_back(begin, end, nodeId, computeCost);
    _userNodeData.setElementIdForRowIndex(nodeId, row);

    auto nodeName = _userNodeData.valueBy(nodeId, _userNodeData.firstUserDataVectorName()).toString();
    graphModel()->setNodeName(nodeId, nodeName);
}

double CorrelationPluginInstance::imputeValue(const TabularData& tabularData,
    size_t firstDataColumn, size_t firstDataRow,
    size_t columnIndex, size_t rowIndex)
{
    double imputedValue = 0.0;

    switch(_missingDataType)
    {
    case MissingDataType::Constant:
    {
        imputedValue = _missingDataReplacementValue;
        break;
    }
    case MissingDataType::ColumnAverage:
    {
        // Calculate column averages
        double averageValue = 0.0;
        size_t rowCount = 0;
        for(size_t avgRowIndex = firstDataRow; avgRowIndex < tabularData.numRows(); avgRowIndex++)
        {
            auto value = tabularData.valueAsQString(columnIndex, avgRowIndex);
            if(!value.isEmpty())
            {
                averageValue += value.toDouble();
                rowCount++;
            }
        }

        if(rowCount > 0)
            averageValue /= rowCount;

        imputedValue = averageValue;
        break;
    }
    case MissingDataType::RowInterpolation:
    {
        double rightValue = 0.0;
        double leftValue = 0.0;
        size_t leftDistance = 0;
        size_t rightDistance = 0;
        bool rightValueFound = false;
        bool leftValueFound = false;

        // Find right value
        for(size_t rightColumn = columnIndex; rightColumn < tabularData.numColumns(); rightColumn++)
        {
            auto value = tabularData.valueAsQString(rightColumn, rowIndex);
            if(!value.isEmpty())
            {
                rightValue = value.toDouble();
                rightValueFound = true;
                rightDistance = (rightColumn > columnIndex) ? rightColumn - columnIndex : columnIndex - rightColumn;
                break;
            }
        }
        // Find left value
        for(size_t leftColumn = columnIndex; leftColumn-- != firstDataColumn;)
        {
            auto value = tabularData.valueAsQString(leftColumn, rowIndex);
            if(!value.isEmpty())
            {
                leftValue = value.toDouble();
                leftValueFound = true;
                leftDistance = (leftColumn > columnIndex) ? leftColumn - columnIndex : columnIndex - leftColumn;
                break;
            }
        }

        // Lerp the result if possible, otherwise just set to found value
        if(leftValueFound && rightValueFound)
        {
            size_t totalDistance = leftDistance + rightDistance;
            double tween = leftDistance / static_cast<double>(totalDistance);
            // https://devblogs.nvidia.com/lerp-faster-cuda/
            double lerpedValue = std::fma(tween, rightValue, std::fma(-tween, leftValue, leftValue));
            imputedValue = lerpedValue;
        }
        else if(leftValueFound && !rightValueFound)
            imputedValue = leftValue;
        else if(!leftValueFound && rightValueFound)
            imputedValue = rightValue;
        else // Nothing on the row, just zero it
            imputedValue = 0.0;
        break;
    }
    default:
        break;
    }

    return imputedValue;
}

double CorrelationPluginInstance::scaleValue(double value)
{
    // LogY(x+c) where c is EPSILON
    // This prevents LogY(0) which is -inf
    // Log2(0+c) = -1057
    // Document this!
    const double EPSILON = std::nextafter(0.0, 1.0);

    switch(_scaling)
    {
    case ScalingType::Log2:
        return std::log2(value + EPSILON);
    case ScalingType::Log10:
        return std::log10(value + EPSILON);
    case ScalingType::AntiLog2:
        return std::pow(2.0, value);
    case ScalingType::AntiLog10:
        return std::pow(10.0, value);
    case ScalingType::ArcSin:
        return std::asin(value);
    default:
        break;
    }
    return value;
}

void CorrelationPluginInstance::onLoadSuccess()
{
    _userNodeData.exposeAsAttributes(*graphModel());
    _nodeAttributeTableModel.updateRoleNames();
    buildColumnAnnotations();
}

QVector<double> CorrelationPluginInstance::rawData()
{
    return QVector<double>::fromStdVector(_data);
}

QVector<QColor> CorrelationPluginInstance::nodeColors()
{
    QVector<QColor> colors;
    colors.reserve(static_cast<int>(_numRows));

    for(size_t i = 0; i < _numRows; i++)
    {
        auto nodeId = _userNodeData.elementIdForRowIndex(i);
        auto color = !nodeId.isNull() ? graphModel()->nodeVisual(nodeId).outerColor() : QColor{};

        colors.append(color);
    }

    return colors;
}

QStringList CorrelationPluginInstance::columnNames()
{
    QStringList list;
    list.reserve(static_cast<int>(_dataColumnNames.size()));

    for(const auto& name : _dataColumnNames)
        list.append(name);

    return list;
}

QStringList CorrelationPluginInstance::rowNames()
{
    QStringList list;
    list.reserve(static_cast<int>(_numRows));

    const auto& [name, firstColumn] = *_userNodeData.begin();
    for(size_t i = 0; i < _numRows; i++)
        list.append(firstColumn.get(i));

    return list;
}

void CorrelationPluginInstance::buildColumnAnnotations()
{
    _columnAnnotations.reserve(_userColumnData.numUserDataVectors());

    for(const auto& [name, values] : _userColumnData)
    {
        auto numValues = values.numValues();
        auto numUniqueValues = values.numUniqueValues();

        // If the number of unique values is more than a quarter of the total
        // number of values, skip it, since a large number of unique values
        // causes performance problems, and it's probably not a useful annotation
        // in the first place
        if(numUniqueValues * 4 > numValues)
            continue;

        QVariantMap columnAnnotation;
        columnAnnotation.insert(QStringLiteral("name"), name);
        columnAnnotation.insert(QStringLiteral("values"), values.toStringList());

        _columnAnnotations.append(columnAnnotation);
    }
}

const CorrelationPluginInstance::DataRow& CorrelationPluginInstance::dataRowForNodeId(NodeId nodeId) const
{
    return _dataRows.at(_userNodeData.rowIndexFor(nodeId));
}

void CorrelationPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    _nodeAttributeTableModel.onSelectionChanged();
}

std::unique_ptr<IParser> CorrelationPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == QLatin1String("CorrelationCSV") || urlTypeName == QLatin1String("CorrelationTSV"))
        return std::make_unique<CorrelationFileParser>(this, urlTypeName, _dataRect);

    return nullptr;
}

void CorrelationPluginInstance::applyParameter(const QString& name, const QVariant& value)
{
    if(name == QLatin1String("minimumCorrelation"))
        _minimumCorrelationValue = value.toDouble();
    else if(name == QLatin1String("initialThreshold"))
        _initialCorrelationThreshold = value.toDouble();
    else if(name == QLatin1String("transpose"))
        _transpose = (value == QLatin1String("true"));
    else if(name == QLatin1String("scaling"))
        _scaling = static_cast<ScalingType>(value.toInt());
    else if(name == QLatin1String("normalise"))
        _normalisation = static_cast<NormaliseType>(value.toInt());
    else if(name == QLatin1String("missingDataType"))
        _missingDataType = static_cast<MissingDataType>(value.toInt());
    else if(name == QLatin1String("missingDataValue"))
        _missingDataReplacementValue = value.toDouble();
    else if(name == QLatin1String("dataFrame"))
        _dataRect = value.toRect();
    else if(name == QLatin1String("clusteringType"))
        _clusteringType = static_cast<ClusteringType>(value.toInt());
    else if(name == QLatin1String("edgeReductionType"))
        _edgeReductionType = static_cast<EdgeReductionType>(value.toInt());
}

QStringList CorrelationPluginInstance::defaultTransforms() const
{
    QStringList defaultTransforms =
    {
        QString(R"("Remove Edges" where $"Pearson Correlation Value" < %1)").arg(_initialCorrelationThreshold),
        R"([pinned] "Remove Components" where $"Component Size" <= 1)"
    };

    if(_edgeReductionType == EdgeReductionType::KNN)
        defaultTransforms.append(QStringLiteral(R"("k-NN" using $"Pearson Correlation Value" with "k" = 5 "Rank Order" = "Ascending")"));

    if(_clusteringType == ClusteringType::MCL)
        defaultTransforms.append(QStringLiteral(R"("MCL Cluster" with "Granularity" = 2.2)"));

    return defaultTransforms;
}

QStringList CorrelationPluginInstance::defaultVisualisations() const
{
    if(_clusteringType == ClusteringType::MCL)
        return { R"("MCL Cluster" "Colour")" };

    return {};
}

QByteArray CorrelationPluginInstance::save(IMutableGraph& graph, Progressable& progressable) const
{
    json jsonObject;

    jsonObject["numColumns"] = static_cast<int>(_numColumns);
    jsonObject["numRows"] = static_cast<int>(_numRows);
    jsonObject["userNodeData"] = _userNodeData.save(graph, progressable);
    jsonObject["userColumnData"] =_userColumnData.save(progressable);
    jsonObject["dataColumnNames"] = jsonArrayFrom(_dataColumnNames, &progressable);

    graph.setPhase(QObject::tr("Data"));
    jsonObject["data"] = jsonArrayFrom(_data, &progressable);

    graph.setPhase(QObject::tr("Pearson Values"));
    jsonObject["pearsonValues"] = jsonArrayFrom(*_pearsonValues);

    jsonObject["minimumCorrelationValue"] = _minimumCorrelationValue;
    jsonObject["transpose"] = _transpose;
    jsonObject["scaling"] = static_cast<int>(_scaling);
    jsonObject["normalisation"] = static_cast<int>(_normalisation);
    jsonObject["missingDataType"] = static_cast<int>(_missingDataType);
    jsonObject["missingDataReplacementValue"] = _missingDataReplacementValue;

    return QByteArray::fromStdString(jsonObject.dump());
}

bool CorrelationPluginInstance::load(const QByteArray& data, int dataVersion, IMutableGraph& graph,
                                     IParser& parser)
{
    if(dataVersion != plugin()->dataVersion())
        return false;

    json jsonObject = parseJsonFrom(data, parser);

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

    for(const auto& dataColumnName : jsonObject["dataColumnNames"])
        _dataColumnNames.emplace_back(QString::fromStdString(dataColumnName));

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
        auto dataStartIndex = row * _numColumns;
        auto dataEndIndex = dataStartIndex + _numColumns;

        auto begin =_data.cbegin() + dataStartIndex;
        auto end = _data.cbegin() + dataEndIndex;
        auto computeCost = static_cast<int>(_numRows - row + 1);

        auto nodeId = _userNodeData.elementIdForRowIndex(row);
        _dataRows.emplace_back(begin, end, nodeId, computeCost);

        parser.setProgress(static_cast<int>((row * 100) / _numRows));
    }

    parser.setProgress(-1);

    createAttributes();

    if(!u::contains(jsonObject, "pearsonValues"))
        return false;

    const auto& jsonPearsonValues = jsonObject["pearsonValues"];
    graph.setPhase(QObject::tr("Pearson Values"));
    i = 0;
    for(const auto& pearsonValue : jsonPearsonValues)
    {
        if(graph.containsEdgeId(i))
            _pearsonValues->set(i, pearsonValue);

        parser.setProgress(static_cast<int>((i++ * 100) / jsonPearsonValues.size()));
    }

    parser.setProgress(-1);

    if(!u::containsAllOf(jsonObject, {"minimumCorrelationValue", "transpose", "scaling",
        "normalisation", "missingDataType", "missingDataReplacementValue"}))
    {
        return false;
    }

    _minimumCorrelationValue = jsonObject["minimumCorrelationValue"];
    _transpose = jsonObject["transpose"];
    _scaling = static_cast<ScalingType>(jsonObject["scaling"]);
    _normalisation = static_cast<NormaliseType>(jsonObject["normalisation"]);
    _missingDataType = static_cast<MissingDataType>(jsonObject["missingDataType"]);
    _missingDataReplacementValue = jsonObject["missingDataReplacementValue"];

    return true;
}

CorrelationPlugin::CorrelationPlugin()
{
    registerUrlType(QStringLiteral("CorrelationCSV"), QObject::tr("Correlation CSV File"), QObject::tr("Correlation CSV Files"), {"csv"});
    registerUrlType(QStringLiteral("CorrelationTSV"), QObject::tr("Correlation TSV File"), QObject::tr("Correlation TSV Files"), {"tsv"});
    qmlRegisterType<CorrelationPlotItem>("com.kajeka", 1, 0, "CorrelationPlot");
    qmlRegisterType<CorrelationPreParser>("com.kajeka", 1, 0, "CorrelationPreParser");
    qmlRegisterType<DataRectTableModel>("com.kajeka", 1, 0, "DataRectTableModel");
}

static QString contentIdentityOf(const QUrl& url)
{
    QString identity;

    std::ifstream file(url.toLocalFile().toStdString());
    std::string line;

    if(file && u::getline(file, line))
    {
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
    }

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

    if(extensionIdentity != contentIdentity)
    {
        return tr("%1 has an extension that indicates it is a '%2', "
            "however its content resembles a '%3'.").arg(url.fileName(),
            individualDescriptionForUrlTypeName(extensionIdentity),
            individualDescriptionForUrlTypeName(contentIdentity));
    }

    return {};
}
