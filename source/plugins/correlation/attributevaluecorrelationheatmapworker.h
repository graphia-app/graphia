/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef ATTRIBUTEVALUECORRELATIONHEATMAPWORKER_H
#define ATTRIBUTEVALUECORRELATIONHEATMAPWORKER_H

#include "correlationplugin.h"

#include "shared/graph/covariancematrix.h"

#include "shared/utils/cancellable.h"

#include <QFutureWatcher>
#include <QString>
#include <QObject>

#include <vector>

struct AttributeValueCorrelationHeatmapResult
{
    struct Value
    {
        QString _value;
        std::vector<size_t> _rows;
    };

    std::vector<Value> _values;
    CovarianceMatrix _heatmap;

    void reset()
    {
        _values.clear();
        _heatmap = {};
    }
};

class AttributeValueCorrelationHeatmapWorker : public QObject, public Cancellable
{
    Q_OBJECT

    Q_PROPERTY(const CorrelationPluginInstance* pluginModel MEMBER _pluginInstance)
    Q_PROPERTY(const AttributeValueCorrelationHeatmapResult* result READ result NOTIFY resultChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

private:
    const CorrelationPluginInstance* _pluginInstance = nullptr;
    AttributeValueCorrelationHeatmapResult _result;
    QString _attributeName;
    QFutureWatcher<void> _watcher;

public:
    AttributeValueCorrelationHeatmapWorker();
    ~AttributeValueCorrelationHeatmapWorker() override;

    Q_INVOKABLE void start(const QString& attributeName);
    Q_INVOKABLE void cancel() override { Cancellable::cancel(); }

    const AttributeValueCorrelationHeatmapResult* result() const { return &_result; } // clazy:exclude=qproperty-type-mismatch

    bool busy() const { return _watcher.isRunning(); }

signals:
    void resultChanged();
    void busyChanged();
    void finished();
};

#endif // ATTRIBUTEVALUECORRELATIONHEATMAPWORKER_H
