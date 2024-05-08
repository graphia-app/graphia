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

#include "importannotationskeydetection.h"

#include "plugins/correlation/correlationplugin.h"

#include <QFuture>
#include <QtConcurrentRun>
#include <QQmlEngine>

using namespace Qt::Literals::StringLiterals;

ImportAnnotationsKeyDetection::ImportAnnotationsKeyDetection()
{
    connect(&_watcher, &QFutureWatcher<void>::started, this, &ImportAnnotationsKeyDetection::busyChanged);
    connect(&_watcher, &QFutureWatcher<void>::finished, this, &ImportAnnotationsKeyDetection::busyChanged);
}

ImportAnnotationsKeyDetection::~ImportAnnotationsKeyDetection()
{
    _watcher.waitForFinished();
}

void ImportAnnotationsKeyDetection::start()
{
    uncancel();

    const QFuture<void> future = QtConcurrent::run([this]
    {
        size_t bestRowIndex = 0;
        int bestPercent = 0;

        QStringList columnNames;
        columnNames.reserve(static_cast<int>(_plugin->numColumns()));

        for(size_t i = 0; i < _plugin->numColumns(); i++)
            columnNames.append(_plugin->columnName(i));

        auto typeIdentities = _tabularData->rowTypeIdentities();

        for(size_t rowIndex = 0; rowIndex < _tabularData->numRows(); rowIndex++)
        {
            auto type = typeIdentities.at(rowIndex).type();
            if(type != TypeIdentity::Type::String && type != TypeIdentity::Type::Int)
                continue;

            auto percent = _tabularData->rowMatchPercentage(rowIndex, columnNames);

            // If we already have an equivalent match, prefer the earlier column
            if(percent == bestPercent && rowIndex > bestRowIndex)
                continue;

            if(percent >= bestPercent)
            {
                bestRowIndex = rowIndex;
                bestPercent = percent;
            }

            // Can't improve on 100%!
            if(bestPercent >= 100 || cancelled())
                break;
        }

        _result.clear();

        if(!cancelled())
        {
            _result.insert(u"row"_s, static_cast<int>(bestRowIndex));
            _result.insert(u"percent"_s, bestPercent);
        }

        emit resultChanged();
    });

    _watcher.setFuture(future);
}

void ImportAnnotationsKeyDetection::reset()
{
    _result = {};
    emit resultChanged();
}
