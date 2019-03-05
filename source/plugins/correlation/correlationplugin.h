#ifndef CORRELATIONPLUGIN_H
#define CORRELATIONPLUGIN_H

#include "shared/plugins/baseplugin.h"
#include "shared/graph/grapharray.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/iparser.h"
#include "shared/plugins/userdata.h"
#include "shared/plugins/userelementdata.h"

#include "loading/correlationfileparser.h"

#include "correlationedge.h"
#include "correlationdatarow.h"
#include "correlationnodeattributetablemodel.h"

#include <vector>
#include <functional>
#include <algorithm>
#include <utility>

#include <QString>
#include <QStringList>
#include <QVector>
#include <QVariantList>
#include <QVariantMap>
#include <QColor>
#include <QRect>
#include <QFutureWatcher>

class CorrelationPluginInstance : public BasePluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QAbstractTableModel* nodeAttributeTableModel READ nodeAttributeTableModel CONSTANT)
    Q_PROPERTY(QStringList columnNames READ columnNames NOTIFY columnNamesChanged)
    Q_PROPERTY(QStringList rowNames READ rowNames NOTIFY rowNamesChanged)

    Q_PROPERTY(QVector<double> rawData READ rawData NOTIFY rawDataChanged)
    Q_PROPERTY(QVector<QColor> nodeColors READ nodeColors NOTIFY nodeColorsChanged)

    Q_PROPERTY(QVariantList columnAnnotations MEMBER _columnAnnotations NOTIFY columnAnnotationsChanged)

    Q_PROPERTY(size_t columnCount MEMBER _numColumns NOTIFY columnCountChanged)
    Q_PROPERTY(size_t rowCount MEMBER _numRows NOTIFY rowCountChanged)

    Q_PROPERTY(QVector<int> highlightedRows MEMBER _highlightedRows
        WRITE setHighlightedRows NOTIFY highlightedRowsChanged)

public:
    CorrelationPluginInstance();

private:
    size_t _numColumns = 0;
    size_t _numRows = 0;

    std::vector<QString> _dataColumnNames;
    QVariantList _columnAnnotations;

    UserNodeData _userNodeData;
    UserData _userColumnData;

    CorrelationNodeAttributeTableModel _nodeAttributeTableModel;

    std::vector<double> _data;

    std::vector<CorrelationDataRow> _dataRows;

    std::unique_ptr<EdgeArray<double>> _pearsonValues;
    double _minimumCorrelationValue = 0.7;
    double _initialCorrelationThreshold = 0.85;
    bool _transpose = false;
    TabularData _tabularData;
    QRect _dataRect;
    ScalingType _scalingType = ScalingType::None;
    NormaliseType _normaliseType = NormaliseType::None;
    MissingDataType _missingDataType = MissingDataType::None;
    ClusteringType _clusteringType = ClusteringType::None;
    EdgeReductionType _edgeReductionType = EdgeReductionType::None;
    double _missingDataReplacementValue = 0.0;

    // The rows that are selected in the table view
    QVector<int> _highlightedRows;

    void initialise(const IPlugin* plugin, IDocument* document,
                    const IParserThread* parserThread) override;

    void setDataColumnName(size_t column, const QString& name);
    void setData(size_t column, size_t row, double value);

    void finishDataRow(size_t row);

    double imputeValue(const TabularData& tabularData,
        size_t firstDataColumn, size_t firstDataRow,
        size_t columnIndex, size_t rowIndex);
    double scaleValue(double value);

    QAbstractTableModel* nodeAttributeTableModel() { return &_nodeAttributeTableModel; }
    QStringList columnNames();
    QStringList rowNames();
    QVector<double> rawData();
    QVector<QColor> nodeColors();

    void buildColumnAnnotations();

    const CorrelationDataRow& dataRowForNodeId(NodeId nodeId) const;

    void setHighlightedRows(const QVector<int>& highlightedRows);

public:
    void setDimensions(size_t numColumns, size_t numRows);
    bool loadUserData(const TabularData& tabularData, size_t firstDataColumn, size_t firstDataRow,
                      IParser& parser);
    bool requiresNormalisation() const { return _normaliseType != NormaliseType::None; }
    void normalise(IParser* parser);
    void finishDataRows();
    void createAttributes();

    std::vector<CorrelationEdge> pearsonCorrelation(double minimumThreshold, IParser& parser);

    double minimumCorrelation() const { return _minimumCorrelationValue; }
    bool transpose() const { return _transpose; }

    bool createEdges(const std::vector<CorrelationEdge>& edges, IParser& parser);

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName) override;
    void applyParameter(const QString& name, const QVariant& value) override;
    QStringList defaultTransforms() const override;
    QStringList defaultVisualisations() const override;

    QByteArray save(IMutableGraph& graph, Progressable& progressable) const override;
    bool load(const QByteArray& data, int dataVersion, IMutableGraph& graph, IParser& parser) override;

private slots:
    void onLoadSuccess();
    void onSelectionChanged(const ISelectionManager* selectionManager);

signals:
    void rowCountChanged();
    void columnCountChanged();

    void rawDataChanged();
    void nodeColorsChanged();

    void columnAnnotationNamesChanged();
    void columnAnnotationsChanged();

    void columnNamesChanged();
    void rowNamesChanged();

    void highlightedRowsChanged();
};

class CorrelationPlugin : public BasePlugin, public PluginInstanceProvider<CorrelationPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "CorrelationPlugin.json")

public:
    CorrelationPlugin();

    QString name() const override { return QStringLiteral("Correlation"); }
    QString description() const override
    {
        return tr("Calculate pearson correlations between rows of data, and create "
                  "a graph based on the resultant matrix.");
    }

    QString imageSource() const override { return QStringLiteral("qrc:///plots.svg"); }

    int dataVersion() const override { return 2; }

    QStringList identifyUrl(const QUrl& url) const override;
    QString failureReason(const QUrl& url) const override;

    bool editable() const override { return true; }
    bool directed() const override { return false; }

    QString parametersQmlPath() const override { return QStringLiteral("qrc:///qml/CorrelationParameters.qml"); }
    QString qmlPath() const override { return QStringLiteral("qrc:///qml/CorrelationPlugin.qml"); }
};

#endif // CORRELATIONPLUGIN_H
