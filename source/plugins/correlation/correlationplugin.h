#ifndef CORRELATIONPLUGIN_H
#define CORRELATIONPLUGIN_H

#include "shared/plugins/baseplugin.h"
#include "shared/graph/grapharray.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/iparser.h"
#include "shared/plugins/attributes.h"
#include "shared/plugins/nodeattributes.h"
#include "shared/plugins/nodeattributestablemodel.h"

#include <vector>
#include <functional>

#include <QString>

class CorrelationPluginInstance : public BasePluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QAbstractTableModel* nodeAttributes READ nodeAttributesTableModel CONSTANT)
    Q_PROPERTY(QVector<int> selectedRows READ selectionRows NOTIFY selectedRowsChanged)
    Q_PROPERTY(QStringList columnNames READ columnNames NOTIFY columnNamesChanged)
    Q_PROPERTY(QStringList rowNames READ rowNames NOTIFY rowNamesChanged)
    Q_PROPERTY(QVector<double> dataset READ attributesDataset NOTIFY datasetChanged)
    Q_PROPERTY(int columnCount MEMBER _numColumns NOTIFY columnCountChanged)
    Q_PROPERTY(int rowCount MEMBER _numRows NOTIFY rowCountChanged)

public:
    CorrelationPluginInstance();

private:
    int _numColumns = 0;
    int _numRows = 0;

    std::vector<QString> _dataColumnNames;
    QVector<double> _selectedData;
    QVector<int> _selectedRows;

    NodeAttributes _nodeAttributes;
    Attributes _columnAttributes;

    NodeAttributesTableModel _nodeAttributesTableModel;

    using DataIterator = std::vector<double>::const_iterator;
    using DataOffset = std::vector<double>::size_type;

    std::vector<double> _data;

    struct DataRow
    {
        DataRow(DataIterator begin, DataIterator end, NodeId nodeId, int computeCost) :
            _begin(begin), _end(end), _nodeId(nodeId), _cost(computeCost)
        {
            sum();
        }

        DataIterator _begin;
        DataIterator _end;

        DataIterator begin() const { return _begin; }
        DataIterator end() const { return _end; }

        NodeId _nodeId;

        int _cost;
        int computeCostHint() const { return _cost; }

        double _sum = 0.0;
        double _sumSq = 0.0;
        double _sumAllSq = 0.0;
        double _variability = 0.0;

        double _mean = 0.0;
        double _variance = 0.0;
        double _stddev = 0.0;

        void sum()
        {
            int numColumns = std::distance(_begin, _end);

            for(auto value : *this)
            {
                _sum += value;
                _sumSq += value * value;
                _mean += value / numColumns;
            }

            _sumAllSq = _sum * _sum;
            _variability = std::sqrt((numColumns * _sumSq) - _sumAllSq);

            double sum = 0.0;
            for(auto value : *this)
            {
                double x = (value - _mean);
                x *= x;
                sum += x;
            }

            _variance = sum / numColumns;
            _stddev = std::sqrt(_variance);
        }
    };

    std::vector<DataRow> _dataRows;

    std::unique_ptr<EdgeArray<double>> _pearsonValues;

    void initialise(IGraphModel* graphModel, ISelectionManager* selectionManager, const IParserThread* parserThread);

    void setDataColumnName(int column, const QString& name);
    void setData(int column, int row, double value);

    void finishDataRow(int row);

    QAbstractTableModel* nodeAttributesTableModel() { return &_nodeAttributesTableModel; }
    QStringList columnNames();
    QStringList rowNames();
    QVector<double> attributesDataset();
    QVector<int> selectionRows();

public:
    void setDimensions(int numColumns, int numRows);
    bool loadAttributes(const TabularData& tabularData, int firstDataColumn, int firstDataRow,
                        const std::function<bool()>& cancelled, const IParser::ProgressFn& progress);

    std::vector<std::tuple<NodeId, NodeId, double>> pearsonCorrelation(
            double minimumThreshold, const std::function<bool()>& cancelled,
            const IParser::ProgressFn& progress);

    void createEdges(const std::vector<std::tuple<NodeId, NodeId, double>>& edges,
                     const IParser::ProgressFn& progress);

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName);

private slots:
    void onLoadComplete();

    void onGraphChanged();
    void onSelectionChanged(const ISelectionManager* selectionManager);
signals:
    QVector<int> selectedRowsChanged();
    int rowCountChanged();
    int columnCountChanged();
    QVector<double> datasetChanged();
    QStringList columnNamesChanged();
    QStringList rowNamesChanged();
};

class CorrelationPlugin : public BasePlugin, public PluginInstanceProvider<CorrelationPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "correlationplugin.json")

public:
    CorrelationPlugin();

    QString name() const { return "Correlation"; }
    QString description() const
    {
        return tr("Calculate pearson correlations between rows of data, and create "
                  "a graph based on the resultant matrix.");
    }

    QString imageSource() const { return "qrc:///plots.svg"; }

    QStringList identifyUrl(const QUrl& url) const;

    bool editable() const { return false; }

    QString qmlPath() const { return "qrc:///qml/correlationplugin.qml"; }
};

#endif // CORRELATIONPLUGIN_H
