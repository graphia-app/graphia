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

#ifndef SHARED_PREFERENCES_H
#define SHARED_PREFERENCES_H

#include <QVariant>
#include <QSettings>
#include <QCoreApplication>
#include <QtGlobal>

class QString;

namespace u
{
    inline QVariant getPref(const QString& key)
    {
#ifdef Q_OS_WASM
        const QSettings::Format format = QSettings::Format::NativeFormat;
#else
        const QSettings::Format format = QSettings::Format::IniFormat;
#endif

        const QSettings settings(format, QSettings::Scope::UserScope,
            QCoreApplication::organizationName(), QCoreApplication::applicationName());

        return settings.value(key);
    }

} // namespace u

#endif // SHARED_PREFERENCES_H

