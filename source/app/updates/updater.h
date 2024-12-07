/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef UPDATER_H
#define UPDATER_H

#include "app/preferences.h"

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QString>
#include <QStringList>

#include <atomic>

class QByteArray;
class QNetworkReply;

class Updater : public QObject
{
    Q_OBJECT

public:
    Updater();
    ~Updater() override;

    void enableAutoBackgroundCheck();
    void disableAutoBackgroundCheck();

    int progress() const { return _progress; }
    void cancelUpdateDownload();

    static bool updateAvailable();
    static bool showUpdatePrompt(const QStringList& arguments);

    static QString updateStatus();
    static void resetUpdateStatus();

public slots:
    void startBackgroundUpdateCheck();

private:
    QTimer _backgroundCheckTimer;

    QTimer _timeoutTimer;
    QNetworkAccessManager _networkManager;
    QNetworkReply* _reply = nullptr;
    QString _updateString;
    int _progress = -1;

    enum class State
    {
        Idle,
        Update,
        File
    } _state = State::Idle;

    QString _fileName;
    QString _checksum;

    PreferencesWatcher _preferencesWatcher;

    void downloadUpdate(QNetworkReply* reply);
    void saveUpdate(QNetworkReply* reply);

    void onPreferenceChanged(const QString& key, const QVariant&);

private slots:
    void onReplyReceived(QNetworkReply* reply);
    void onTimeout();

signals:
    void noNewUpdateAvailable(bool existing);
    void updateDownloaded();
    void progressChanged();

    void changeLogStored();
};

#endif // UPDATER_H
