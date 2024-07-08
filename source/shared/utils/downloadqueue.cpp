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

#include "downloadqueue.h"

#include <QNetworkReply>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QRegularExpression>

#include <algorithm>
#include <iostream>
#include <cstdio>

DownloadQueue::DownloadQueue()
{
    _networkManager.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(&_networkManager, &QNetworkAccessManager::finished, this, &DownloadQueue::onReplyReceived);
}

DownloadQueue::~DownloadQueue()
{
    cancel();

    for(const auto& [deletee, isDir] : _downloaded)
    {
        if(isDir == IsDir::Yes)
            QDir(deletee).removeRecursively();
        else
            QFile::remove(deletee);
    }
}

bool DownloadQueue::add(const QUrl& url)
{
    // If idle, start downloading
    if(idle())
    {
        start(url);
        return true;
    }

    _queue.push(url);

    return false;
}

void DownloadQueue::cancel()
{
    // Clear _queue
    _queue = {};

    if(!idle())
    {
        _reply->abort();
        reset();
    }
}

bool DownloadQueue::resume()
{
    // If idle and non empty, start downloading
    if(!_queue.empty() && idle())
    {
        const auto& url = _queue.front();
        start(url);
        _queue.pop();

        return true;
    }

    return false;
}

bool DownloadQueue::downloaded(const QUrl& url) const
{
    auto fileinfo = QFileInfo(url.toLocalFile());
    auto filename = fileinfo.canonicalFilePath();
    auto dirname = fileinfo.canonicalPath();

    return std::any_of(_downloaded.begin(), _downloaded.end(), [&](const auto& deletee)
    {
        auto downloadedFilename =
            QFileInfo(deletee.first).canonicalFilePath();

        const bool match = deletee.second == IsDir::Yes ?
            (dirname == downloadedFilename) :
            (filename == downloadedFilename);

        return match;
    });
}

void DownloadQueue::start(const QUrl& url)
{
    _temporaryFile = std::make_unique<QTemporaryFile>();
    if(!_temporaryFile->open())
    {
        qDebug() << "Failed to create temporary file while downloading" << url.toString();
        return;
    }
    _downloaded.emplace(_temporaryFile->fileName(), IsDir::No);
    _temporaryFile->setAutoRemove(false);

    QNetworkRequest request;
    request.setUrl(url);
    _reply = _networkManager.get(request);

    _timeoutTimer.setSingleShot(true);
    const int HTTP_TIMEOUT = 60000;
    _timeoutTimer.start(HTTP_TIMEOUT);

    connect(_reply, &QNetworkReply::downloadProgress, [this]
    (qint64 bytesReceived, qint64 bytesTotal)
    {
        _timeoutTimer.start(HTTP_TIMEOUT);

        auto oldProgress = _progress;

        if(bytesTotal > 0)
            _progress = static_cast<int>((bytesReceived * 100u) / bytesTotal);

        if(_progress != oldProgress)
            emit progressChanged(_progress);
    });

    connect(_reply, &QNetworkReply::readyRead, [this]
    {
        _temporaryFile->write(_reply->readAll());
    });

    connect(&_timeoutTimer, &QTimer::timeout, [this]
    {
        _reply->abort();
        reset();
    });

    _progress = 0;
    emit progressChanged(_progress);
    emit idleChanged();
}

void DownloadQueue::onReplyReceived(QNetworkReply* reply)
{
    Q_ASSERT(reply == _reply);

    if(_timeoutTimer.isActive())
        _timeoutTimer.stop();

    if(_reply->error() != QNetworkReply::NetworkError::NoError)
    {
        auto httpCode = static_cast<int>(_reply->error());
        auto networkErrorString = QVariant::fromValue(_reply->error()).toString();
        auto errorString = QString("Network Error: %1 (%2/%3)")
            .arg(_reply->errorString())
            .arg(httpCode)
            .arg(networkErrorString);

        if(_reply->error() != QNetworkReply::NetworkError::OperationCanceledError)
            emit error(_reply->url(), errorString);

        _reply->deleteLater();
        reset();
        return;
    }

    _progress = -1;
    emit progressChanged(_progress);

    QString filename;

    auto contentDisposition = _reply->header(QNetworkRequest::ContentDispositionHeader);
    if(contentDisposition.isValid())
    {
        filename = contentDisposition.toString();
        static const QRegularExpression re(QStringLiteral(
            R"|(^(?:[^;]*;)*\s*filename\*?="?([^"]+)"?$)|"));
        filename.replace(re, QStringLiteral(R"(\1)"));
    }
    else
        filename = _reply->url().fileName();

    if(filename.isEmpty())
    {
        // Can't figure out an appropriate name from the reply,
        // so resort to using the existing temp file name
        filename = _temporaryFile->fileName();
        _temporaryFile->close();
    }
    else
    {
        QTemporaryDir tempDir;
        filename = tempDir.filePath(filename);
        tempDir.setAutoRemove(false);

        if(!_temporaryFile->rename(filename))
        {
            qDebug() << "Failed to rename" << _temporaryFile->fileName() << "to" << filename;
            return;
        }

        _downloaded.erase(_temporaryFile->fileName());
        _downloaded.emplace(tempDir.path(), IsDir::Yes);
    }

    if(!filename.isEmpty())
        emit complete(_reply->url(), filename);

    _reply->deleteLater();
    reset();
}

void DownloadQueue::reset()
{
    if(_progress >= 0)
    {
        _progress = -1;
        emit progressChanged(_progress);
    }

    if(_reply != nullptr)
    {
        _reply = nullptr;
        emit idleChanged();
    }

    if(_temporaryFile != nullptr)
        _temporaryFile = nullptr;
}
