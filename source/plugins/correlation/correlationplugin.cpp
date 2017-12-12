#include "correlationplugin.h"

#include "correlationplotitem.h"
#include "loading/correlationfileparser.h"
#include "shared/utils/threadpool.h"
#include "shared/utils/iterator_range.h"
#include "shared/utils/container.h"
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
    _nodeAttributeTableModel.initialise(document, &_userNodeData, &_dataColumnNames, &_data);
    _pearsonValues = std::make_unique<EdgeArray<double>>(graphModel->mutableGraph());

    graphModel->createAttribute(tr("Pearson Correlation Value"))
            .setFloatValueFn([this](EdgeId edgeId) { return _pearsonValues->get(edgeId); })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Pearson_correlation_coefficient">)"
                               "Pearson Correlation Coefficient</a> is an indication of "
                               "the linear relationship between two variables."));
}

bool CorrelationPluginInstance::loadUserData(const TabularData& tabularData, size_t firstDataColumn, size_t firstDataRow,
                                             const std::function<bool()>& cancelled, const ProgressFn& progressFn)
{
    if(firstDataColumn == 0 || firstDataRow == 0)
    {
        qDebug() << "tabularData has no row or column names!";
        return false;
    }

    progressFn(-1);

    uint64_t numDataPoints = static_cast<uint64_t>(tabularData.numColumns()) * tabularData.numRows();

    for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            if(cancelled())
                return false;

            uint64_t rowOffset = static_cast<uint64_t>(rowIndex) * tabularData.numColumns();
            uint64_t dataPoint = columnIndex + rowOffset;
            progressFn(static_cast<int>((dataPoint * 100) / numDataPoints));

            QString value = tabularData.valueAsQString(columnIndex, rowIndex);

            auto dataColumnIndex = static_cast<int>(columnIndex - firstDataColumn);
            auto dataRowIndex = static_cast<int>(rowIndex - firstDataRow);

            if(dataColumnIndex >= static_cast<int>(_numColumns) ||
               dataRowIndex >= static_cast<int>(_numRows))
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
                if(dataColumnIndex < 0)
                    _userNodeData.add(value);
                else
                    setDataColumnName(dataColumnIndex, value);
            }
            else if(dataRowIndex < 0)
            {
                if(columnIndex == 0)
                    _userColumnData.add(value);
                else if(dataColumnIndex >= 0)
                    _userColumnData.setValue(dataColumnIndex, tabularData.valueAsQString(0, rowIndex), value);
            }
            else
            {
                if(dataColumnIndex >= 0)
                {
                    double doubleValue = 0.0;

                    // Replace missing values if required
                    if(_missingDataType == MissingDataType::Constant && value.isEmpty())
                        doubleValue = _missingDataReplacementValue;
                    else
                        doubleValue = value.toDouble();

                    // Scale
                    doubleValue = scaleValue(doubleValue);

                    setData(dataColumnIndex, dataRowIndex, doubleValue);
                }
                else
                    _userNodeData.setValue(dataRowIndex, tabularData.valueAsQString(columnIndex, 0), value);
            }
        }
    }

    progressFn(-1);

    return true;
}

bool CorrelationPluginInstance::normalise(const std::function<bool()>& cancelled, const ProgressFn& progressFn)
{
    switch(_normalisation)
    {
    case NormaliseType::MinMax:
    {
        MinMaxNormaliser normaliser;
        return normaliser.process(_data, _numColumns, _numRows, cancelled, progressFn);
    }
    case NormaliseType::Quantile:
    {
        QuantileNormaliser normaliser;
        return normaliser.process(_data, _numColumns, _numRows, cancelled, progressFn);
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
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The Mean Data Value is the mean of the values associated "
                               "with the node."));

    graphModel()->createAttribute(tr("Minimum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._minValue; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The Minimum Data Value is the minimum value associated "
                               "with the node."));

    graphModel()->createAttribute(tr("Maximum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._maxValue; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The Maximum Data Value is the maximum value associated "
                               "with the node."));

    graphModel()->createAttribute(tr("Variance"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._variance; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Variance">Variance</a> )"
                               "is a measure of the spread of the values associated "
                               "with the node. It is defined as ∑(𝑥-𝜇)², where 𝑥 is the value "
                               "and 𝜇 is the mean."));

    graphModel()->createAttribute(tr("Standard Deviation"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._stddev; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Standard_deviation">)"
                               "Standard Deviation</a> is a measure of the spread of the values associated "
                               "with the node. It is defined as √∑(𝑥-𝜇)², where 𝑥 is the value "
                               "and 𝜇 is the mean."));

    graphModel()->createAttribute(tr("Coefficient of Variation"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._coefVar; })
            .setValueMissingFn([this](NodeId nodeId) { return std::isnan(dataRowForNodeId(nodeId)._coefVar); })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Coefficient_of_variation">)"
                               "Coefficient of Variation</a> "
                               "is a measure of the spread of the values associated "
                               "with the node. It is defined as the standard deviation "
                               "divided by the mean."));
}

std::vector<std::tuple<NodeId, NodeId, double>> CorrelationPluginInstance::pearsonCorrelation(
        double minimumThreshold,
        const std::function<bool()>& cancelled,
        const ProgressFn& progressFn)
{
    progressFn(-1);

    uint64_t totalCost = 0;
    for(auto& row : _dataRows)
        totalCost += row.computeCostHint();

    std::atomic<uint64_t> cost(0);

    auto results = ThreadPool(QStringLiteral("PearsonCor")).concurrent_for(_dataRows.cbegin(), _dataRows.cend(),
    [&](std::vector<DataRow>::const_iterator rowAIt)
    {
        const auto& rowA = *rowAIt;
        std::vector<std::tuple<NodeId, NodeId, double>> edges;

        if(cancelled())
            return edges;

        for(const auto& rowB : make_iterator_range(rowAIt + 1, _dataRows.cend()))
        {
            double productSum = std::inner_product(rowA.cbegin(), rowA.cend(), rowB.cbegin(), 0.0);
            double numerator = (_numColumns * productSum) - (rowA._sum * rowB._sum);
            double denominator = rowA._variability * rowB._variability;

            double r = numerator / denominator;

            if(std::isfinite(r) && r >= minimumThreshold)
                edges.emplace_back(rowA._nodeId, rowB._nodeId, r);
        }

        cost += rowA.computeCostHint();
        progressFn((cost * 100) / totalCost);

        return edges;
    });

    // Returning the results might take time
    progressFn(-1);

    std::vector<std::tuple<NodeId, NodeId, double>> edges;

    for(auto& result : results.get())
        edges.insert(edges.end(), result.begin(), result.end());

    return edges;
}

bool CorrelationPluginInstance::createEdges(const std::vector<std::tuple<NodeId, NodeId, double>>& edges,
                                            const std::function<bool()>& cancelled,
                                            const ProgressFn& progressFn)
{
    progressFn(-1);
    for(auto edgeIt = edges.begin(); edgeIt != edges.end(); ++edgeIt)
    {
        if(cancelled())
            return false;

        progressFn(std::distance(edges.begin(), edgeIt) * 100 / static_cast<int>(edges.size()));

        auto& edge = *edgeIt;
        auto edgeId = graphModel()->mutableGraph().addEdge(std::get<0>(edge), std::get<1>(edge));
        _pearsonValues->set(edgeId, std::get<2>(edge));
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
    _userNodeData.setNodeIdForRowIndex(nodeId, row);
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
    _userNodeData.setNodeNamesToFirstUserDataVector(*graphModel());
    _userNodeData.exposeAsAttributes(*graphModel());
    _nodeAttributeTableModel.updateRoleNames();
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
        auto nodeId = _userNodeData.nodeIdForRowIndex(i);
        colors.append(graphModel()->nodeVisual(nodeId).outerColor());
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

    for(size_t i = 0; i < _numRows; i++)
        list.append(_userNodeData.begin()->get(i));

    return list;
}

const CorrelationPluginInstance::DataRow& CorrelationPluginInstance::dataRowForNodeId(NodeId nodeId) const
{
    return _dataRows.at(_userNodeData.rowIndexForNodeId(nodeId));
}

void CorrelationPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    _nodeAttributeTableModel.onSelectionChanged();
}

std::unique_ptr<IParser> CorrelationPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == QLatin1String("CorrelationCSV") || urlTypeName == QLatin1String("CorrelationTSV"))
        return std::make_unique<CorrelationFileParser>(this, urlTypeName);

    return nullptr;
}

void CorrelationPluginInstance::applyParameter(const QString& name, const QString& value)
{
    if(name == QLatin1String("minimumCorrelation"))
        _minimumCorrelationValue = value.toDouble();
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
}

QStringList CorrelationPluginInstance::defaultTransforms() const
{
    double defaultCorrelationValue = (_minimumCorrelationValue + 1.0) * 0.5;

    return
    {
        QString(R"("Remove Edges" where $"Pearson Correlation Value" < %1)").arg(defaultCorrelationValue),
        R"([pinned] "Remove Components" where $"Component Size" <= 1)",
    };
}

QByteArray CorrelationPluginInstance::save(IMutableGraph& graph, const ProgressFn& progressFn) const
{
    json jsonObject;

    jsonObject["numColumns"] = static_cast<int>(_numColumns);
    jsonObject["numRows"] = static_cast<int>(_numRows);
    jsonObject["userNodeData"] = _userNodeData.save(graph, progressFn);
    jsonObject["userColumnData"] =_userColumnData.save(progressFn);
    jsonObject["dataColumnNames"] = jsonArrayFrom(_dataColumnNames, progressFn);

    graph.setPhase(QObject::tr("Data"));
    jsonObject["data"] = jsonArrayFrom(_data, progressFn);

    graph.setPhase(QObject::tr("Pearson Values"));
    auto range = make_iterator_range(_pearsonValues->cbegin(), _pearsonValues->cbegin() + graph.nextEdgeId());
    jsonObject["pearsonValues"] = jsonArrayFrom(range);

    jsonObject["minimumCorrelationValue"] = _minimumCorrelationValue;
    jsonObject["transpose"] = _transpose;
    jsonObject["scaling"] = static_cast<int>(_scaling);
    jsonObject["normalisation"] = static_cast<int>(_normalisation);
    jsonObject["missingDataType"] = static_cast<int>(_missingDataType);
    jsonObject["missingDataReplacementValue"] = _missingDataReplacementValue;

    return QByteArray::fromStdString(jsonObject.dump());
}

bool CorrelationPluginInstance::load(const QByteArray& data, int dataVersion, IMutableGraph& graph,
                                     const ProgressFn& progressFn)
{
    if(dataVersion != plugin()->dataVersion())
        return false;

    json jsonObject = json::parse(data.begin(), data.end(), nullptr, false);

    if(jsonObject.is_null() || !jsonObject.is_object())
        return false;

    if(!u::contains(jsonObject, "numColumns") || !u::contains(jsonObject, "numRows"))
        return false;

    _numColumns = static_cast<size_t>(jsonObject["numColumns"].get<int>());
    _numRows = static_cast<size_t>(jsonObject["numRows"].get<int>());

    if(!u::contains(jsonObject, "userNodeData") || !u::contains(jsonObject, "userColumnData"))
        return false;

    if(!_userNodeData.load(jsonObject["userNodeData"], progressFn))
        return false;

    if(!_userColumnData.load(jsonObject["userColumnData"], progressFn))
        return false;

    progressFn(-1);

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
        progressFn(static_cast<int>((i++ * 100) / jsonData.size()));
    }

    progressFn(-1);

    for(size_t row = 0; row < _numRows; row++)
    {
        auto dataStartIndex = row * _numColumns;
        auto dataEndIndex = dataStartIndex + _numColumns;

        auto begin =_data.cbegin() + dataStartIndex;
        auto end = _data.cbegin() + dataEndIndex;
        auto computeCost = static_cast<int>(_numRows - row + 1);

        auto nodeId = _userNodeData.nodeIdForRowIndex(row);
        _dataRows.emplace_back(begin, end, nodeId, computeCost);
    }

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

        progressFn(static_cast<int>((i++ * 100) / jsonPearsonValues.size()));
    }

    progressFn(-1);

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
}

QStringList CorrelationPlugin::identifyUrl(const QUrl& url) const
{
    //FIXME actually look at the file contents
    return identifyByExtension(url);
}
