#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include <chrono>

class WatchdogWorker : public QObject
{
    Q_OBJECT

private:
    QTimer* _timer = nullptr;
    using clock_type = std::chrono::steady_clock;
    clock_type::time_point _expectedExpiry;

public slots:
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
