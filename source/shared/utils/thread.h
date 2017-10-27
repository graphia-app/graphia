#ifndef THREAD_H
#define THREAD_H

#include <QString>

namespace u
{
    int currentThreadId();
    void setCurrentThreadName(const QString& name);
    QString currentThreadName();
    QString parentProcessName();
}

#endif // THREAD_H
