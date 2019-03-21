#include "installer.h"

#include "shared/updates/updates.h"
#include "shared/utils/preferences.h"
#include "shared/utils/container.h"
#include "shared/utils/checksum.h"
#include "shared/utils/scope_exit.h"

#include <QString>
#include <QFile>

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include <iostream>

void Installer::signalComplete()
{
    _complete = true;
    emit completeChanged();
}

Installer::Installer(json details, QString version,
    QString existingInstallation) :
    _details(std::move(details)), _version(std::move(version)),
    _existingInstallation(std::move(existingInstallation))
{
    connect(&_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    [this](int exitCode, QProcess::ExitStatus exitStatus)
    {
        auto allStdOut = _process.readAllStandardOutput();
        auto allStdErr = _process.readAllStandardError();
        QString error = allStdOut + allStdErr;

        if(exitCode == 0 && exitStatus == QProcess::NormalExit)
        {
            // Remove the installer file
            QFile installerFile(_installerFileName);

            if(installerFile.exists())
                installerFile.remove();

            _success = true;
            emit successChanged();
        }
        else
        {
            error += tr("\nProcess failed (%1, %2)")
                .arg(exitCode).arg(exitStatus);
        }

        if(error != _error)
        {
            _error = error;
            emit errorChanged();
        }

        signalComplete();
    });

    connect(&_process, &QProcess::errorOccurred,
    [this](QProcess::ProcessError processError)
    {
        QString error = tr("Process error %1:\n%2")
            .arg(processError).arg(_process.errorString());

        if(error != _error)
        {
            _error = error;
            emit errorChanged();
        }

        signalComplete();
    });
}

void Installer::start()
{
    if(!_busy)
    {
        _busy = true;
        emit busyChanged();
    }

    QFuture<bool> future = QtConcurrent::run([this]
    {
        if(!u::contains(_details, "installerFileName") ||
            !u::contains(_details, "installerChecksum") ||
            !u::contains(_details, "command"))
        {
            _error = tr("Corrupt update details.");
            emit errorChanged();
            return false;
        }

        _installerFileName = fullyQualifiedInstallerFileName(_details);
        if(!QFile::exists(_installerFileName))
        {
            _error = tr("Installer asset does not exist.");
            emit errorChanged();
            return false;
        }

        const auto& checksum = QString::fromStdString(_details["installerChecksum"]);
        if(!u::sha256ChecksumMatchesFile(_installerFileName, checksum))
        {
            _error = tr("Installer asset checksum does not match.");
            emit errorChanged();
            return false;
        }

        return true;
    });

    auto* watcher = new QFutureWatcher<bool>;
    connect(watcher, &QFutureWatcher<bool>::finished, [this, watcher]
    {
        if(watcher->result())
        {
            QStringList arguments = _details["command"];
            arguments.replaceInStrings(QStringLiteral("EXISTING_INSTALL"), _existingInstallation);
            arguments.replaceInStrings(QStringLiteral("INSTALLER_FILE"), _installerFileName);

            _process.start(arguments.at(0), arguments.mid(1));
        }
        else
            signalComplete();

        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void Installer::retry()
{
    if(_complete)
    {
        _complete = false;
        emit completeChanged();
    }

    start();
}

void Installer::setStatus(const QString& status)
{
    storeUpdateStatus(status);
}
