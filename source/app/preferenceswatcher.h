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

#ifndef PREFERENCESWATCHER_H
#define PREFERENCESWATCHER_H

#include "preferences.h"

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
