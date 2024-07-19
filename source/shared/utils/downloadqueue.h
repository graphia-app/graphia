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

#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include <map>
#include <queue>
#include <memory>

#include <QObject>
#include <QUrl>
#include <QString>
#include <QTemporaryFile>
#include <QTimer>
#include <QNetworkAccessManager>

class QNetworkReply;

class DownloadQueue : public QObject
{
    Q_OBJECT

public:
    DownloadQueue();
    ~DownloadQueue() override;

    bool add(const QUrl& url);
    void cancel();
    bool resume();

    bool idle() const { return _reply == nullptr; }
    int progress() const { return _progress; }

    bool downloaded(const QUrl& url) const;

private:
    int _progress = -1;

    QTimer _timeoutTimer;
    QNetworkAccessManager _networkManager;
    QNetworkReply* _reply = nullptr;
    std::unique_ptr<QTemporaryFile> _temporaryFile;

    std::queue<QUrl> _queue;

    const QUrl _corsUrl;

    enum class IsDir
    {
        No,
        Yes
    };

    std::map<QString, IsDir> _downloaded;

    void start(const QUrl& url);
    void onReplyReceived(QNetworkReply* reply);
    void reset();

signals:
    void idleChanged();
    void progressChanged(int progress);
    void error(const QUrl& url, const QString& text);
    void complete(const QUrl& url, const QString& fileName);
};

#endif // DOWNLOADQUEUE_H
