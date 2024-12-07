/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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
