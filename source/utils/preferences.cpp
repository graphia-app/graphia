#include "preferences.h"

#include <QSettings>
#include <QMetaProperty>
#include <QTimerEvent>
#include <QRegularExpression>

QVariant Preferences::get(const QString& key, const QVariant& defaultValue)
{
    if(!exists(key))
        set(key, defaultValue);

    return _settings.value(key);
}

void Preferences::set(const QString& key, const QVariant& value)
{
    bool changed = (value != _settings.value(key));
    _settings.setValue(key, value);

    if(changed)
        emit preferenceChanged(key, value);
}

bool Preferences::exists(const QString& key)
{
    return _settings.contains(key);
}

QVariant u::pref(const QString& key, const QVariant& defaultValue)
{
    return Preferences::instance()->get(key, defaultValue);
}

void u::setPref(const QString& key, const QVariant& value)
{
    Preferences::instance()->set(key, value);
}

bool u::prefExists(const QString& key)
{
    return Preferences::instance()->exists(key);
}


QmlPreferences::QmlPreferences(QObject* parent) :
    QObject(parent)
{
    connect(Preferences::instance(), &Preferences::preferenceChanged,
            this, &QmlPreferences::onPreferenceChanged);
}

QmlPreferences::~QmlPreferences()
{
    flush();
}

QString QmlPreferences::section() const
{
    return _section;
}

void QmlPreferences::setSection(const QString& section)
{
    if(_section != section)
    {
        flush();
        _section = section;

        if(_initialised)
            load();
    }
}

void QmlPreferences::timerEvent(QTimerEvent* event)
{
    if(event->timerId() == _timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
        save();
    }

    QObject::timerEvent(event);
}

template<typename Fn> static void forEachProperty(const QObject* o, Fn fn)
{
    const QMetaObject *mo = o->metaObject();
    const int offset = mo->propertyOffset();
    const int count = mo->propertyCount();
    for(int i = offset; i < count; i++)
        fn(mo->property(i));
}

void QmlPreferences::componentComplete()
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

QString QmlPreferences::preferenceNameFrom(const QString& propertyName)
{
    return QString("%1/%2").arg(_section).arg(propertyName);
}

QMetaProperty QmlPreferences::propertyFrom(const QString& preferenceName)
{
    QString propertyName;

    auto sepRegex = QRegularExpression("\\/");
    if(preferenceName.contains(sepRegex))
    {
        auto stringList = preferenceName.split(sepRegex);
        if(stringList.size() == 2)
            propertyName = stringList.at(1);
    }
    else
        propertyName = preferenceName;

    if(!propertyName.isEmpty())
    {
        auto index = metaObject()->indexOfProperty(propertyName.toLatin1());
        if(index >= 0)
            return metaObject()->property(index);
    }

    return {};
}

void QmlPreferences::setProperty(QMetaProperty property, const QVariant& value)
{
    const QVariant previousValue = property.read(this);
    const QVariant currentValue = value;

    if((previousValue != currentValue && currentValue.canConvert(previousValue.type())) ||
       !previousValue.isValid())
    {
        property.write(this, currentValue);
    }
}

void QmlPreferences::load()
{
    forEachProperty(this,
    [this](const QMetaProperty& property)
    {
        auto preferenceName = preferenceNameFrom(property.name());
        if(u::prefExists(preferenceName))
            setProperty(property, u::pref(preferenceName));
    });
}

void QmlPreferences::save()
{
    for(auto& pendingPreferenceChange : _pendingPreferenceChanges)
    {
        u::setPref(preferenceNameFrom(pendingPreferenceChange.first),
                   pendingPreferenceChange.second);
    }

    _pendingPreferenceChanges.clear();
}

void QmlPreferences::flush()
{
    if(_initialised && !_pendingPreferenceChanges.empty())
        save();
}

void QmlPreferences::onPreferenceChanged(const QString& key, const QVariant& value)
{
    auto property = propertyFrom(key);

    if(property.isValid())
        setProperty(property, value);
}

void QmlPreferences::onPropertyChanged()
{
    auto metaMethodName = metaObject()->method(senderSignalIndex()).name();
    QMetaProperty changedProperty;

    forEachProperty(this,
    [this, &metaMethodName, &changedProperty](const QMetaProperty& property)
    {
        if(property.notifySignal().name() == metaMethodName)
            changedProperty = property;
    });

    _pendingPreferenceChanges[changedProperty.name()] = changedProperty.read(this);

    if(_timerId != 0)
        killTimer(_timerId);

    // Delay actually setting the preference until a property hasn't changed for some time
    // This prevents many rapid property changes causing redundant preference writes
    _timerId = startTimer(500);
}
