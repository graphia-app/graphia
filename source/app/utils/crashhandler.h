#ifndef EXECEPTIONHANDLER_H
#define EXECEPTIONHANDLER_H

#include <QtGlobal>

#include <memory>

namespace google_breakpad { class ExceptionHandler; }

using platform_char =
#ifdef Q_OS_WIN
    wchar_t
#else
    char
#endif
;

class CrashHandler
{
public:
    CrashHandler();
    virtual ~CrashHandler();

    const platform_char* crashReporterExecutableName() const { return _crashReporterExecutableName; }

private:
    std::unique_ptr<google_breakpad::ExceptionHandler> _handler;
    platform_char _crashReporterExecutableName[1024] = {0};
};

#endif // EXECEPTIONHANDLER_H
