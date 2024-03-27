/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "watchdog.h"

#include "shared/utils/fatalerror.h"
#include "shared/utils/thread.h"

#include "application.h"

#ifndef _MSC_VER
#include <valgrind/valgrind.h>
#else
#define RUNNING_ON_VALGRIND 0
#endif

#include <QDebug>
#include <QCoreApplication>
#include <QMessageBox>
#include <QProcess>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

Watchdog::Watchdog()
{
    auto *worker = new WatchdogWorker;
    worker->moveToThread(&_thread);
    connect(&_thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &Watchdog::reset, worker, &WatchdogWorker::onReset);
    _thread.start();
}

// NOLINTNEXTLINE modernize-use-equals-default
Watchdog::~Watchdog()
{
    _thread.quit();
    _thread.wait();
}

void WatchdogWorker::showWarning()
{
#if QT_CONFIG(process)
    const QString messageBoxExe = Application::resolvedExe(u"MessageBox"_s);

    if(messageBoxExe.isEmpty())
    {
        qWarning() << "Couldn't resolve MessageBox executable";
        return;
    }

    QStringList arguments;
    arguments <<
        u"-title"_s << u"Error"_s <<
        u"-text"_s << QString(
            tr("%1 is not responding. System resources could be under pressure, "
               "so you may optionally wait in case a recovery occurs. "
               "Alternatively, please report a bug if you believe the "
               "freeze is as a result of a software problem."))
            .arg(QCoreApplication::applicationName()) <<
        u"-icon"_s << u"Critical"_s <<
        u"-button"_s << u"Wait:Reset"_s <<
        u"-button"_s << u"Close and Report Bug:Destructive"_s <<
        u"-defaultButton"_s << u"Wait"_s;

    auto* warningProcess = new QProcess(this);

    // Remove the warning if we recover in the mean time
    connect(this, &WatchdogWorker::reset, warningProcess, &QProcess::kill);

    connect(warningProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &WatchdogWorker::onWarningProcessFinished);
    connect(warningProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        warningProcess, &WatchdogWorker::deleteLater);

    std::cerr << "Starting " << messageBoxExe.toStdString() << "\n";
    warningProcess->start(messageBoxExe, arguments);
#endif
}

void WatchdogWorker::onReset()
{
    _timeoutDuration = _defaultTimeoutDuration;
    startTimer();
}

void WatchdogWorker::startTimer()
{
    using namespace std::chrono;

    if(_timer == nullptr)
    {
        _timer = new QTimer(this);
        _timer->setSingleShot(true);
        connect(_timer, &QTimer::timeout, this, [this]
        {
            auto howLate = duration_cast<milliseconds>(clock_type::now() - _expectedExpiry);

            // QTimers are guaranteed to be accurate within 5%, so this should be generous enough
            auto lateThreshold = _timeoutDuration * 0.1;

            if(howLate > lateThreshold)
            {
                // If we're significantly late, then the watchdog thread itself has been paused
                // for some time, implying that the *entire* application has been paused, so our
                // detection of the freeze is probably incorrect and we should wait another interval
                startTimer();
                return;
            }

            // Don't bother doing anything when running under Valgrind
            if(RUNNING_ON_VALGRIND) // NOLINT
                return;

            qWarning() << "Watchdog timed out! Deadlock? "
                "Infinite loop? Resuming from a breakpoint?";

#ifndef _DEBUG
            showWarning();
#endif
        });

        u::setCurrentThreadName(u"WatchdogThread"_s);
    }

    emit reset();

    _expectedExpiry = clock_type::now() + _timeoutDuration;
    _timer->start(_timeoutDuration);
}

#if QT_CONFIG(process)
void WatchdogWorker::onWarningProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Check for a sane exit code and status in case our warning has been killed (by us)
    if(exitCode >= 0 && exitCode < QMessageBox::NRoles && exitStatus == QProcess::NormalExit)
    {
        if(exitCode == QMessageBox::DestructiveRole)
        {
            // Deliberately crash if the user chooses not to wait
            FATAL_ERROR(WatchdogTimedOut);
        }
        else
        {
            _timeoutDuration *= 2;
            startTimer();
        }
    }
    else
        std::cerr << "Watchdog warning cancelled; process recovered\n";
}
#endif
