/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "showinfolder.h"

#include <QtGlobal>
#include <QProcess>
#include <QDir>
#include <QFileInfo>

#include <QDebug>

using namespace Qt::Literals::StringLiterals;

void u::showInFolder(const QString& path)
{
#if defined(Q_OS_WIN32) && QT_CONFIG(process)
    QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(path)});
#elif defined(Q_OS_MACOS)
    QProcess::execute("/usr/bin/osascript", {"-e",
        u"tell application \"Finder\" to reveal POSIX file \"%1\""_s.arg(path)});
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
#elif defined(Q_OS_UNIX) && QT_CONFIG(process)
    const QFileInfo fileInfo(path);

    // AFAICT, the best we can do on *nix is to open the folder itself
    QProcess::execute(u"xdg-open"_s, {fileInfo.absolutePath()});
#else
    Q_UNUSED(path);
    qDebug() << "u::showInFolder not implemented";
#endif
}
