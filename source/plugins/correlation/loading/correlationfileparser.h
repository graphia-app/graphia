/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CORRELATIONFILEPARSER_H
#define CORRELATIONFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"

#include "shared/utils/qmlenum.h"
#include "shared/utils/cancellable.h"

#include "shared/graph/edgelist.h"

#include "correlation.h"
#include "correlationdatarow.h"
#include "datarecttablemodel.h"

#include <QString>
#include <QRect>
#include <QVariantMap>
#include <QtConcurrent/QtConcurrent>

#include <memory>
#include <atomic>

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
        const TabularData& tabularData, const QRect& dataRect, size_t columnIndex, size_t rowIndex);
    static double scaleValue(ScalingType scalingType, double value);
    static void normalise(NormaliseType normaliseType,
        std::vector<CorrelationDataRow>& dataRows,
        IParser* parser = nullptr);

    static EdgeList pearsonCorrelation(
        const std::vector<CorrelationDataRow>& rows,
        double minimumThreshold, IParser* parser = nullptr);

    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    QString log() const override;

    static bool canLoad(const QUrl&) { return true; }
};

class CorrelationTabularDataParser : public QObject, public Cancellable
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap dataRect READ dataRect NOTIFY dataRectChanged)
    Q_PROPERTY(bool dataHasNumericalRect MEMBER _dataHasNumericalRect NOTIFY dataHasNumericalRectChanged)
    Q_PROPERTY(std::shared_ptr<TabularData> data MEMBER _dataPtr NOTIFY dataChanged)
    Q_PROPERTY(QAbstractTableModel* model READ tableModel NOTIFY dataRectChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool transposed READ transposed WRITE setTransposed NOTIFY transposedChanged)

    Q_PROPERTY(int progress MEMBER _progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(bool complete MEMBER _complete NOTIFY completeChanged)
    Q_PROPERTY(bool failed MEMBER _failed NOTIFY failedChanged)

    Q_PROPERTY(double minimumCorrelation MEMBER _minimumCorrelation NOTIFY parameterChanged)
    Q_PROPERTY(int correlationType MEMBER _correlationType NOTIFY parameterChanged)
    Q_PROPERTY(int correlationPolarity MEMBER _correlationPolarity NOTIFY parameterChanged)
    Q_PROPERTY(int scalingType MEMBER _scalingType NOTIFY parameterChanged)
    Q_PROPERTY(int normaliseType MEMBER _normaliseType NOTIFY parameterChanged)
    Q_PROPERTY(int missingDataType MEMBER _missingDataType NOTIFY parameterChanged)
    Q_PROPERTY(double replacementValue MEMBER _replacementValue NOTIFY parameterChanged)

    Q_PROPERTY(QVariantMap graphSizeEstimate MEMBER _graphSizeEstimate NOTIFY graphSizeEstimateChanged)
    Q_PROPERTY(bool graphSizeEstimateInProgress READ graphSizeEstimateInProgress
        NOTIFY graphSizeEstimateInProgressChanged)

private:
    QFutureWatcher<void> _dataRectangleFutureWatcher;
    QFutureWatcher<void> _dataParserWatcher;
    QRect _dataRect;
    bool _hasMissingValues = false;
    bool _hasDiscreteValues = false;
    bool _appearsToBeContinuous = false;
    bool _dataHasNumericalRect = false;
    std::shared_ptr<TabularData> _dataPtr = nullptr;
    DataRectTableModel _model;
    bool _transposed = false;

    int _progress = -1;
    std::atomic<Cancellable*> _cancellableParser = nullptr;
    bool _complete = false;
    bool _failed = false;

    double _minimumCorrelation = 0.0;
    int _correlationType = static_cast<int>(CorrelationType::Pearson);
    int _correlationPolarity = static_cast<int>(CorrelationPolarity::Positive);
    int _scalingType = static_cast<int>(ScalingType::None);
    int _normaliseType = static_cast<int>(NormaliseType::None);
    int _missingDataType = static_cast<int>(MissingDataType::Constant);
    double _replacementValue = 0.0;

    void setProgress(int progress);

    bool _graphSizeEstimateQueued = false;

    Cancellable _graphSizeEstimateCancellable;
    QFutureWatcher<QVariantMap> _graphSizeEstimateFutureWatcher;
    QVariantMap _graphSizeEstimate;

    QVariantMap dataRect() const;

    std::vector<CorrelationDataRow> sampledDataRows(size_t numSamples);
    void waitForDataRectangleFuture();

public:
    CorrelationTabularDataParser();
    virtual ~CorrelationTabularDataParser();

    Q_INVOKABLE bool parse(const QUrl& fileUrl, const QString& fileType);
    Q_INVOKABLE void cancelParse();
    Q_INVOKABLE void autoDetectDataRectangle();
    Q_INVOKABLE void setDataRectangle(size_t column, size_t row);

    Q_INVOKABLE void clearData();

    void estimateGraphSize();
    bool graphSizeEstimateInProgress() const { return _graphSizeEstimateFutureWatcher.isRunning(); }

    DataRectTableModel* tableModel();
    bool busy() const { return _dataRectangleFutureWatcher.isRunning() || _dataParserWatcher.isRunning(); }

    bool transposed() const;
    void setTransposed(bool transposed);

signals:
    void dataChanged();
    void dataRectChanged();
    void dataHasNumericalRectChanged();
    void busyChanged();
    void dataLoaded();
    void transposedChanged();
    void progressChanged();
    void completeChanged();
    void failedChanged();

    void parameterChanged();
    void graphSizeEstimateChanged();
    void graphSizeEstimateInProgressChanged();

private slots:
    void onDataLoaded();
};

#endif // CORRELATIONFILEPARSER_H
