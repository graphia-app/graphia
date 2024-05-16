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

#include "wasmlocalfileaccess.h"

#include <QFileDialog>
#include <QByteArray>
#include <QFile>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTimer>
#include <QDebug>

using namespace Qt::Literals::StringLiterals;

WasmLocalFileAccess::WasmLocalFileAccess(QQuickItem* parent) : QQuickItem(parent) {}

void WasmLocalFileAccess::open(const QStringList& nameFilters)
{
    auto fileContentReady = [this](const QString& filename, const QByteArray& content)
    {
        if(filename.isEmpty())
        {
            emit rejected();
            return;
        }

        QTemporaryDir dir;
        dir.setAutoRemove(false);
        auto path = dir.filePath(filename);

        QFile file(path);

        if(!dir.isValid() || !file.open(QIODevice::WriteOnly))
        {
            qDebug() << "Couldn't open" << path << "for writing";
            emit rejected();
        }

        file.write(content);
        file.close();

        _fileUrl = QUrl::fromLocalFile(path);
        emit fileUrlChanged();

        emit accepted();
    };

    QString nameFilterString = nameFilters.join('\n');
    QFileDialog::getOpenFileContent(nameFilterString, fileContentReady);
}

QUrl WasmLocalFileAccess::save(const QString& filenameHint)
{
    // Create a temporary file, and immediately delete it
    QTemporaryFile tempFile;
    tempFile.open();
    auto tempFilename = tempFile.fileName();
    tempFile.close();
    tempFile.remove();

    // The timer here is a (very) crude file system watcher that polls
    // for the target file to exist and be closed, the slightly ropey
    // assumption being that it has been written completely
    // (QFileSystemWatcher has no wasm implementation)
    auto* timer = new QTimer();
    connect(timer, &QTimer::timeout,
    [filenameHint, tempFilename, timer]
    {
        QFile file(tempFilename);

        if(file.exists() && !file.isOpen())
        {
            if(file.open(QIODevice::ReadOnly))
            {
                QFileDialog::saveFileContent(file.readAll(), filenameHint);
                file.close();
            }

            file.remove();
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start(500);

    return QUrl::fromLocalFile(tempFilename);
}
