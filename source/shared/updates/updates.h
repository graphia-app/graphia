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

#ifndef UPDATES_H
#define UPDATES_H

#include <json_helper.h>

#include <QString>

QString updatesLocation();
json updateStringToJson(const QString& updateString, QString* status = nullptr);

QString fullyQualifiedInstallerFileName(const json& update);
json latestUpdateJson(QString* status = nullptr);

bool storeUpdateJson(const QString& updateString);
bool storeUpdateStatus(const QString& status);
bool clearUpdateStatus();

bool storeChangeLogJson(const QString& changeLogString);
json changeLogJson();

#endif // UPDATES_H
