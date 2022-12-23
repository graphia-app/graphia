/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "correlationplotsaveimagecommand.h"

#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

void CorrelationPlotSaveImageCommand::saveNextImage()
{
    auto image = _images.front();
    _deferredExecutor.enqueue([image, this]
    {
        _correlationPlotItem.setSelectedRows(image._rows);
    });

    emit taskAdded();
}

CorrelationPlotSaveImageCommand::CorrelationPlotSaveImageCommand(
    const CorrelationPlotItem& correlationPlotItem,
    const QString& baseFilename, const QString& extension) :
    _baseFilename(baseFilename), _extension(extension)
{
    correlationPlotItem.clone(_correlationPlotItem);

    connect(&_correlationPlotItem, &CorrelationPlotItem::pixmapUpdated,
    [this]
    {
        std::unique_lock<std::mutex> lock(_mutex);

        Q_ASSERT(!_images.empty());
        if(_images.empty())
        {
            std::cerr << "_images empty when saving plot image";
            return;
        }

        const auto& image = _images.front();
        QString filename;
        QUrl target;

        if(!image._label.isEmpty())
        {
            QFileInfo fileInfo(_baseFilename);
            filename = QStringLiteral("%1/%2-%3.%4")
                .arg(fileInfo.dir().path(), fileInfo.baseName(),
                image._label, fileInfo.completeSuffix());
            target = QUrl::fromLocalFile(fileInfo.dir().path());
        }
        else
        {
            filename = _baseFilename;
            target = QUrl::fromLocalFile(filename);
        }

        _correlationPlotItem.savePlotImage(filename);

        _images.pop_front();
        if(_images.empty())
        {
            QDesktopServices::openUrl(target);

            // Notify the command that we're finished
            _cv.notify_one();
            return;
        }

        saveNextImage();
    });

    connect(this, &CorrelationPlotSaveImageCommand::taskAdded,
        this, &CorrelationPlotSaveImageCommand::executeDeferred);
}

bool CorrelationPlotSaveImageCommand::execute()
{
    std::unique_lock<std::mutex> lock(_mutex);

    Q_ASSERT(!_images.empty());
    saveNextImage();

    // Wait for images to be written
    _cv.wait(lock);

    return true;
}

void CorrelationPlotSaveImageCommand::addImageConfiguration(const QString& label, const QVector<int>& rows)
{
    _images.push_back({label, rows});
}
