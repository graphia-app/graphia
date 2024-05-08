/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#ifndef CORRELATIONTABULARDATAPARSER_H
#define CORRELATIONTABULARDATAPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/tabulardatamodel.h"

#include "shared/utils/qmlenum.h"
#include "shared/utils/cancellable.h"

#include "shared/graph/edgelist.h"

#include "plugins/correlation/correlationtype.h"
#include "plugins/correlation/correlationdatavector.h"

#include <QQmlEngine>
#include <QString>
#include <QRect>
#include <QVariantMap>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include <memory>
#include <atomic>

class CorrelationTabularDataParser : public QObject, public Cancellable, public Progressable
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantMap dataRect READ dataRect NOTIFY dataRectChanged)
    Q_PROPERTY(bool dataHasNumericalRect MEMBER _dataHasNumericalRect NOTIFY dataHasNumericalRectChanged)
    Q_PROPERTY(std::shared_ptr<TabularData> data MEMBER _dataPtr NOTIFY dataChanged)
    Q_PROPERTY(QAbstractTableModel* model READ tableModel NOTIFY dataRectChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool transposed READ transposed WRITE setTransposed NOTIFY transposedChanged)

    Q_PROPERTY(int progress MEMBER _progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(bool complete MEMBER _complete NOTIFY completeChanged)
    Q_PROPERTY(bool failed MEMBER _failed NOTIFY failedChanged)

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
    std::pair<double, double> _numericalMinMax;
    bool _dataHasNumericalRect = false;
    std::shared_ptr<TabularData> _dataPtr = nullptr;
    TabularDataModel _model;

    int _progress = -1;
    std::atomic<Cancellable*> _cancellableParser = nullptr;
    bool _complete = false;
    bool _failed = false;

    void setProgress(int progress) override;

    QVariantMap _graphSizeEstimateQueuedParameters;

    Cancellable _graphSizeEstimateCancellable;
    QFutureWatcher<QVariantMap> _graphSizeEstimateFutureWatcher;
    QVariantMap _graphSizeEstimate;

    QVariantMap dataRect() const;

    ContinuousDataVectors sampledContinuousDataRows(size_t numSampleRows, const QVariantMap& parameters);
    DiscreteDataVectors sampledDiscreteDataRows(size_t numSampleRows, const QVariantMap& parameters);

    void waitForDataRectangleFuture();

public:
    CorrelationTabularDataParser();
    ~CorrelationTabularDataParser() override;

    Q_INVOKABLE bool parse(const QUrl& fileUrl, const QString& fileType);
    Q_INVOKABLE void cancelParse();
    Q_INVOKABLE void autoDetectDataRectangle();
    Q_INVOKABLE void setDataRectangle(size_t column, size_t row);

    Q_INVOKABLE void clearData();

    Q_INVOKABLE void estimateGraphSize(const QVariantMap& parameters);
    bool graphSizeEstimateInProgress() const { return _graphSizeEstimateFutureWatcher.isRunning(); }

    QAbstractTableModel* tableModel();
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

    void graphSizeEstimateChanged();
    void graphSizeEstimateInProgressChanged();

private slots:
    void onDataLoaded();
};

#endif // CORRELATIONTABULARDATAPARSER_H
