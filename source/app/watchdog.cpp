#include "watchdog.h"

#include "shared/utils/fatalerror.h"
#include "shared/utils/thread.h"

// Disable warnings that Valgrind
#ifndef _MSC_VER
#include "thirdparty/gccdiagaware.h"
#ifdef GCC_DIAGNOSTIC_AWARE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include "thirdparty/valgrind/valgrind.h"
#else
#define RUNNING_ON_VALGRIND 0
#endif

#include <QDebug>

Watchdog::Watchdog()
{
    auto *worker = new WatchdogWorker;
    worker->moveToThread(&_thread);
    connect(&_thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &Watchdog::reset, worker, &WatchdogWorker::reset);
    _thread.start();
}

Watchdog::~Watchdog()
{
    _thread.quit();
    _thread.wait();
}

void WatchdogWorker::reset()
{
    using namespace std::chrono;
    const auto timeout = 30s;

    if(_timer == nullptr)
    {
        _timer = new QTimer(this);
        _timer->setSingleShot(true);
        connect(_timer, &QTimer::timeout, this, [=]
        {
            auto howLate = duration_cast<milliseconds>(clock_type::now() - _expectedExpiry);

            // QTimers are guaranteed to be accurate within 5%, so this should be generous enough
            auto lateThreshold = timeout * 0.1;

            if(howLate > lateThreshold)
            {
                // If we're significantly late, then the watchdog thread itself has been paused
                // for some time, implying that the *entire* application has been paused, so our
                // detection of the freeze is probably incorrect and we should wait another interval
                reset();
                return;
            }

            // Don't bother doing anything when running under Valgrind
            if(RUNNING_ON_VALGRIND)
                return;

            qWarning() << "Watchdog timed out! Deadlock? "
                "Infinite loop? Resuming from a breakpoint?";

#ifndef _DEBUG
            // Deliberately crash in release mode...
            FATAL_ERROR(WatchdogTimedOut);
#endif
        });

        u::setCurrentThreadName(QStringLiteral("WatchdogThread"));
    }

    _expectedExpiry = clock_type::now() + timeout;
    _timer->start(timeout);
}
