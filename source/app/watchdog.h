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

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QProcess>

#include <chrono>

class WatchdogWorker : public QObject
{
    Q_OBJECT

private:
    QTimer* _timer = nullptr;
    using clock_type = std::chrono::steady_clock;
    clock_type::time_point _expectedExpiry;

    const std::chrono::seconds _defaultTimeoutDuration{30};
    std::chrono::seconds _timeoutDuration{_defaultTimeoutDuration};

    void startTimer();
    void showWarning();

public slots:
    void onReset();

private slots:
    void onWarningProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

signals:
    void reset();
};

class Watchdog : public QObject
{
    Q_OBJECT

private:
    QThread _thread;

public:
    Watchdog();
    ~Watchdog() override;

signals:
    void reset();
};

#endif // WATCHDOG_H
