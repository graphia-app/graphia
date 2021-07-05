#ifndef PREFERENCESWATCHER_H
#define PREFERENCESWATCHER_H

#include "shared/utils/preferences.h"

#include <QObject>
#include <QString>
#include <QVariant>

#include <mutex>
#include <set>

class PreferencesWatcher : public QObject
{
    Q_OBJECT

    friend void u::setPref(const QString& key, const QVariant& value);

private:
    static struct Watchers
    {
        std::recursive_mutex _mutex;
        std::set<PreferencesWatcher*> _set;
    } _watchers;

    static void setPref(const QString& key, const QVariant& value);

public:
    PreferencesWatcher();
    ~PreferencesWatcher() override;

signals:
    void preferenceChanged(const QString& key, const QVariant& value);
};

#endif // PREFERENCESWATCHER_H
