#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QVariant>
#include <QString>

namespace u
{
    QVariant pref(const QString& key, const QVariant& defaultValue = QVariant());
    void setPref(const QString& key, const QVariant& value);
}

#endif // PREFERENCES_H

