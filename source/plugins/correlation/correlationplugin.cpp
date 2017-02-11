#include "correlationplugin.h"

#include "correlationplotitem.h"
#include "loading/correlationfileparser.h"
#include "shared/utils/threadpool.h"
#include "shared/utils/iterator_range.h"

CorrelationPluginInstance::CorrelationPluginInstance() :
    _nodeAttributesTableModel(&_nodeAttributes)
{
    connect(this, SIGNAL(loadComplete()), this, SLOT(onLoadComplete()));
    connect(this, SIGNAL(graphChanged()), this, SLOT(onGraphChanged()));
    connect(this, SIGNAL(selectionChanged(const ISelectionManager*)),
            this, SLOT(onSelectionChanged(const ISelectionManager*)));
}

void CorrelationPluginInstance::initialise(IGraphModel* graphModel, ISelectionManager* selectionManager,
                                           ICommandManager* commandManager, const IParserThread* parserThread)
{
    BasePluginInstance::initialise(graphModel, selectionManager, commandManager, parserThread);

    _nodeAttributes.initialise(graphModel->mutableGraph());
    _nodeAttributesTableModel.initialise(selectionManager);
    _pearsonValues = std::make_unique<EdgeArray<double>>(graphModel->mutableGraph());

    graphModel->dataField(tr("Pearson Correlation Value"))
            .setFloatValueFn([this](EdgeId edgeId) { return _pearsonValues->get(edgeId); })
            .setDescription(tr("The <a href=\"https://en.wikipedia.org/wiki/Pearson_correlation_coefficient\">"
                               "Pearson Correlation Coefficient</a> is an indication of "
                               "the linear relationship between two variables."));
}

bool CorrelationPluginInstance::loadAttributes(const TabularData& tabularData, int firstDataColumn, int firstDataRow,
                                               const std::function<bool()>& cancelled, const IParser::ProgressFn& progress)
{
    progress(0);

    int numDataPoints = tabularData.numColumns() * tabularData.numRows();

    for(int rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        for(int columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            if(cancelled())
                return false;

            progress(((columnIndex + (rowIndex * tabularData.numColumns())) * 100) / numDataPoints);

            QString value = tabularData.valueAtQString(columnIndex, rowIndex);
            int dataColumnIndex = columnIndex - firstDataColumn;
            int dataRowIndex = rowIndex - firstDataRow;

            if(rowIndex == 0)
            {
                if(dataColumnIndex < 0)
                    _nodeAttributes.add(value);
                else
                    setDataColumnName(dataColumnIndex, value);
            }
            else if(dataRowIndex < 0)
            {
                if(columnIndex == 0)
                    _columnAttributes.add(value);
                else if(dataColumnIndex >= 0)
                    _columnAttributes.setValue(dataColumnIndex, tabularData.valueAtQString(0, rowIndex), value);
            }
            else
            {
                if(dataColumnIndex >= 0)
                {
                    setData(dataColumnIndex, dataRowIndex, value.toDouble());

                    if(dataColumnIndex == _numColumns - 1)
                        finishDataRow(dataRowIndex);
                }
                else
                    _nodeAttributes.setValue(dataRowIndex, tabularData.valueAtQString(columnIndex, 0), value);
            }
        }
    }

    double minMeanValue = std::numeric_limits<double>::max();
    double maxMeanValue = std::numeric_limits<double>::min();
    double minMinValue = std::numeric_limits<double>::max();
    double maxMinValue = std::numeric_limits<double>::min();
    double minMaxValue = std::numeric_limits<double>::max();
    double maxMaxValue = std::numeric_limits<double>::min();
    double minVariance = std::numeric_limits<double>::max();
    double maxVariance = std::numeric_limits<double>::min();
    double minStdDev = std::numeric_limits<double>::max();
    double maxStdDev = std::numeric_limits<double>::min();

    for(const auto& dataRow : _dataRows)
    {
        minMeanValue = std::min(minMeanValue, dataRow._mean);
        maxMeanValue = std::max(maxMeanValue, dataRow._mean);
        minMinValue = std::min(minMinValue, dataRow._minValue);
        maxMinValue = std::max(maxMinValue, dataRow._minValue);
        minMaxValue = std::min(minMaxValue, dataRow._maxValue);
        maxMaxValue = std::max(maxMaxValue, dataRow._maxValue);
        minVariance = std::min(minVariance, dataRow._variance);
        maxVariance = std::max(maxVariance, dataRow._variance);
        minStdDev = std::min(minStdDev, dataRow._stddev);
        maxStdDev = std::max(maxStdDev, dataRow._stddev);
    }

    graphModel()->dataField(tr("Mean Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._mean; })
            .setFloatMin(minMeanValue).setFloatMax(maxMeanValue)
            .setDescription(tr("The Mean Data Value is the mean of the values associated "
                               "with the node."));

    graphModel()->dataField(tr("Minimum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._minValue; })
            .setFloatMin(minMinValue).setFloatMax(maxMinValue)
            .setDescription(tr("The Minimum Data Value is the minimum value associated "
                               "with the node."));

    graphModel()->dataField(tr("Maximum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._maxValue; })
            .setFloatMin(minMaxValue).setFloatMax(maxMaxValue)
            .setDescription(tr("The Maximum Data Value is the maximum value associated "
                               "with the node."));

    graphModel()->dataField(tr("Variance"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._variance; })
            .setFloatMin(minVariance).setFloatMax(maxVariance)
            .setDescription(tr("The <a href=\"https://en.wikipedia.org/wiki/Variance\">Variance</a> "
                               "is a measure of the spread of the values associated "
                               "with the node. It is defined as ∑(x-μ)², where x is the value "
                               "and μ is the mean."));

    graphModel()->dataField(tr("Standard Deviation"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._stddev; })
            .setFloatMin(minStdDev).setFloatMax(maxStdDev)
            .setDescription(tr("The <a href=\"https://en.wikipedia.org/wiki/Standard_deviation\">"
                               "Standard Deviation</a> is a measure of the spread of the values associated "
                               "with the node. It is defined as √∑(x-μ)², where x is the value "
                               "and μ is the mean."));

    return true;
}

std::vector<std::tuple<NodeId, NodeId, double>> CorrelationPluginInstance::pearsonCorrelation(
        double minimumThreshold,
        const std::function<bool()>& cancelled,
        const IParser::ProgressFn& progress)
{
    progress(0);

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
    progress(0);
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

void CorrelationPluginInstance::setDimensions(int numColumns, int numRows)
{
    Q_ASSERT(_dataColumnNames.empty());
    Q_ASSERT(_columnAttributes.empty());
    Q_ASSERT(_nodeAttributes.empty());

    _numColumns = numColumns;
    _numRows = numRows;

    _dataColumnNames.resize(numColumns);
    _data.resize(numColumns * numRows);
}

void CorrelationPluginInstance::setDataColumnName(int column, const QString& name)
{
    Q_ASSERT(column < _numColumns);
    _dataColumnNames.at(column) = name;
}

void CorrelationPluginInstance::setData(int column, int row, double value)
{
    int index = (row * _numColumns) + column;
    Q_ASSERT(index < static_cast<int>(_data.size()));
    _data.at(index) = value;
}

void CorrelationPluginInstance::finishDataRow(int row)
{
    Q_ASSERT(row < _numRows);

    int dataStartIndex = row * _numColumns;
    int dataEndIndex = dataStartIndex + _numColumns;

    auto begin =_data.cbegin() + dataStartIndex;
    auto end = _data.cbegin() + dataEndIndex;
    auto nodeId = graphModel()->mutableGraph().addNode();
    auto computeCost = _numRows - row + 1;

    _dataRows.emplace_back(begin, end, nodeId, computeCost);
    _nodeAttributes.setNodeIdForRowIndex(nodeId, row);
}

void CorrelationPluginInstance::onLoadComplete()
{
    _nodeAttributes.setNodeNamesToFirstAttribute(*graphModel());
    _nodeAttributes.exposeToGraphModel(*graphModel());
}

QVector<double> CorrelationPluginInstance::attributesDataset()
{
    return QVector<double>::fromStdVector(_data);
}

QStringList CorrelationPluginInstance::columnNames()
{
    QStringList list;
    for(QString name : _dataColumnNames)
        list.append(name);
    return list;
}

QStringList CorrelationPluginInstance::rowNames()
{
    QStringList list;

    for(int i = 0; i < _numRows; i++)
        list.append(_nodeAttributes.begin()->get(i));

    return list;
}

const CorrelationPluginInstance::DataRow& CorrelationPluginInstance::dataRowForNodeId(NodeId nodeId) const
{
    return _dataRows.at(_nodeAttributes.rowIndexForNodeId(nodeId));
}

void CorrelationPluginInstance::onGraphChanged()
{
    if(_pearsonValues != nullptr && !_pearsonValues->empty())
    {
        double min = *std::min_element(_pearsonValues->begin(), _pearsonValues->end());
        double max = *std::max_element(_pearsonValues->begin(), _pearsonValues->end());

        graphModel()->dataField(tr("Pearson Correlation Value")).setFloatMin(min).setFloatMax(max);
    }
}

void CorrelationPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    _nodeAttributesTableModel.onSelectionChanged();
}

std::unique_ptr<IParser> CorrelationPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == "Correlation")
        return std::make_unique<CorrelationFileParser>(this);

    return nullptr;
}

void CorrelationPluginInstance::applyParameter(const QString& name, const QString& value)
{
    if(name == "minimumCorrelation")
        _minimumCorrelationValue = value.toDouble();
    else if(name == "transpose")
        _transpose = (value == "true");
}

QStringList CorrelationPluginInstance::defaultTransforms() const
{
    double defaultCorrelationValue = (_minimumCorrelationValue + 1.0) * 0.5;

    return
    {
        QString("\"Remove Edges\" where \"Pearson Correlation Value\" < %1").arg(defaultCorrelationValue),
        "[pinned] \"Remove Components\" where \"Component Size\" <= 1"
    };
}

CorrelationPlugin::CorrelationPlugin()
{
    registerUrlType("Correlation", QObject::tr("Correlation CSV File"), QObject::tr("Correlation CSV Files"), {"csv"});
    qmlRegisterType<CorrelationPlotItem>("com.kajeka", 1, 0, "CorrelationPlot");
}

QStringList CorrelationPlugin::identifyUrl(const QUrl& url) const
{
    //FIXME actually look at the file contents
    return identifyByExtension(url);
}
