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

#include "build_defines.h"

#include "updates.h"

#include "shared/utils/container.h"
#include "shared/utils/checksum.h"
#include "shared/utils/string.h"
#include "shared/utils/crypto.h"
#include "shared/utils/utils.h"

#include <json_helper.h>

#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QSysInfo>
#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <regex>

using namespace Qt::Literals::StringLiterals;

QString updatesLocation()
{
    auto appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if(appDataLocation.isEmpty())
        return {};

    return appDataLocation + u"/Updates"_s;
}

static QString updateFilePath()
{
    return u"%1/update.json"_s.arg(updatesLocation());
}

json updateStringToJson(const QString& updateString, QString* status)
{
    auto updateStringStdString = updateString.toStdString();
    auto updateObject = json::parse(updateStringStdString.begin(), updateStringStdString.end(),
        nullptr, false);

    if(updateObject.is_discarded())
        return {};

    if(!u::contains(updateObject, "updates") || !u::contains(updateObject, "signature"))
        return {};

    if(!updateObject["updates"].is_string() || !updateObject["signature"].is_string())
        return {};

    auto updatesString = updateObject["updates"].get<std::string>();
    auto hexSignature = updateObject["signature"].get<std::string>();
    auto signature = u::hexToString(hexSignature);

    if(!u::rsaVerifySignature(updatesString, signature, ":/update_keys/public_update_key.der"))
        return {};

    auto decodedUpdatesString = u::isHex(updatesString) ? u::hexToString(updatesString) : updatesString;
    auto updates = json::parse(decodedUpdatesString.begin(), decodedUpdatesString.end(), nullptr, false);

    if(updates.is_discarded())
        return {};

    // Remove updates that don't apply to the running version
    updates.erase(std::remove_if(updates.begin(), updates.end(),
    [](const auto& update)
    {
        // Retain if it matches the running version, so that the changelog can be grabbed
        if(u::contains(update, "version") && update["version"] == VERSION)
            return false;

        if(!u::contains(update, "targetVersionRegex"))
            return true;

        const std::string targetVersionRegex = update["targetVersionRegex"];

        return !std::regex_match(VERSION, std::regex{targetVersionRegex});
    }), updates.end());

    // Remove updates that don't have a payload for the running OS
    updates.erase(std::remove_if(updates.begin(), updates.end(),
    [](const auto& update)
    {
        if(!u::contains(update, "payloads"))
            return true;

        const auto& payloads = update["payloads"];

        return payloads.find(QSysInfo::kernelType().toStdString()) == payloads.end();
    }), updates.end());

    // Remove updates that are older than the running version
    updates.erase(std::remove_if(updates.begin(), updates.end(),
    [](const auto& update)
    {
        // Don't discard updates on the basis of version number if they are forced
        if(u::contains(update, "force"))
            return false;

        return u::numericCompare(update["version"], VERSION) < 0;
    }), updates.end());

    if(updates.empty())
        return {};

    std::sort(updates.begin(), updates.end(),
    [](const auto& a, const auto& b)
    {
        // Forced updates get sorted to the front
        if(u::exclusiveOr(u::contains(a, "force"), u::contains(b, "force")))
            return u::contains(a, "force");

        return u::numericCompare(QString::fromStdString(a["version"]),
            QString::fromStdString(b["version"])) > 0;
    });

    json latestUpdate = updates.at(0);
    json payload = *latestUpdate["payloads"].find(QSysInfo::kernelType().toStdString());

    if(status != nullptr && u::contains(updateObject, "status"))
        *status = QString::fromStdString(updateObject["status"]);

    json update =
    {
        {"version",             latestUpdate["version"]},
        {"url",                 payload["url"]},
        {"installerFileName",   payload["installerFileName"]},
        {"installerChecksum",   payload["installerChecksum"]},
        {"command",             payload["command"]},
        {"changeLog",           latestUpdate["changeLog"]},
        {"images",              json::array()},
    };

    for(const auto& image : latestUpdate["images"])
        update["images"].push_back(image);

    if(u::contains(payload, "httpUserName") || u::contains(payload, "httpPassword"))
    {
        update["httpUserName"] = payload["httpUserName"];
        update["httpPassword"] = payload["httpPassword"];
    }

    return update;
}

QString fullyQualifiedInstallerFileName(const json& update)
{
    auto filename = QString::fromStdString(update["installerFileName"]);
    return u"%1/%2"_s
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
    auto fileName = updateFilePath();
    QDir().mkpath(QFileInfo(fileName).absolutePath());
    QFile updateFile(fileName);

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

static QString changeLogFilePath()
{
    return u"%1/changeLog.json"_s.arg(updatesLocation());
}

bool storeChangeLogJson(const QString& changeLogString)
{
    auto fileName = changeLogFilePath();
    QDir().mkpath(QFileInfo(fileName).absolutePath());
    QFile changeLogFile(fileName);

    if(!changeLogFile.open(QFile::WriteOnly | QFile::Text))
        return false;

    auto byteArray = changeLogString.toUtf8();
    return changeLogFile.write(byteArray) == byteArray.size();
}

json changeLogJson()
{
    QFile changeLogFile(changeLogFilePath());

    if(!changeLogFile.exists())
        return {};

    if(!changeLogFile.open(QFile::ReadOnly | QFile::Text))
        return {};

    auto changeLogFileContents = changeLogFile.readAll();

    auto changeLogObject = json::parse(changeLogFileContents.begin(), changeLogFileContents.end(),
        nullptr, false);

    if(changeLogObject.is_discarded())
        return {};

    if(!u::contains(changeLogObject, "text") || !u::contains(changeLogObject, "images"))
        return {};

    return changeLogObject;
}
