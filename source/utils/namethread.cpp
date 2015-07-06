#include "namethread.h"
#include "utils.h"

#if defined(__linux__)
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

void nameCurrentThread(const QString& name)
{
    if(syscall(SYS_gettid) != getpid()) // Avoid renaming main thread
        prctl(PR_SET_NAME, (char*)name.toUtf8().constData());
}

const QString currentThreadName()
{
    char threadName[16] = {0};
    prctl(PR_GET_NAME, (unsigned long)threadName);

    return threadName;
}

#else
void nameCurrentThread(const QString&) {}
const QString currentThreadName()
{
    return QString::number(Utils::currentThreadId());
}

#endif
