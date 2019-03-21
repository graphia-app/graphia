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

#endif // UPDATES_H
