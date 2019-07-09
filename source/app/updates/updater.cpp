#include "updater.h"

#include "shared/updates/updates.h"

#include "application.h"

#include "shared/utils/preferences.h"
#include "shared/utils/container.h"
#include "shared/utils/string.h"
#include "shared/utils/checksum.h"

#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QTimer>
#include <QSysInfo>
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

    connect(&_networkManager, &QNetworkAccessManager::finished,
        this, &Updater::onReplyReceived);

    QObject::connect(&_timeoutTimer, &QTimer::timeout, this, &Updater::onTimeout);
    QObject::connect(&_backgroundCheckTimer, &QTimer::timeout,
        this, &Updater::startBackgroundUpdateCheck);

    connect(S(Preferences), &Preferences::preferenceChanged,
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
    if(!u::pref("misc/autoBackgroundUpdateCheck").toBool())
    {
        disableAutoBackgroundCheck();
        return;
    }

    if(_backgroundCheckTimer.isActive())
        return;

    const int BACKGROUND_CHECK_INTERVAL = 60 * 60000;
    _backgroundCheckTimer.start(BACKGROUND_CHECK_INTERVAL);

    // Do a check right away, too
    startBackgroundUpdateCheck();
}

void Updater::disableAutoBackgroundCheck()
{
    if(!_backgroundCheckTimer.isActive())
        return;

    _backgroundCheckTimer.stop();
    cancelUpdateDownload();
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

    auto emailAddress = u::pref("auth/emailAddress").toString();

    json requestJson =
    {
        {"currentVersion", VERSION},
        {"os", {
            {"kernel", QSysInfo::kernelType()},
            {"kernelVersion", QSysInfo::kernelVersion()},
            {"product", QSysInfo::productType()},
            {"productVersion", QSysInfo::productVersion()}}
        },
        {"emailAddress", emailAddress}
    };

    std::string requestString = requestJson.dump();

    // Work around for QTBUG-31652 (PRNG takes time to initialise) to
    // effectively call QNetworkAccessManager::post asynchronously
    QTimer::singleShot(0, [this, requestString]
    {
        QNetworkRequest request;
        request.setUrl(QUrl(QStringLiteral("https://updates.kajeka.com/")));

        auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(R"(form-data; name="request")"));
        part.setBody(requestString.data());
        multiPart->append(part);

        _state = Updater::State::Update;
        Q_ASSERT(_reply == nullptr);
        _reply = _networkManager.post(request, multiPart);
        multiPart->setParent(_reply);
    });
}

void Updater::downloadUpdate(QNetworkReply* reply)
{
    QFuture<json> future = QtConcurrent::run([this, reply]
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
            QUrl url = update["url"];
            urlIsValid = url.isValid();
        }

        if(!update.is_object() || !urlIsValid)
        {
            // Update isn't valid, for whatever reason
            update["error"] = QStringLiteral("invalid");
            return update;
        }

        QString status;
        auto oldUpdate = latestUpdateJson(&status);

        if(oldUpdate.is_object() && u::contains(oldUpdate, "version") &&
            oldUpdate["version"] == update["version"] &&
            status != QStringLiteral("failed"))
        {
            // We already have this update, and it was either successfully
            // installed, the user has skipped it, or they haven't dealt
            // with it yet
            update["error"] = status.isEmpty() ?
                QStringLiteral("existing") : status;
        }
        else
            _updateString = updateString;

        return update;
    });

    auto* watcher = new QFutureWatcher<json>;
    connect(watcher, &QFutureWatcher<json>::finished, [this, watcher, reply]
    {
        const auto& update = watcher->result();

        if(u::contains(update, "error"))
        {
            QString error = update["error"];
            emit noNewUpdateAvailable(error ==
                QStringLiteral("existing"));
            _state = State::Idle;
            return;
        }

        QUrl url = update["url"];
        QNetworkRequest request(url);

        if(u::contains(update, "httpUserName") && u::contains(update, "httpPassword"))
        {
            QString concatenatedCredentials = QStringLiteral("%1:%2")
                .arg(QString::fromStdString(update["httpUserName"]),
                QString::fromStdString(update["httpPassword"]));

            QByteArray data = concatenatedCredentials.toLocal8Bit().toBase64();
            QString headerData = "Basic " + data;
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
            _progress = (bytesReceived * 100) / bytesTotal;
            emit progressChanged();
        });

        reply->deleteLater();
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void Updater::saveUpdate(QNetworkReply* reply)
{
    QFuture<bool> future = QtConcurrent::run([this, reply]
    {
        _progress = -1;
        emit progressChanged();

        // Recursively delete existing update files
        auto location = updatesLocation();
        if(!location.isEmpty())
        {
            QDir dir(location);
            if(dir.exists())
            {
                for(const auto& subFileName : dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
                {
                    QFileInfo info(QStringLiteral("%1/%2").arg(location, subFileName));

                    if(info.isDir())
                        QDir(info.absoluteFilePath()).removeRecursively();
                    else
                        QFile(info.absoluteFilePath()).remove();
                }
            }
        }

        QTemporaryFile tempFile;
        if(!tempFile.open())
            return false;

        tempFile.write(reply->readAll());
        tempFile.close();

        if(!u::sha256ChecksumMatchesFile(tempFile.fileName(), _checksum))
            return false;

        QDir().mkpath(QFileInfo(_fileName).absolutePath());
        tempFile.rename(_fileName);
        tempFile.setAutoRemove(false);

        return true;
    });

    auto* watcher = new QFutureWatcher<bool>;
    connect(watcher, &QFutureWatcher<bool>::finished, [this, watcher, reply]
    {
        auto success = watcher->result();

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
        watcher->deleteLater();
    });
    watcher->setFuture(future);
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
    if(!latestUpdateJson(&status).is_object())
        return false;

    // Update has already been skipped or installed
    if(status == "skipped" || status == "installed")
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
    quotedArguments.replaceInStrings(QStringLiteral("\""), QStringLiteral("\\\""));
    quotedArguments.replaceInStrings(QRegularExpression(QStringLiteral("^(.*)$")),
        QStringLiteral("\"\\1\""));

    std::cerr << "Starting Updater: " << updaterExeFileName.toStdString() << "\n";
    if(!QProcess::startDetached(updaterExeFileName, quotedArguments))
    {
        std::cerr << "  ...failed\n";
        return false;
    }

    return true;
}
