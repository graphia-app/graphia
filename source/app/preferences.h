/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef PREFERENCES_H
#define PREFERENCES_H

class QVariant;
class QString;

namespace u
{
    void definePref(const QString& key, const QVariant& defaultValue);
    QVariant pref(const QString& key);
    bool prefExists(const QString& key);

    bool removePref(const QString& key);
    void setPref(const QString& key, const QVariant& value);

    void updateOldPrefs();
} // namespace u

#endif // PREFERENCES_H
