/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "app/preferences.h"

#include <QRegularExpression>
#include <QQmlEngine>

using namespace Qt::Literals::StringLiterals;

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
