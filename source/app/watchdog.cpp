#include "watchdog.h"

#include "shared/utils/fatalerror.h"
#include "shared/utils/thread.h"

#include <QDebug>

#ifdef _MSC_VER
#include <intrin.h>
#endif

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
    if(_timer == nullptr)
    {
        _timer = new QTimer(this);
        _timer->setSingleShot(true);
        connect(_timer, &QTimer::timeout, []
        {
            qWarning() << "Watchdog timed out! Deadlock? Infinite loop?";

#ifndef _DEBUG
            // Deliberately crash in release mode...
            FATAL_ERROR(WatchdogTimedOut);
#elif Q_PROCESSOR_X86
            // ...or invoke the debugger otherwise
#ifdef _MSC_VER
            __debugbreak();
#else
            __asm__("int3");
#endif
#endif
        });

        u::setCurrentThreadName(QStringLiteral("WatchdogThread"));
    }

    const int timeout = 15000;
    _timer->start(timeout);
}
