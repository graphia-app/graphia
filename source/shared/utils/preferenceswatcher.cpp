#include "preferenceswatcher.h"

PreferencesWatcher::Watchers PreferencesWatcher::_watchers; // NOLINT cppcoreguidelines-avoid-non-const-global-variables

PreferencesWatcher::PreferencesWatcher()
{
    std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);
    PreferencesWatcher::_watchers._set.insert(this);
}

PreferencesWatcher::~PreferencesWatcher()
{
    std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);
    PreferencesWatcher::_watchers._set.erase(this);
}

void PreferencesWatcher::setPref(const QString& key, const QVariant& value)
{
    std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);

    for(const auto* watcher : PreferencesWatcher::_watchers._set)
        emit watcher->preferenceChanged(key, value);
}
