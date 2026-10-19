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

#ifndef TESTENVIRONMENT_H
#define TESTENVIRONMENT_H

#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <QTemporaryDir>

namespace TestEnvironment
{
// Call from initTestCase in any test that exercises code which reads or writes
// preferences, so that the user's real settings are left untouched
inline void initialise()
{
    using namespace Qt::Literals::StringLiterals;

    static const QTemporaryDir settingsDir;

    QCoreApplication::setOrganizationName(u"GraphiaTests"_s);
    QCoreApplication::setApplicationName(u"GraphiaTests"_s);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());
}
} // namespace TestEnvironment

#endif // TESTENVIRONMENT_H
