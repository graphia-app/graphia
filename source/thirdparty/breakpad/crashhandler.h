#ifndef EXECEPTIONHANDLER_H
#define EXECEPTIONHANDLER_H

#include <QtGlobal>

#include <memory>
#include <functional>

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

    void onCrash(std::function<void(const QString& directory)>&& userHandler) { _userHandler = userHandler; }
    const auto& userHandler() const { return _userHandler; }

private:
    std::unique_ptr<google_breakpad::ExceptionHandler> _handler;
    std::function<void(const QString& directory)> _userHandler;
    platform_char _crashReporterExecutableName[1024] = {0};
};

#endif // EXECEPTIONHANDLER_H
