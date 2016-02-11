#include "preferences.h"

#include <QSettings>

QVariant u::pref(const QString& key, const QVariant& defaultValue)
{
    QSettings settings;

    if(!settings.contains(key))
        u::setPref(key, defaultValue);

    return settings.value(key);
}

void u::setPref(const QString& key, const QVariant& value)
{
    QSettings().setValue(key, value);
}
