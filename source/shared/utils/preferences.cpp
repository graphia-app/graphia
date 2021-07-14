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

#include "preferences.h"

#include "shared/utils/preferenceswatcher.h"

#include <QSettings>
#include <QCoreApplication>

namespace
{
QSettings settings()
{
    return {QSettings::Format::IniFormat, QSettings::Scope::UserScope,
            QCoreApplication::organizationName(), QCoreApplication::applicationName()};
}
} // namespace

void u::definePref(const QString& key, const QVariant& defaultValue)
{
    if(!u::prefExists(key))
        u::setPref(key, defaultValue);
}

bool u::removePref(const QString& key)
{
    if(u::prefExists(key))
    {
        settings().remove(key);
        return true;
    }

    return false;
}

QVariant u::pref(const QString& key)
{
    u::definePref(key);

    return settings().value(key);
}

void u::setPref(const QString& key, const QVariant& value)
{
    auto s = settings();
    bool changed = (value != s.value(key));

    s.setValue(key, value);

    if(changed)
        PreferencesWatcher::setPref(key, value);
}

bool u::prefExists(const QString& key)
{
    return settings().contains(key);
}

void u::updateOldPrefs()
{
}
