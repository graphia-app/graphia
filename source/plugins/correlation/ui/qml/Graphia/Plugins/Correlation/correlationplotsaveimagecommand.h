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

#ifndef CORRELATIONPLOTSAVEIMAGECOMMAND_H
#define CORRELATIONPLOTSAVEIMAGECOMMAND_H

#include "correlationplot.h"

#include "shared/commands/icommand.h"
#include "shared/utils/deferredexecutor.h"

#include <QObject>
#include <QString>

#include <mutex>
#include <condition_variable>
#include <deque>

class CorrelationPlotSaveImageCommand : public QObject, public ICommand
{
    Q_OBJECT

private:
    CorrelationPlot _correlationPlotItem;
    QString _baseFilename;
    QString _extension;

    DeferredExecutor _deferredExecutor;
    std::mutex _mutex;
    std::condition_variable _cv;

    struct ImageConfiguration
    {
        QString _label;
        QVector<int> _rows;
    };

    std::deque<ImageConfiguration> _images;

    void saveNextImage();

private slots:
    void executeDeferred() { _deferredExecutor.execute(); }

public:
    CorrelationPlotSaveImageCommand(const CorrelationPlot& correlationPlotItem,
        const QString& baseFilename, const QString& extension);

    QString description() const override { return QObject::tr("Saving Plot Image(s)"); }

    bool execute() override;

    bool cancellable() const override { return true; }

    void addImageConfiguration(const QString& label, const QVector<int>& rows);

signals:
    void taskAdded();
};

#endif // CORRELATIONPLOTSAVEIMAGECOMMAND_H
