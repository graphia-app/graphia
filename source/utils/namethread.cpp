#include "namethread.h"

#if defined(__linux__)
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

void nameCurrentThread(const QString& name)
{
    if(syscall(SYS_gettid) != getpid()) // Avoid renaming main thread
        prctl(PR_SET_NAME, (char*)name.toUtf8().constData());
}
#else
void nameCurrentThread(const QString&) {}
#endif
