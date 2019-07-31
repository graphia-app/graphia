#ifndef CORRELATIONFILEPARSER_H
#define CORRELATIONFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"

#include "shared/utils/qmlenum.h"
#include "shared/utils/cancellable.h"

#include "correlation.h"
#include "correlationedge.h"
#include "correlationdatarow.h"
#include "datarecttablemodel.h"

#include <QString>
#include <QRect>
#include <QtConcurrent/QtConcurrent>

#include <memory>

// Note: the ordering of these enums is important from a save
// file point of view; i.e. only append, don't reorder

DEFINE_QML_ENUM(
    Q_GADGET, ScalingType,
    None,
    Log2,
    Log10,
    AntiLog2,
    AntiLog10,
    ArcSin);

DEFINE_QML_ENUM(
    Q_GADGET, NormaliseType,
    None,
    MinMax,
    Quantile,
    Mean,
    Standarisation,
    UnitScaling);

DEFINE_QML_ENUM(
    Q_GADGET, MissingDataType,
    Constant,
    ColumnAverage,
    RowInterpolation);

// (...although not these ones:)

DEFINE_QML_ENUM(
    Q_GADGET, ClusteringType,
    None,
    Louvain,
    MCL);

DEFINE_QML_ENUM(
    Q_GADGET, EdgeReductionType,
    None,
    KNN,
    PercentNN);

class CorrelationPluginInstance;

class CorrelationFileParser : public IParser
{
private:
    CorrelationPluginInstance* _plugin;
    QString _urlTypeName;
    TabularData _tabularData;
    QRect _dataRect;

public:
    explicit CorrelationFileParser(CorrelationPluginInstance* plugin, QString urlTypeName,
        TabularData& tabularData, QRect dataRect);

    static double imputeValue(MissingDataType missingDataType, double replacementValue,
        const TabularData& tabularData, size_t firstDataColumn, size_t firstDataRow,
                              size_t columnIndex, size_t rowIndex);
    static double scaleValue(ScalingType scalingType, double value);
    static void normalise(NormaliseType normaliseType,
        std::vector<CorrelationDataRow>& dataRows,
        IParser* parser = nullptr);

    static std::vector<CorrelationEdge> pearsonCorrelation(
        const std::vector<CorrelationDataRow>& rows,
        double minimumThreshold, IParser* parser = nullptr);

    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    static bool canLoad(const QUrl&) { return true; }
};

Q_DECLARE_METATYPE(std::shared_ptr<TabularData>)

class TabularDataParser : public QObject, public Cancellable
{
    Q_OBJECT

    Q_PROPERTY(QRect dataRect MEMBER _dataRect NOTIFY dataRectChanged)
    Q_PROPERTY(bool hasMissingValues MEMBER _hasMissingValues NOTIFY hasMissingValuesChanged)
    Q_PROPERTY(std::shared_ptr<TabularData> data MEMBER _dataPtr NOTIFY dataChanged)
    Q_PROPERTY(QAbstractTableModel* model READ tableModel NOTIFY dataRectChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool transposed READ transposed WRITE setTransposed NOTIFY transposedChanged)

    Q_PROPERTY(int progress MEMBER _progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(bool complete MEMBER _complete NOTIFY completeChanged)

    Q_PROPERTY(double minimumCorrelation MEMBER _minimumCorrelation NOTIFY parameterChanged)
    Q_PROPERTY(int correlationType MEMBER _correlationType NOTIFY parameterChanged)
    Q_PROPERTY(int correlationPolarity MEMBER _correlationPolarity NOTIFY parameterChanged)
    Q_PROPERTY(int scalingType MEMBER _scalingType NOTIFY parameterChanged)
    Q_PROPERTY(int normaliseType MEMBER _normaliseType NOTIFY parameterChanged)
    Q_PROPERTY(int missingDataType MEMBER _missingDataType NOTIFY parameterChanged)
    Q_PROPERTY(double replacementValue MEMBER _replacementValue NOTIFY parameterChanged)

    Q_PROPERTY(QVariantMap graphSizeEstimate READ graphSizeEstimate NOTIFY graphSizeEstimateChanged)
    Q_PROPERTY(bool graphSizeEstimateInProgress READ graphSizeEstimateInProgress
        NOTIFY graphSizeEstimateInProgressChanged)

private:
    QFutureWatcher<void> _autoDetectDataRectangleWatcher;
    QFutureWatcher<void> _dataParserWatcher;
    QRect _dataRect;
    bool _hasMissingValues = false;
    std::shared_ptr<TabularData> _dataPtr;
    DataRectTableModel _model;
    bool _transposed = false;

    int _progress = -1;
    bool _complete = false;

    double _minimumCorrelation = 0.0;
    int _correlationType = static_cast<int>(CorrelationType::Pearson);
    int _correlationPolarity = static_cast<int>(CorrelationPolarity::Positive);
    int _scalingType = static_cast<int>(ScalingType::None);
    int _normaliseType = static_cast<int>(NormaliseType::None);
    int _missingDataType = static_cast<int>(MissingDataType::Constant);
    double _replacementValue = 0.0;

    void setProgress(int progress);

    bool _graphSizeEstimateQueued = false;
    QFutureWatcher<QVariantMap> _graphSizeEstimateFutureWatcher;
    QVariantMap _graphSizeEstimate;

    std::vector<CorrelationDataRow> sampledDataRows(size_t numSamples);

public:
    TabularDataParser();
    Q_INVOKABLE bool parse(const QUrl& fileUrl, const QString& fileType);
    Q_INVOKABLE void cancelParse() { cancel(); }
    Q_INVOKABLE void autoDetectDataRectangle(size_t column = 0, size_t row = 0);

    Q_INVOKABLE void clearData();

    void estimateGraphSize();
    QVariantMap graphSizeEstimate() const;
    bool graphSizeEstimateInProgress() const { return _graphSizeEstimateFutureWatcher.isRunning(); }


    DataRectTableModel* tableModel();
    bool busy() const { return _autoDetectDataRectangleWatcher.isRunning() || _dataParserWatcher.isRunning(); }

    bool transposed() const;
    void setTransposed(bool transposed);

signals:
    void dataChanged();
    void dataRectChanged();
    void hasMissingValuesChanged();
    void busyChanged();
    void fileUrlChanged();
    void fileTypeChanged();
    void dataLoaded();
    void transposedChanged();
    void progressChanged();
    void completeChanged();

    void parameterChanged();
    void graphSizeEstimateChanged();
    void graphSizeEstimateInProgressChanged();

private slots:
    void onDataLoaded();
};

#endif // CORRELATIONFILEPARSER_H
