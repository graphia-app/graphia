#include "correlationplugin.h"

#include "correlationplotitem.h"
#include "loading/correlationfileparser.h"
#include "shared/utils/threadpool.h"
#include "shared/utils/iterator_range.h"

CorrelationPluginInstance::CorrelationPluginInstance() :
    _userNodeDataTableModel(&_userNodeData)
{
    connect(this, SIGNAL(loadSuccess()), this, SLOT(onLoadSuccess()));
    connect(this, SIGNAL(selectionChanged(const ISelectionManager*)),
            this, SLOT(onSelectionChanged(const ISelectionManager*)));
}

void CorrelationPluginInstance::initialise(IGraphModel* graphModel, ISelectionManager* selectionManager,
                                           ICommandManager* commandManager, const IParserThread* parserThread)
{
    BasePluginInstance::initialise(graphModel, selectionManager, commandManager, parserThread);

    _userNodeData.initialise(graphModel->mutableGraph());
    _userNodeDataTableModel.initialise(selectionManager);
    _pearsonValues = std::make_unique<EdgeArray<double>>(graphModel->mutableGraph());

    graphModel->attribute(tr("Pearson Correlation Value"))
            .setFloatValueFn([this](EdgeId edgeId) { return _pearsonValues->get(edgeId); })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The <a href=\"https://en.wikipedia.org/wiki/Pearson_correlation_coefficient\">"
                               "Pearson Correlation Coefficient</a> is an indication of "
                               "the linear relationship between two variables."));
}

bool CorrelationPluginInstance::loadUserData(const TabularData& tabularData, int firstDataColumn, int firstDataRow,
                                             const std::function<bool()>& cancelled, const IParser::ProgressFn& progress)
{
    Q_ASSERT(firstDataColumn > 0);
    Q_ASSERT(firstDataRow > 0);

    if(firstDataColumn <= 0 || firstDataRow <= 0)
        return false;

    progress(-1);

    uint64_t numDataPoints = tabularData.numColumns() * tabularData.numRows();

    for(int rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        for(int columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            if(cancelled())
                return false;

            uint64_t rowOffset = rowIndex * tabularData.numColumns();
            uint64_t dataPoint = columnIndex + rowOffset;
            progress(static_cast<int>((dataPoint * 100) / numDataPoints));

            QString value = tabularData.valueAtQString(columnIndex, rowIndex);
            int dataColumnIndex = columnIndex - firstDataColumn;
            int dataRowIndex = rowIndex - firstDataRow;

            if(dataColumnIndex >= _numColumns || dataRowIndex >= _numRows)
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
                    _userColumnData.setValue(dataColumnIndex, tabularData.valueAtQString(0, rowIndex), value);
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
                    _userNodeData.setValue(dataRowIndex, tabularData.valueAtQString(columnIndex, 0), value);
            }
        }
    }

    graphModel()->attribute(tr("Mean Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._mean; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The Mean Data Value is the mean of the values associated "
                               "with the node."));

    graphModel()->attribute(tr("Minimum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._minValue; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The Minimum Data Value is the minimum value associated "
                               "with the node."));

    graphModel()->attribute(tr("Maximum Data Value"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._maxValue; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The Maximum Data Value is the maximum value associated "
                               "with the node."));

    graphModel()->attribute(tr("Variance"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._variance; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The <a href=\"https://en.wikipedia.org/wiki/Variance\">Variance</a> "
                               "is a measure of the spread of the values associated "
                               "with the node. It is defined as ∑(𝑥-𝜇)², where 𝑥 is the value "
                               "and 𝜇 is the mean."));

    graphModel()->attribute(tr("Standard Deviation"))
            .setFloatValueFn([this](NodeId nodeId) { return dataRowForNodeId(nodeId)._stddev; })
            .setFlag(AttributeFlag::AutoRangeMutable)
            .setDescription(tr("The <a href=\"https://en.wikipedia.org/wiki/Standard_deviation\">"
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

void CorrelationPluginInstance::setDimensions(int numColumns, int numRows)
{
    Q_ASSERT(_dataColumnNames.empty());
    Q_ASSERT(_userColumnData.empty());
    Q_ASSERT(_userNodeData.empty());

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
    _userNodeData.setNodeIdForRowIndex(nodeId, row);
}

void CorrelationPluginInstance::onLoadSuccess()
{
    _userNodeData.setNodeNamesToFirstUserDataVector(*graphModel());
    _userNodeData.exposeAsAttributes(*graphModel());
}

QVector<double> CorrelationPluginInstance::rawData()
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
        list.append(_userNodeData.begin()->get(i));

    return list;
}

const CorrelationPluginInstance::DataRow& CorrelationPluginInstance::dataRowForNodeId(NodeId nodeId) const
{
    return _dataRows.at(_userNodeData.rowIndexForNodeId(nodeId));
}

void CorrelationPluginInstance::onSelectionChanged(const ISelectionManager*)
{
    _userNodeDataTableModel.onSelectionChanged();
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
