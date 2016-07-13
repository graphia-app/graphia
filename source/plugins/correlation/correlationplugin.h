#ifndef CORRELATIONPLUGIN_H
#define CORRELATIONPLUGIN_H

#include "shared/plugins/baseplugin.h"
#include "shared/graph/grapharray.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/iparser.h"

#include <vector>
#include <map>
#include <functional>

class CorrelationPluginInstance : public BasePluginInstance
{
    Q_OBJECT

public:
    CorrelationPluginInstance();

private:
    void initialise(IGraphModel* graphModel, ISelectionManager* selectionManager);

    void addRowAttribute(const QString& name);
    void setRowAttribute(int row, const QString& name, const QString& attribute);
    void addColumnAttribute(const QString& name);
    void setColumnAttribute(int column, const QString& name, const QString& attribute);

    void setDataColumnName(int column, const QString& name);
    void setData(int column, int row, double value);

    void finishDataRow(int row);

    int _numColumns = 0;
    int _numRows = 0;

    std::vector<QString> _dataColumnNames;
    std::map<QString, std::vector<QString>> _rowAttributes;
    std::map<QString, std::vector<QString>> _columnAttributes;

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

        DataIterator begin() { return _begin; }
        DataIterator end() { return _end; }

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

    std::unique_ptr<NodeArray<int>> _dataRowIndexes;
    std::unique_ptr<EdgeArray<double>> _pearsonValues;

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
    void onGraphChanged();
};

class CorrelationPlugin : public BasePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "correlationplugin.json")

public:
    CorrelationPlugin();

    QString name() const { return "Correlation"; }
    QString description() const
    {
        return tr("Correlations.");
    }

    QStringList identifyUrl(const QUrl& url) const;

    bool editable() const { return false; }

    std::unique_ptr<IPluginInstance> createInstance();

    QString qmlPath() const { return "qrc:///qml/correlationplugin.qml"; }
};

#endif // CORRELATIONPLUGIN_H
