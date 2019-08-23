#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <QtGlobal>
#include <QString>

#include "shared/utils/singleton.h"

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

class CrashHandler : public Singleton<CrashHandler>
{
public:
    CrashHandler(const QString& crashReporterExecutableName);
    virtual ~CrashHandler();

    // These are only here so the callback can access them
    const platform_char* crashReporterExecutableName() const { return _crashReporterExecutableName; }
    QString reason() const { return _reason; }
    const auto& userHandler() const { return _userHandler; }

    void onCrash(std::function<void(const QString& directory)>&& userHandler) { _userHandler = userHandler; }

    void submitMinidump(const QString& reason);

private:
    std::unique_ptr<google_breakpad::ExceptionHandler> _handler;
    std::function<void(const QString& directory)> _userHandler;
    platform_char _crashReporterExecutableName[1024] = {0};
    QString _reason;
};

#endif // CRASHHANDLER_H
