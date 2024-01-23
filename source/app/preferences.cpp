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

#include "preferences.h"

#include "app/limitconstants.h"

#include "shared/utils/utils.h"
#include "app/preferenceswatcher.h"

#include <QSettings>
#include <QCoreApplication>
#include <QtGlobal>

using namespace Qt::Literals::StringLiterals;

namespace
{
QSettings settings()
{
#ifdef Q_OS_WASM
    const QSettings::Format format = QSettings::Format::NativeFormat;
#else
    const QSettings::Format format = QSettings::Format::IniFormat;
#endif

    return {format, QSettings::Scope::UserScope,
            QCoreApplication::organizationName(), QCoreApplication::applicationName()};
}
} // namespace

void u::definePref(const QString& key, const QVariant& defaultValue)
{
    if(!u::prefExists(key))
        u::setPref(key, defaultValue);
}

QVariant u::pref(const QString& key)
{
    u::definePref(key, {});

    return settings().value(key);
}

bool u::prefExists(const QString& key)
{
    return settings().contains(key);
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

void u::setPref(const QString& key, const QVariant& value)
{
    auto s = settings();
    const bool changed = (value != s.value(key));

    s.setValue(key, value);

    if(changed)
        PreferencesWatcher::setPref(key, value);
}

void u::updateOldPrefs()
{
    if(u::prefExists(u"visuals/defaultNodeSize"_s))
    {
        auto absNodeSize = u::pref(u"visuals/defaultNodeSize"_s).toFloat();
        auto normalNodeSize = u::normalise(LimitConstants::minimumNodeSize(),
            LimitConstants::maximumNodeSize(), absNodeSize);

        u::setPref(u"visuals/defaultNormalNodeSize"_s, normalNodeSize);
        u::removePref(u"visuals/defaultNodeSize"_s);
    }

    if(u::prefExists(u"visuals/defaultEdgeSize"_s))
    {
        auto absEdgeSize = u::pref(u"visuals/defaultEdgeSize"_s).toFloat();
        auto normalEdgeSize = u::normalise(LimitConstants::minimumEdgeSize(),
            LimitConstants::maximumEdgeSize(), absEdgeSize);

        u::setPref(u"visuals/defaultNormalEdgeSize"_s, normalEdgeSize);
        u::removePref(u"visuals/defaultEdgeSize"_s);
    }
}

QString u::settingsFileName()
{
    return settings().fileName();
}
