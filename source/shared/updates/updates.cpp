/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include "updates.h"

#include "shared/utils/container.h"
#include "shared/utils/checksum.h"
#include "shared/utils/preferences.h"
#include "shared/utils/string.h"
#include "shared/utils/crypto.h"

#include <json_helper.h>

#include <QString>
#include <QStringList>
#include <QCollator>
#include <QStandardPaths>

#include <algorithm>

QString updatesLocation()
{
    auto appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if(appDataLocation.isEmpty())
        return {};

    return appDataLocation + QStringLiteral("/Updates");
}

static QString updateFilePath()
{
    return QStringLiteral("%1/update.json").arg(updatesLocation());
}

json updateStringToJson(const QString& updateString, QString* status)
{
    auto updateStringStdString = updateString.toStdString();
    auto payload = json::parse(updateStringStdString.begin(), updateStringStdString.end(),
        nullptr, false);

    if(payload.is_discarded())
        return {};

    if(!u::contains(payload, "update") || !u::contains(payload, "signature"))
        return {};

    auto hexString = payload["update"];
    auto hexSignature = payload["signature"].get<std::string>();
    auto signature = u::hexToString(hexSignature);

    if(!u::rsaVerifySignature(hexString, signature, ":/update_keys/public_update_key.der"))
        return {};

    auto decodedUpdateString = u::hexToString(hexString.get<std::string>());
    auto update = json::parse(decodedUpdateString.begin(), decodedUpdateString.end(), nullptr, false);

    if(update.is_discarded())
        return {};

    if(!u::contains(update, "url") || !u::contains(update, "installerFileName"))
        return {};

    if(status != nullptr && u::contains(payload, "status"))
        *status = QString::fromStdString(payload["status"]);

    return update;
}

QString fullyQualifiedInstallerFileName(const json& update)
{
    auto filename = QString::fromStdString(update["installerFileName"]);
    return QStringLiteral("%1/%2")
        .arg(updatesLocation(), filename);
}

static QString latestUpdateString()
{
    QFile updateFile(updateFilePath());

    if(!updateFile.exists())
        return {};

    if(!updateFile.open(QFile::ReadOnly | QFile::Text))
        return {};

    return updateFile.readAll();
}

json latestUpdateJson(QString* status)
{
    auto updateString = latestUpdateString();
    return updateStringToJson(updateString, status);
}

bool storeUpdateJson(const QString& updateString)
{
    QFile updateFile(updateFilePath());

    if(!updateFile.open(QFile::WriteOnly | QFile::Text))
        return false;

    auto byteArray = updateString.toUtf8();
    return updateFile.write(byteArray) == byteArray.size();
}

bool storeUpdateStatus(const QString& status)
{
    auto updateString = latestUpdateString();
    auto updateStringStdString = updateString.toStdString();
    auto payload = json::parse(updateStringStdString.begin(), updateStringStdString.end(),
        nullptr, false);

    if(payload.is_discarded())
        return false;

    payload["status"] = status;

    return storeUpdateJson(QString::fromStdString(payload.dump()));
}

bool clearUpdateStatus()
{
    return storeUpdateStatus({});
}
