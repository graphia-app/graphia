#include "correlationplugin.h"

#include "loading/correlationfileparser.h"
#include "shared/utils/threadpool.h"

CorrelationPluginInstance::CorrelationPluginInstance()
{
    connect(this, &CorrelationPluginInstance::graphChanged,
            this, &CorrelationPluginInstance::onGraphChanged);
}

void CorrelationPluginInstance::initialise(IGraphModel* graphModel, ISelectionManager* selectionManager)
{
    BasePluginInstance::initialise(graphModel, selectionManager);

    _dataRowIndexes = std::make_unique<NodeArray<int>>(graphModel->mutableGraph());
    _pearsonValues = std::make_unique<EdgeArray<double>>(graphModel->mutableGraph());

    graphModel->dataField(tr("Pearson Correlation Value"))
            .setFloatValueFn([this](EdgeId edgeId) { return _pearsonValues->get(edgeId); });
}

bool CorrelationPluginInstance::loadAttributes(const TabularData& tabularData, int firstDataColumn, int firstDataRow,
                                               const std::function<bool ()>& cancelled, const IParser::ProgressFn& progress)
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
                    addRowAttribute(value);
                else
                    setDataColumnName(dataColumnIndex, value);
            }
            else if(dataRowIndex < 0)
            {
                if(columnIndex == 0)
                    addColumnAttribute(value);
                else if(dataColumnIndex >= 0)
                    setColumnAttribute(dataColumnIndex, tabularData.valueAtQString(0, rowIndex), value);
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
                    setRowAttribute(dataRowIndex, tabularData.valueAtQString(columnIndex, 0), value);
            }
        }
    }

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

        for(auto rowBIt = rowAIt + 1; rowBIt != _dataRows.end(); rowBIt++)
        {
            auto& rowB = *rowBIt;

            double productSum = std::inner_product(rowA.begin(), rowA.end(), rowB.begin(), 0.0);
            double numerator = (_numColumns * productSum) - (rowA._sum * rowB._sum);
            double denominator = rowA._variability * rowB._variability;

            double r = u::clamp(-1.0, 1.0, numerator / denominator);

            if(std::isfinite(r) && r > minimumThreshold)
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

void CorrelationPluginInstance::createEdges(const std::vector<std::tuple<NodeId, NodeId, double>>& edges,
                                            const IParser::ProgressFn& progress)
{
    progress(0);
    for(auto edgeIt = edges.begin(); edgeIt != edges.end(); ++edgeIt)
    {
        progress(std::distance(edges.begin(), edgeIt) * 100 / static_cast<int>(edges.size()));

        auto& edge = *edgeIt;
        auto edgeId = graphModel()->mutableGraph().addEdge(std::get<0>(edge), std::get<1>(edge));
        _pearsonValues->set(edgeId, std::get<2>(edge));
    }

}

void CorrelationPluginInstance::setDimensions(int numColumns, int numRows)
{
    Q_ASSERT(_dataColumnNames.empty());
    Q_ASSERT(_rowAttributes.empty());
    Q_ASSERT(_columnAttributes.empty());

    _numColumns = numColumns;
    _numRows = numRows;

    _dataColumnNames.resize(numColumns);
    _data.resize(numColumns * numRows);
}

void CorrelationPluginInstance::addRowAttribute(const QString& name)
{
    Q_ASSERT(_numRows > 0);
    _rowAttributes.emplace(name, std::vector<QString>(_numRows));
}

void CorrelationPluginInstance::setRowAttribute(int row,
                                                const QString& name,
                                                const QString& attribute)
{
    Q_ASSERT(row < _numRows);
    Q_ASSERT(u::contains(_rowAttributes, name));
    _rowAttributes[name].at(row) = attribute;
}

void CorrelationPluginInstance::addColumnAttribute(const QString& name)
{
    Q_ASSERT(_numColumns > 0);
    _columnAttributes.emplace(name, std::vector<QString>(_numColumns));
}

void CorrelationPluginInstance::setColumnAttribute(int column,
                                                   const QString& name,
                                                   const QString& attribute)
{
    Q_ASSERT(column < _numColumns);
    Q_ASSERT(u::contains(_columnAttributes, name));
    _columnAttributes[name].at(column) = attribute;
}

void CorrelationPluginInstance::setDataColumnName(int column, const QString& name)
{
    Q_ASSERT(column < _numColumns);
    _dataColumnNames.at(column) = name;
}

void CorrelationPluginInstance::setData(int column, int row, double value)
{
    int index = (row * _numColumns) + column;
    Q_ASSERT(index < _data.size());
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
}

void CorrelationPluginInstance::onGraphChanged()
{
    if(_pearsonValues != nullptr)
    {
        float min = *std::min_element(_pearsonValues->begin(), _pearsonValues->end());
        float max = *std::max_element(_pearsonValues->begin(), _pearsonValues->end());

        graphModel()->dataField(tr("Pearson Correlation Value")).setFloatMin(min).setFloatMax(max);
    }
}

std::unique_ptr<IParser> CorrelationPluginInstance::parserForUrlTypeName(const QString& urlTypeName)
{
    if(urlTypeName == "Correlation")
        return std::make_unique<CorrelationFileParser>(this);

    return nullptr;
}

CorrelationPlugin::CorrelationPlugin()
{
    registerUrlType("Correlation", QObject::tr("Correlation CSV File"), QObject::tr("Correlation CSV Files"), {"csv"});
}

QStringList CorrelationPlugin::identifyUrl(const QUrl& url) const
{
    //FIXME actually look at the file contents
    return identifyByExtension(url);
}

std::unique_ptr<IPluginInstance> CorrelationPlugin::createInstance()
{
    return std::make_unique<CorrelationPluginInstance>();
}
