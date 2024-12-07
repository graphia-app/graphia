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

#include "preferences.h"

#include "app/limitconstants.h"

#include "shared/utils/utils.h"

#include <QSettings>
#include <QCoreApplication>
#include <QtGlobal>
#include <QRegularExpression>

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

PreferencesWatcher::Watchers PreferencesWatcher::_watchers;

PreferencesWatcher::PreferencesWatcher()
{
    const std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);
    PreferencesWatcher::_watchers._set.insert(this);
}

PreferencesWatcher::~PreferencesWatcher()
{
    const std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);
    PreferencesWatcher::_watchers._set.erase(this);
}

void PreferencesWatcher::setPref(const QString& key, const QVariant& value)
{
    const std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);

    for(auto* watcher : PreferencesWatcher::_watchers._set)
        emit watcher->preferenceChanged(key, value);
}

Preferences::Preferences(QObject* parent) :
    QObject(parent)
{
    connect(&_watcher, &PreferencesWatcher::preferenceChanged, this, &Preferences::onPreferenceChanged);
}

QString Preferences::section() const
{
    return _section;
}

void Preferences::setSection(const QString& section)
{
    if(_section != section)
    {
        _section = section;

        if(_initialised)
            load();

        emit sectionChanged();
    }
}

template<typename Fn> static void forEachProperty(const QObject* o, Fn fn)
{
    const QMetaObject *mo = o->metaObject();
    const int offset = mo->propertyOffset();
    const int count = mo->propertyCount();
    for(int i = offset; i < count; i++)
        fn(mo->property(i));
}

void Preferences::componentComplete()
{
    if(!_initialised)
    {
        load();

               // Connect all the property notify signals so we know when they change
        forEachProperty(this,
        [this](const QMetaProperty& property)
        {
            if(property.hasNotifySignal())
            {
                auto index = metaObject()->indexOfSlot("onPropertyChanged()");
                auto slot = metaObject()->method(index);
                QObject::connect(this, property.notifySignal(), this, slot);
            }
        });

        _initialised = true;
    }
}

QString Preferences::preferenceNameByPropertyName(const QString& propertyName)
{
    return u"%1/%2"_s.arg(_section, propertyName);
}

QMetaProperty Preferences::propertyByName(const QString& propertyName) const
{
    if(!propertyName.isEmpty())
    {
        auto index = metaObject()->indexOfProperty(propertyName.toLatin1());
        if(index >= 0)
            return metaObject()->property(index);
    }

    return {};
}

static QString propertyNameFrom(const QString& preferenceName)
{
    static const QRegularExpression sepRegex(uR"(\/)"_s);
    if(preferenceName.contains(sepRegex))
    {
        auto stringList = preferenceName.split(sepRegex);
        if(stringList.size() == 2)
            return stringList.at(1);
    }

    return preferenceName;
}

QMetaProperty Preferences::propertyFrom(const QString& preferenceName)
{
    return propertyByName(propertyNameFrom(preferenceName));
}

void Preferences::setProperty(QMetaProperty property, const QVariant& value)
{
    if(!property.isValid())
        return;

    const QVariant previousValue = property.read(this);

    if((previousValue != value && QMetaType::canConvert(value.metaType(), previousValue.metaType())) ||
       !previousValue.isValid())
    {
        property.write(this, value);
    }
}

void Preferences::load()
{
    forEachProperty(this,
    [this](const QMetaProperty& property)
    {
        setProperty(property, u::pref(preferenceNameByPropertyName(property.name())));
    });
}

void Preferences::onPreferenceChanged(const QString& key, const QVariant& value)
{
    setProperty(propertyFrom(key), value);
}

void Preferences::onPropertyChanged()
{
    auto metaMethodName = metaObject()->method(senderSignalIndex()).name();
    QMetaProperty changedProperty;

    forEachProperty(this,
    [&metaMethodName, &changedProperty](const QMetaProperty& property)
    {
        if(property.notifySignal().name() == metaMethodName)
            changedProperty = property;
    });

    if(changedProperty.isValid())
    {
        u::setPref(preferenceNameByPropertyName(changedProperty.name()),
            changedProperty.read(this));
    }
}
