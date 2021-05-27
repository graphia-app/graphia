/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "shared/utils/doasyncthen.h"

#include <QNetworkReply>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFileInfo>

DownloadQueue::DownloadQueue()
{
    _networkManager.setRedirectPolicy(QNetworkRequest::UserVerifiedRedirectPolicy);
    connect(&_networkManager, &QNetworkAccessManager::finished, this, &DownloadQueue::onReplyReceived);
}

DownloadQueue::~DownloadQueue()
{
    cancel();

    for(const auto& deletee : _downloaded)
    {
        if(deletee._directory)
            QDir(deletee._filename).removeRecursively();
        else
            QFile::remove(deletee._filename);
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
    else
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

    for(const auto& deletee : _downloaded)
    {
        auto downloadedFilename =
            QFileInfo(deletee._filename).canonicalFilePath();

        bool match = deletee._directory ?
            (dirname == downloadedFilename) :
            (filename == downloadedFilename);

        if(match)
            return true;
    }

    return false;
}

void DownloadQueue::start(const QUrl& url)
{
    QNetworkRequest request;
    request.setUrl(url);
    _reply = _networkManager.get(request);

    _timeoutTimer.setSingleShot(true);
    const int HTTP_TIMEOUT = 60000;
    _timeoutTimer.start(HTTP_TIMEOUT);

    // Follow any redirects
    connect(_reply, &QNetworkReply::redirected, _reply, &QNetworkReply::redirectAllowed);

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

    u::doAsync([this, reply]() -> QString
    {
        if(reply->error() != QNetworkReply::NetworkError::NoError)
        {
            if(reply->error() != QNetworkReply::NetworkError::OperationCanceledError)
                emit error(reply->url(), reply->errorString());

            return {};
        }

        _progress = -1;
        emit progressChanged(_progress);

        QString filename;

        auto contentDisposition = reply->header(QNetworkRequest::ContentDispositionHeader);
        if(contentDisposition.isValid())
        {
            filename = contentDisposition.toString();
            filename.replace(QRegularExpression(R"|(^(?:[^;]*;)*\s*filename\*?="?([^"]+)"?$)|"), R"(\1)");
        }
        else
            filename = reply->url().fileName();

        if(filename.isEmpty())
        {
            QTemporaryFile tempFile;
            tempFile.setAutoRemove(false);

            if(!tempFile.open())
                return {};

            filename = tempFile.fileName();
            _downloaded.push_back({filename, false});

            tempFile.write(reply->readAll());
            tempFile.close();
        }
        else
        {
            QTemporaryDir tempDir;
            filename = tempDir.filePath(filename);

            QSaveFile file(filename);
            if(!file.open(QIODevice::WriteOnly))
                return {};

            file.write(reply->readAll());

            if(!file.commit())
                return {};

            _downloaded.push_back({tempDir.path(), true});
            tempDir.setAutoRemove(false);
        }

        return filename;
    })
    .then([this, reply](const QString& filename)
    {
        if(!filename.isEmpty())
            emit complete(reply->url(), filename);

        reply->deleteLater();
        reset();
    });
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
}
