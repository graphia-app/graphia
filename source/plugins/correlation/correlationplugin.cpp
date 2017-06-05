#include "correlationplugin.h"

#include "correlationplotitem.h"
#include "loading/correlationfileparser.h"
#include "shared/utils/threadpool.h"
#include "shared/utils/iterator_range.h"
#include "shared/utils/qmlenum.h"

CorrelationPluginInstance::CorrelationPluginInstance()
{
    connect(this, SIGNAL(loadSuccess()), this, SLOT(onLoadSuccess()));
    connect(this, SIGNAL(selectionChanged(const ISelectionManager*)),
            this, SLOT(onSelectionChanged(const ISelectionManager*)));
    connect(this, SIGNAL(visualsChanged()), this, SIGNAL(nodeColorsChanged()));
}

void CorrelationPluginInstance::initialise(IGraphModel* graphModel, ISelectionManager* selectionManager,
                                           ICommandManager* commandManager, const IParserThread* parserThread)
{
    BasePluginInstance::initialise(graphModel, selectionManager, commandManager, parserThread);

    _userNodeData.initialise(graphModel->mutableGraph());
    _nodeAttributeTableModel.initialise(selectionManager, graphModel, &_userNodeData);
    _pearsonValues = std::make_unique<EdgeArray<double>>(graphModel->mutableGraph());

    graphModel->createAttribute(tr("Pearson Correlation Value"))
            .setFloatValueFn([this](EdgeId edgeId) { return _pearsonValues->get(edgeId); })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Pearson_correlation_coefficient">)" //
                               "Pearson Correlation Coefficient</a> is an indication of "
                               "the linear relationship between two variables."));
}

bool CorrelationPluginInstance::loadUserData(const TabularData& tabularData, size_t firstDataColumn, size_t firstDataRow,
                                             const std::function<bool()>& cancelled, const IParser::ProgressFn& progress)
{
    Q_ASSERT(firstDataColumn > 0);
    Q_ASSERT(firstDataRow > 0);

    if(firstDataColumn == 0 || firstDataRow == 0)
        return false;

    progress(-1);

    uint64_t numDataPoints = static_cast<uint64_t>(tabularData.numColumns()) * tabularData.numRows();

    TabularData scaledData = tabularData;

    for(size_t column = firstDataColumn; column < tabularData.numColumns(); column++)
    {
        for(size_t row = firstDataRow; row < tabularData.numRows(); row++)
        {
            double value = tabularData.valueAsQString(column, row).toDouble();
            value = scaleValue(value);
            scaledData.setValueAt(column, row, std::to_string(value));
        }
    }
    std::unique_ptr<Normaliser> normaliser = nullptr;

    switch(_normaliseType)
    {
    case NormaliseType::MinMax:
        normaliser = std::make_unique<MinMaxNormaliser>(scaledData, firstDataColumn, firstDataRow);
        break;
    case NormaliseType::Quantile:
        normaliser = std::make_unique<QuantileNormaliser>(scaledData, firstDataColumn, firstDataRow);
        break;
    default:
        break;
    }

    for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            if(cancelled())
                return false;

            uint64_t rowOffset = static_cast<uint64_t>(rowIndex) * tabularData.numColumns();
            uint64_t dataPoint = columnIndex + rowOffset;
            progress(static_cast<int>((dataPoint * 100) / numDataPoints));

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
                    double transformedValue = scaledData.valueAsQString(columnIndex, rowIndex).toDouble();
                    if(normaliser != nullptr)
                        transformedValue = normaliser->value(columnIndex, rowIndex);

                    setData(dataColumnIndex, dataRowIndex, transformedValue);

                    if(dataColumnIndex == static_cast<int>(_numColumns) - 1)
                        finishDataRow(dataRowIndex);
                }
                else
                    _userNodeData.setValue(dataRowIndex, tabularData.valueAsQString(columnIndex, 0), value);
            }
        }
    }

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
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Variance">Variance</a> )" //
                               "is a measure of the spread of the values associated "
                               "with the node. It is defined as ∑(𝑥-𝜇)², where 𝑥 is the value "
                               "and 𝜇 is the mean."));

    graphModel()->createAttribute(tr("Standard Deviation"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._stddev; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr(R"(The <a href="https://en.wikipedia.org/wiki/Standard_deviation">)" //
                               "Standard Deviation</a> is a measure of the spread of the values associated "
                               "with the node. It is defined as √∑(𝑥-𝜇)², where 𝑥 is the value "
                               "and 𝜇 is the mean."));

    return true;
}

std::vector<std::tuple<NodeId, NodeId, double>> CorrelationPluginInstance::pearsonCorrelation(
        double minimumThreshold,
        const std::function<bool()>& cancelled,
        const IParser::ProgressFn& progress)
{
    progress(-1);

    uint64_t totalCost = 0;
    for(auto& row : _dataRows)
        totalCost += row.computeCostHint();

    std::atomic<uint64_t> cost(0);

    auto results = ThreadPool("PearsonCor").concurrent_for(_dataRows.begin(), _dataRows.end(),
    [&](std::vector<DataRow>::iterator rowAIt)
    {
        auto& rowA = *rowAIt;
        std::vector<std::tuple<NodeId, NodeId, double>> edges;

        if(cancelled())
            return edges;

        for(auto rowB : make_iterator_range(rowAIt + 1, _dataRows.end()))
        {
            double productSum = std::inner_product(rowA.begin(), rowA.end(), rowB.begin(), 0.0);
            double numerator = (_numColumns * productSum) - (rowA._sum * rowB._sum);
            double denominator = rowA._variability * rowB._variability;

            double r = numerator / denominator;

            if(std::isfinite(r) && r >= minimumThreshold)
                edges.emplace_back(rowA._nodeId, rowB._nodeId, r);
        }

        cost += rowA.computeCostHint();
        progress((cost * 100) / totalCost);

        return edges;
    });

    // Returning the results might take time
    progress(-1);

    std::vector<std::tuple<NodeId, NodeId, double>> edges;

    for(auto& result : results.get())
        edges.insert(edges.end(), result.begin(), result.end());

    return edges;
}

bool CorrelationPluginInstance::createEdges(const std::vector<std::tuple<NodeId, NodeId, double>>& edges,
                                            const std::function<bool()>& cancelled,
                                            const IParser::ProgressFn& progress)
{
    progress(-1);
    for(auto edgeIt = edges.begin(); edgeIt != edges.end(); ++edgeIt)
    {
        if(cancelled())
            return false;

        progress(std::distance(edges.begin(), edgeIt) * 100 / static_cast<int>(edges.size()));

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
    _nodeAttributeTableModel.refreshRoleNames();
}

QVector<double> CorrelationPluginInstance::rawData()
{
    return QVector<double>::fromStdVector(_data);
}

QVector<QColor> CorrelationPluginInstance::nodeColors()
{
    QVector<QColor> colors;

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
    for(const auto& name : _dataColumnNames)
        list.append(name);
    return list;
}

QStringList CorrelationPluginInstance::rowNames()
{
    QStringList list;

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
    if(urlTypeName == "CorrelationCSV" || urlTypeName == "CorrelationTSV")
        return std::make_unique<CorrelationFileParser>(this, urlTypeName);

    return nullptr;
}

void CorrelationPluginInstance::applyParameter(const QString& name, const QString& value)
{
    if(name == "minimumCorrelation")
        _minimumCorrelationValue = value.toDouble();
    else if(name == "transpose")
        _transpose = (value == "true");
    else if(name == "scaling")
        _scaling = static_cast<ScalingType>(value.toInt());
    else if(name == "normalise")
        _normaliseType = static_cast<NormaliseType>(value.toInt());
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

CorrelationPlugin::CorrelationPlugin()
{
    registerUrlType("CorrelationCSV", QObject::tr("Correlation CSV File"), QObject::tr("Correlation CSV Files"), {"csv"});
    registerUrlType("CorrelationTSV", QObject::tr("Correlation TSV File"), QObject::tr("Correlation TSV Files"), {"tsv"});
    qmlRegisterType<CorrelationPlotItem>("com.kajeka", 1, 0, "CorrelationPlot");
}

QStringList CorrelationPlugin::identifyUrl(const QUrl& url) const
{
    //FIXME actually look at the file contents
    return identifyByExtension(url);
}
