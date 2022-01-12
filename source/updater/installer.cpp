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

#include "installer.h"

#include "shared/updates/updates.h"
#include "shared/utils/container.h"
#include "shared/utils/checksum.h"
#include "shared/utils/scope_exit.h"
#include "shared/utils/doasyncthen.h"

#include <QString>
#include <QFile>

#include <iostream>

void Installer::signalComplete()
{
    _complete = true;
    emit completeChanged();
}

Installer::Installer(const json& details, const QString& version,
    const QString& existingInstallation) :
    _details(details), _version(version),
    _existingInstallation(existingInstallation)
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

    u::doAsync([this]
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
    })
    .then([this](bool success)
    {
        if(success)
        {
            QString command = _details["command"];
            command.replace(QStringLiteral("EXISTING_INSTALL"), _existingInstallation)
                .replace(QStringLiteral("INSTALLER_FILE"), _installerFileName);

            QStringList arguments(
#if defined(Q_OS_UNIX)
                {"/bin/bash", "-c"}
#elif defined(Q_OS_WIN)
                {"cmd.exe", "/C"}
#endif
            );

            arguments.append(command);
            _process.start(arguments.at(0), arguments.mid(1));
        }
        else
            signalComplete();
    });
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

// NOLINTNEXTLINE readability-convert-member-functions-to-static
void Installer::setStatus(const QString& status)
{
    storeUpdateStatus(status);
}
