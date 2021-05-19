/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "apppathname.h"

#include <QtGlobal>
#include <QFileInfo>
#include <QDir>

static QString appPath;

QString u::appPathName() { return appPath; }

void u::setAppPathName(const QString& exe)
{
    // Default
    appPath = QFileInfo(exe).absoluteFilePath();

#if defined(Q_OS_LINUX)
    if(qEnvironmentVariableIsSet("APPIMAGE"))
        appPath = qgetenv("APPIMAGE");
#elif defined(Q_OS_MAC)
    auto dir = QFileInfo(exe).absoluteDir();
    dir.cdUp(); // Contents
    dir.cdUp(); // .app
    appPath = dir.absolutePath();
#endif
}
