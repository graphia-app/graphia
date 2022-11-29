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

#include "build_defines.h"

#include "updater.h"

#include "shared/updates/updates.h"

#include "application.h"
#include "preferences.h"

#include "shared/utils/container.h"
#include "shared/utils/string.h"
#include "shared/utils/checksum.h"
#include "shared/utils/doasyncthen.h"

#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QTemporaryFile>

#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QHttpPart>

#include <iostream>
#include <vector>
#include <utility>

#include <json_helper.h>

#ifdef _DEBUG
//#define DEBUG_BACKGROUND_UPDATE

#ifndef DEBUG_BACKGROUND_UPDATE
#define DISABLE_BACKGROUND_UPDATE
#endif
#endif

static QString tempUpdaterPath()
{
    auto appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if(appDataLocation.isEmpty())
        return {};

    return appDataLocation + QStringLiteral("/Updater");
}

Updater::Updater()
{
    // Remove any existing temporary Updater instance
    QDir dir(tempUpdaterPath());
    if(dir.exists())
        dir.removeRecursively();

    _networkManager.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    connect(&_networkManager, &QNetworkAccessManager::finished,
        this, &Updater::onReplyReceived);

    connect(&_timeoutTimer, &QTimer::timeout, this, &Updater::onTimeout);
    connect(&_backgroundCheckTimer, &QTimer::timeout,
        this, &Updater::startBackgroundUpdateCheck);

    connect(&_preferencesWatcher, &PreferencesWatcher::preferenceChanged,
        this, &Updater::onPreferenceChanged);
}

void Updater::onPreferenceChanged(const QString& key, const QVariant&)
{
    if(key == QStringLiteral("misc/autoBackgroundUpdateCheck"))
        enableAutoBackgroundCheck(); // Also disables
}

Updater::~Updater()
{
    cancelUpdateDownload();
}

void Updater::enableAutoBackgroundCheck()
{
    if(!u::pref(QStringLiteral("misc/autoBackgroundUpdateCheck")).toBool())
    {
        disableAutoBackgroundCheck();
        return;
    }

    if(_backgroundCheckTimer.isActive())
        return;

#ifndef DISABLE_BACKGROUND_UPDATE
    const int BACKGROUND_CHECK_INTERVAL = 60 * 60000;
    _backgroundCheckTimer.start(BACKGROUND_CHECK_INTERVAL);

    // Do a check right away, too
    startBackgroundUpdateCheck();
#endif
}

void Updater::disableAutoBackgroundCheck()
{
    if(!_backgroundCheckTimer.isActive())
        return;

#ifndef DISABLE_BACKGROUND_UPDATE
    _backgroundCheckTimer.stop();
    cancelUpdateDownload();
#endif
}

QString Updater::updateStatus()
{
    QString status;
    latestUpdateJson(&status);
    return status;
}

void Updater::resetUpdateStatus()
{
    clearUpdateStatus();
}

void Updater::startBackgroundUpdateCheck()
{
    if(_timeoutTimer.isActive() || _state != State::Idle)
        return;

    _timeoutTimer.setSingleShot(true);
    const int HTTP_TIMEOUT = 60000;
    _timeoutTimer.start(HTTP_TIMEOUT);

    // Work around for QTBUG-31652 (PRNG takes time to initialise) to
    // effectively call QNetworkAccessManager::post asynchronously
    QTimer::singleShot(0, [this]
    {
        QNetworkRequest request;
        request.setUrl(QUrl(u::pref(QStringLiteral("servers/updates")).toString()));

        _state = Updater::State::Update;
        Q_ASSERT(_reply == nullptr);
        _reply = _networkManager.get(request);
    });
}

void Updater::downloadUpdate(QNetworkReply* reply)
{
    u::doAsync([this, reply]
    {
        auto updateString = reply->readAll();
        auto update = updateStringToJson(updateString);

        if(update.is_null())
        {
            update["error"] = QStringLiteral("none");
            return update;
        }

        bool urlIsValid = false;
        if(u::contains(update, "url"))
        {
            const QUrl url = update["url"];
            urlIsValid = url.isValid();
        }

        if(!update.is_object() || !urlIsValid)
        {
            // Update isn't valid, for whatever reason
            update["error"] = QStringLiteral("invalid");
            std::cerr << "Update not valid: " <<
                (!urlIsValid ? update["url"] : "not an object") << "\n";
            return update;
        }

        if(update["version"] == VERSION)
        {
            // The update is the same version as what we're running
            update["error"] = QStringLiteral("running");
            return update;
        }

        QString status;
        auto oldUpdate = latestUpdateJson(&status);

        const bool alreadyHaveUpdate = oldUpdate.is_object() && u::contains(oldUpdate, "version") &&
            oldUpdate["version"] == update["version"];

        // We've got an update marked as installed, but it doesn't match the running version
        const bool runningVersionDoesntMatchInstalledVersion = (status == QStringLiteral("installed")) &&
            VERSION != oldUpdate["version"];

        const bool previousAttemptFailed = (status == QStringLiteral("failed"));

        if(alreadyHaveUpdate && !previousAttemptFailed && !runningVersionDoesntMatchInstalledVersion)
        {
            // We already have this update, and it was either successfully
            // installed, the user has skipped it, or they haven't dealt
            // with it yet
            update["error"] = status.isEmpty() ? QStringLiteral("existing") : status;
        }
        else
            _updateString = updateString;

        return update;
    })
    .then([this, reply](const json& update)
    {
        if(u::contains(update, "version") && update["version"] == VERSION)
        {
            // If the update matches the running version, store
            // the changeLog so the user can view it later
            json changeLog;

            changeLog["text"] = update["changeLog"];
            changeLog["images"] = update["images"];

            storeChangeLogJson(QString::fromStdString(changeLog.dump()));
            emit changeLogStored();
        }

        if(u::contains(update, "error"))
        {
            const QString error = update["error"];
            emit noNewUpdateAvailable(error ==
                QStringLiteral("existing"));
            _state = State::Idle;
            return;
        }

        const QUrl url = update["url"];
        QNetworkRequest request(url);

        if(u::contains(update, "httpUserName") && u::contains(update, "httpPassword"))
        {
            const QString concatenatedCredentials = QStringLiteral("%1:%2")
                .arg(QString::fromStdString(update["httpUserName"]),
                QString::fromStdString(update["httpPassword"]));

            const QByteArray data = concatenatedCredentials.toLocal8Bit().toBase64();
            const QString headerData = "Basic " + data;
            request.setRawHeader("Authorization", headerData.toLocal8Bit());
        }

        _fileName = fullyQualifiedInstallerFileName(update);
        _checksum = QString::fromStdString(update["installerChecksum"]);

        _state = Updater::State::File;
        Q_ASSERT(_reply == nullptr);
        _reply = _networkManager.get(request);

        connect(_reply, &QNetworkReply::downloadProgress, [this]
        (qint64 bytesReceived, qint64 bytesTotal)
        {
            if(bytesTotal > 0)
                _progress = static_cast<int>((bytesReceived * 100u) / bytesTotal);

            emit progressChanged();
        });

        reply->deleteLater();
    });
}

void Updater::saveUpdate(QNetworkReply* reply)
{
    u::doAsync([this, reply]
    {
        _progress = -1;
        emit progressChanged();

        // Recursively delete existing update files
        auto location = updatesLocation();
        if(!location.isEmpty())
        {
            const QDir dir(location);
            if(dir.exists())
            {
                const auto& subFileNames = dir.entryList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot);
                for(const auto& subFileName : subFileNames)
                {
                    const QFileInfo info(QStringLiteral("%1/%2").arg(location, subFileName));

                    if(info.isDir())
                        QDir(info.absoluteFilePath()).removeRecursively();
                    else
                        QFile(info.absoluteFilePath()).remove();
                }
            }
        }

        QTemporaryFile tempFile;
        if(!tempFile.open())
        {
            std::cerr << "Failed to open temporary file for update\n";
            return false;
        }

        tempFile.write(reply->readAll());
        tempFile.close();

        if(!u::sha256ChecksumMatchesFile(tempFile.fileName(), _checksum))
        {
            std::cerr << "Downloaded installer " << tempFile.fileName().toStdString() <<
                " does not match SHA256 checksum: " << _checksum.toStdString() << "\n";
            return false;
        }

        QDir().mkpath(QFileInfo(_fileName).absolutePath());
        tempFile.rename(_fileName);
        tempFile.setAutoRemove(false);

        return true;
    })
    .then([this, reply](bool success)
    {
        if(success)
        {
            storeUpdateJson(_updateString);
            emit updateDownloaded();
        }
        else
            emit noNewUpdateAvailable(false);

        _updateString.clear();
        _state = State::Idle;

        reply->deleteLater();
    });
}

void Updater::cancelUpdateDownload()
{
    _updateString.clear();

    if(_reply != nullptr)
        _reply->abort();
}

void Updater::onReplyReceived(QNetworkReply* reply)
{
    if(_timeoutTimer.isActive())
        _timeoutTimer.stop();

    if(reply->error() == QNetworkReply::NetworkError::NoError)
    {
        switch(_state)
        {
        case Updater::State::Update:
            downloadUpdate(reply);
            break;

        case Updater::State::File:
            saveUpdate(reply);
            break;

        default:
            break;
        };
    }
    else
    {
        auto error = reply->errorString();
        std::cerr << "Error while retrieving update from server (" << error.toStdString() << ").\n";

        _state = Updater::State::Idle;
        emit noNewUpdateAvailable(false);
    }

    _reply = nullptr;
}

void Updater::onTimeout()
{
    cancelUpdateDownload();
}

bool Updater::updateAvailable()
{
    QString status;

    auto latestUpdate = latestUpdateJson(&status);
    if(!latestUpdate.is_object())
        return false;

    // Don't allow update to the running version
    if(latestUpdate["version"] == VERSION)
        return false;

    // Update has already been skipped or installed
    if(status == QStringLiteral("skipped") || status == QStringLiteral("installed"))
        return false;

    return true;
}

#ifdef Q_OS_WIN
static bool copyUpdaterToTemporaryLocation(QString& updaterExeFileName)
{
    // Copy the updater to a temporary location... this is necessary
    // on Windows, where a running executable cannot be overwritten
    QFileInfo updaterExeFileInfo(updaterExeFileName);

    const auto& sourceDir = QDir(updaterExeFileInfo.path());
    auto destinationDir = QDir(tempUpdaterPath());

    if(destinationDir.exists())
        destinationDir.removeRecursively();

    if(!QDir().mkpath(destinationDir.path()))
    {
        std::cerr << "Failed to create temporary updater directory.\n";
        return false;
    }

    auto temporaryUpdaterExeFileName = QStringLiteral("%1/%2")
        .arg(destinationDir.path(), updaterExeFileInfo.fileName());

    std::vector<std::pair<QString, QString>> filesToCopy;

    auto updaterDepsFileName = QStringLiteral("%1/%2.deps")
        .arg(updaterExeFileInfo.path(),
        updaterExeFileInfo.completeBaseName());

    if(QFile::exists(updaterDepsFileName))
    {
        QFile updaterDepsFile(updaterDepsFileName);
        if(!updaterDepsFile.open(QIODevice::ReadOnly))
            return false;

        QTextStream textStream(&updaterDepsFile);
        while(!textStream.atEnd())
        {
            auto line = textStream.readLine();
            auto sourceFileName = QStringLiteral("%1/%2")
                .arg(sourceDir.path(), line);
            auto destinationFileName = QStringLiteral("%1/%2")
                .arg(destinationDir.path(), line);

            filesToCopy.emplace_back(sourceFileName,
                destinationFileName);
        }

        updaterDepsFile.close();
    }
    else
    {
        // No deps file, just copy the executable and hope for the best
        filesToCopy.emplace_back(updaterExeFileName, temporaryUpdaterExeFileName);
    }

    for(const auto& fileToCopy : filesToCopy)
    {
        // Ensure destination directory exists
        QDir().mkpath(QFileInfo(fileToCopy.second).path());

        if(!QFile::copy(fileToCopy.first, fileToCopy.second))
        {
            std::cerr << "Failed to copy '" << fileToCopy.first.toStdString()
                << "' to '" << fileToCopy.second.toStdString() << "'\n";
            return false;
        }
    }

    updaterExeFileName = temporaryUpdaterExeFileName;
    return true;
}
#endif

bool Updater::showUpdatePrompt(const QStringList& arguments)
{
    auto updaterExeFileName = Application::resolvedExe(QStringLiteral("Updater"));

#ifdef Q_OS_WIN
    if(!copyUpdaterToTemporaryLocation(updaterExeFileName))
        return false;
#endif

    if(!QFile::exists(updaterExeFileName))
        return false;

    auto quotedArguments = arguments;
    quotedArguments.replaceInStrings(QStringLiteral(R"(")"), QStringLiteral(R"(\")"));
    quotedArguments.replaceInStrings(QRegularExpression(QStringLiteral("^(.*)$")),
        QStringLiteral(R"("\1")"));

    std::cerr << "Starting Updater: " << updaterExeFileName.toStdString() << "\n";
    if(!QProcess::startDetached(updaterExeFileName, quotedArguments))
    {
        std::cerr << "  ...failed\n";
        return false;
    }

    return true;
}
