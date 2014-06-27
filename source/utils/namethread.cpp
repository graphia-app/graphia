#include "namethread.h"

#if defined(__linux__) && defined(QT_DEBUG)
#include <sys/prctl.h>

void nameCurrentThread(const QString& name)
{
    prctl(PR_SET_NAME, (char*)name.toUtf8().constData());
}
#else
void nameCurrentThread(const QString&) {}
#endif
