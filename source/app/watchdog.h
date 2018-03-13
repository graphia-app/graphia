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
    ~Watchdog();

signals:
    void reset();
};

#endif // WATCHDOG_H
