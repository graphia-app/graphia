#ifndef CORRELATIONPLUGIN_H
#define CORRELATIONPLUGIN_H

#include "shared/plugins/baseplugin.h"
#include "shared/graph/grapharray.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/iparser.h"
#include "shared/plugins/userdata.h"
#include "shared/plugins/userelementdata.h"

#include "correlationnodeattributetablemodel.h"
#include "minmaxnormaliser.h"
#include "quantilenormaliser.h"

#include <vector>
#include <functional>
#include <algorithm>
#include <utility>

#include <QString>
#include <QStringList>
#include <QVector>
#include <QColor>

DEFINE_QML_ENUM(Q_GADGET, ScalingType,
                None,
                Log2,
                Log10,
                AntiLog2,
                AntiLog10,
                ArcSin);

DEFINE_QML_ENUM(Q_GADGET, NormaliseType,
                None,
                MinMax,
                Quantile);

DEFINE_QML_ENUM(Q_GADGET, MissingDataType,
                None,
                Constant,
                ColumnAverage,
                RowInterpolation);

class CorrelationPluginInstance : public BasePluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QAbstractTableModel* nodeAttributeTableModel READ nodeAttributeTableModel CONSTANT)
    Q_PROPERTY(QStringList columnNames READ columnNames NOTIFY columnNamesChanged)
    Q_PROPERTY(QStringList rowNames READ rowNames NOTIFY rowNamesChanged)
    Q_PROPERTY(QVector<double> rawData READ rawData NOTIFY rawDataChanged)
    Q_PROPERTY(QVector<QColor> nodeColors READ nodeColors NOTIFY nodeColorsChanged)
    Q_PROPERTY(size_t columnCount MEMBER _numColumns NOTIFY columnCountChanged)
    Q_PROPERTY(size_t rowCount MEMBER _numRows NOTIFY rowCountChanged)

public:
    CorrelationPluginInstance();

private:
    size_t _numColumns = 0;
    size_t _numRows = 0;

    std::vector<QString> _dataColumnNames;

    UserNodeData _userNodeData;
    UserData _userColumnData;

    CorrelationNodeAttributeTableModel _nodeAttributeTableModel;

    using ConstDataIterator = std::vector<double>::const_iterator;
    using DataIterator = std::vector<double>::iterator;
    using DataOffset = std::vector<double>::size_type;

    std::vector<double> _data;

    struct DataRow
    {
        DataRow() = default;
        DataRow(const DataRow&) = default;
        DataRow(ConstDataIterator _begin, ConstDataIterator _end, NodeId nodeId, int computeCost) :
            _data(_begin, _end), _nodeId(nodeId), _cost(computeCost)
        {
            _sortedData = _data;
            std::sort(_sortedData.begin(), _sortedData.end());

            sum();
        }

        std::vector<double> _data;
        std::vector<double> _sortedData;

        DataIterator begin() { return _data.begin(); }
        DataIterator end() { return _data.end(); }

        ConstDataIterator cbegin() const { return _data.cbegin(); }
        ConstDataIterator cend() const { return _data.cend(); }

        DataIterator sortedBegin() { return _sortedData.begin(); }
        DataIterator sortedEnd() { return _sortedData.end(); }

        NodeId _nodeId;

        int _cost = 0;
        int computeCostHint() const { return _cost; }

        double _sum = 0.0;
        double _sumSq = 0.0;
        double _sumAllSq = 0.0;
        double _variability = 0.0;

        double _mean = 0.0;
        double _variance = 0.0;
        double _stddev = 0.0;
        double _coefVar = 0.0;

        double _minValue = std::numeric_limits<double>::max();
        double _maxValue = std::numeric_limits<double>::lowest();

        void sum()
        {
            auto numColumns = std::distance(begin(), end());

            for(auto value : *this)
            {
                _sum += value;
                _sumSq += value * value;
                _mean += value / numColumns;
                _minValue = std::min(_minValue, value);
                _maxValue = std::max(_maxValue, value);
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
            _coefVar = _stddev / (_mean > 0.0 ? _mean : 0.0);
        }
    };

    std::vector<DataRow> _dataRows;

    struct CorrelationEdge
    {
        NodeId _source;
        NodeId _target;
        double _r;
    };

    std::unique_ptr<EdgeArray<double>> _pearsonValues;
    double _minimumCorrelationValue = 0.7;
    bool _transpose = false;
    ScalingType _scaling = ScalingType::None;
    NormaliseType _normalisation = NormaliseType::None;
    MissingDataType _missingDataType = MissingDataType::None;
    double _missingDataReplacementValue = 0.0;

    void initialise(const IPlugin* plugin, IDocument* document,
                    const IParserThread* parserThread) override;

    void setDataColumnName(size_t column, const QString& name);
    void setData(size_t column, size_t row, double value);

    void finishDataRow(size_t row);

    double scaleValue(double value);

    QAbstractTableModel* nodeAttributeTableModel() { return &_nodeAttributeTableModel; }
    QStringList columnNames();
    QStringList rowNames();
    QVector<double> rawData();
    QVector<QColor> nodeColors();

    const DataRow& dataRowForNodeId(NodeId nodeId) const;

public:
    void setDimensions(size_t numColumns, size_t numRows);
    bool loadUserData(const TabularData& tabularData, size_t firstDataColumn, size_t firstDataRow,
                      Cancellable& cancellable, const ProgressFn& progressFn);
    bool requiresNormalisation() const { return _normalisation != NormaliseType::None; }
    bool normalise(Cancellable& cancellable, const ProgressFn& progressFn);
    void finishDataRows();
    void createAttributes();

    std::vector<CorrelationEdge> pearsonCorrelation(double minimumThreshold,
        Cancellable& cancellable, const ProgressFn& progressFn);

    double minimumCorrelation() const { return _minimumCorrelationValue; }
    bool transpose() const { return _transpose; }

    bool createEdges(const std::vector<CorrelationEdge>& edges,
                     Cancellable& cancellable,
                     const ProgressFn& progressFn);

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName) override;
    void applyParameter(const QString& name, const QString& value) override;
    QStringList defaultTransforms() const override;

    QByteArray save(IMutableGraph& graph, const ProgressFn& progressFn) const override;
    bool load(const QByteArray& data, int dataVersion, IMutableGraph& graph,
              const ProgressFn& progressFn) override;

private slots:
    void onLoadSuccess();
    void onSelectionChanged(const ISelectionManager* selectionManager);

signals:
    void rowCountChanged();
    void columnCountChanged();
    void rawDataChanged();
    void nodeColorsChanged();
    void columnNamesChanged();
    void rowNamesChanged();
};

class CorrelationPlugin : public BasePlugin, public PluginInstanceProvider<CorrelationPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "correlationplugin.json")

public:
    CorrelationPlugin();

    QString name() const override { return QStringLiteral("Correlation"); }
    QString description() const override
    {
        return tr("Calculate pearson correlations between rows of data, and create "
                  "a graph based on the resultant matrix.");
    }

    QString imageSource() const override { return QStringLiteral("qrc:///plots.svg"); }

    int dataVersion() const override { return 1; }

    QStringList identifyUrl(const QUrl& url) const override;

    bool editable() const override { return true; }

    QString parametersQmlPath() const override { return QStringLiteral("qrc:///qml/parameters.qml"); }
    QString qmlPath() const override { return QStringLiteral("qrc:///qml/correlationplugin.qml"); }
};

#endif // CORRELATIONPLUGIN_H
