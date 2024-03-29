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

#include "preferenceswatcher.h"

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
