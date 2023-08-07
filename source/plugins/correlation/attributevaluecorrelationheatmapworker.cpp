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

#include "attributevaluecorrelationheatmapworker.h"

#include "correlationdatavector.h"
#include "correlation.h"

#include <QDebug>

#include <map>
#include <vector>

AttributeValueCorrelationHeatmapWorker::AttributeValueCorrelationHeatmapWorker()
{
    connect(&_watcher, &QFutureWatcher<void>::started, this, &AttributeValueCorrelationHeatmapWorker::busyChanged);
    connect(&_watcher, &QFutureWatcher<void>::finished, this, &AttributeValueCorrelationHeatmapWorker::busyChanged);
    connect(&_watcher, &QFutureWatcher<void>::finished, [this] { _attributeName.clear(); });
}

AttributeValueCorrelationHeatmapWorker::~AttributeValueCorrelationHeatmapWorker()
{
    _watcher.waitForFinished();
}

void AttributeValueCorrelationHeatmapWorker::start(const QString& attributeName)
{
    if(_watcher.isRunning())
    {
         // Already in progress for this attribute, skip
        if(attributeName == _attributeName)
            return;

        qDebug() << "AttributeValueCorrelationHeatmapWorker::start called concurrently";
    }

    _watcher.waitForFinished();

    if(_pluginInstance == nullptr || attributeName.isEmpty())
    {
        _result.reset();
        emit resultChanged();
        return;
    }

    uncancel();
    _attributeName = attributeName;

    std::map<QString, std::vector<size_t>> map;
    for(auto row : _pluginInstance->rowsForGraph())
    {
        const auto value = _pluginInstance->attributeValueFor(_attributeName, row);
        map[value].push_back(row); // clazy:exclude=reserve-candidates
    }

    const QFuture<void> future = QtConcurrent::run([this, map = std::move(map)]
    {
        auto numColumns = _pluginInstance->numContinuousColumns();
        std::vector<double> dataRow(numColumns);
        ContinuousDataVectors cdv;

        _result._values.clear();
        _result._values.reserve(map.size());

        for(const auto& [attributeValue, rows] : map)
        {
            std::fill(dataRow.begin(), dataRow.end(), 0.0);
            double r = 1.0 / static_cast<double>(rows.size());

            for(size_t row = 0; row < rows.size(); row++)
            {
                for(size_t column = 0; column < numColumns; column++)
                {
                    auto dataValue = _pluginInstance->continuousDataAt(row, column);
                    dataRow[column] += dataValue * r;
                }
            }

            _result._values.push_back({attributeValue, rows});
            cdv.emplace_back(dataRow, NodeId{}).update();

            if(cancelled())
                break;
        }

        if(cancelled())
        {
            _result.reset();
            emit resultChanged();
            return;
        }

        PearsonCorrelation pc;
        _result._heatmap = pc.matrix(cdv, {}, this);

        if(cancelled())
        {
            _result.reset();
            emit resultChanged();
        }
        else
        {
            emit resultChanged();
            emit finished();
        }
    });

    _watcher.setFuture(future);
}
