/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "tracking.h"

#include "app/preferences.h"
#include "shared/utils/container.h"

#include <json_helper.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHostInfo>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QSysInfo>
#include <QEventLoop>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QDebug>

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

static QString postToTrackingServer(const QString& text)
{
    QNetworkAccessManager manager;
    QEventLoop loop;

    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(10s);

    QNetworkRequest request;
    request.setUrl(u::pref(QStringLiteral("servers/tracking")).toString());

    auto* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentDispositionHeader,
                   QVariant(R"(form-data; name="request")"));
    part.setBody(text.toUtf8());
    multiPart->append(part);

    auto* reply = manager.post(request, multiPart);
    multiPart->setParent(reply);

    // Block until the server replies
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if(!timer.isActive())
    {
        // Timeout
        reply->abort();
        return {};
    }

    if(reply->error() != QNetworkReply::NetworkError::NoError)
    {
        qDebug() << "postToTrackingServer QNetworkReply::NetworkError" << reply->errorString();
        return {};
    }

    auto replyJsonString = reply->readAll();
    auto replyJson = json::parse(replyJsonString.begin(), replyJsonString.end(), nullptr, false);

    if(!u::contains(replyJson, "error") || !u::contains(replyJson, "content"))
    {
        qDebug() << "postToTrackingServer malformed reply";
        return {};
    }

    if(replyJson["error"] != "none")
        qDebug() << "postToTrackingServer error" << QString::fromStdString(replyJson["error"]);

    return replyJson["content"];
}

static QString anonymousIdentity()
{
    auto anonUser = QStringLiteral("anon");
    QString hostName = QHostInfo::localDomainName();

    const json ipRequestJson = {{"action", "getip"}};
    auto ipAddress = postToTrackingServer(QString::fromStdString(ipRequestJson.dump()));

    auto hostInfo = QHostInfo::fromName(ipAddress);
    if(hostInfo.error() == QHostInfo::NoError)
    {
        hostName = hostInfo.hostName();

        // Remove any sections of the hostname that contain non-alpha characters, as
        // they're probably DNS encoded IP addresses or similar such things, and we
        // only really care about the sub-top level domain
        hostName = hostName.split('.').filter(QRegularExpression(QStringLiteral("^[a-zA-Z]+$"))).join('.');
    }

    if(hostInfo.error() != QHostInfo::NoError || hostName.isEmpty())
        hostName = ipAddress;

    auto homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if(homePath.length() > 0)
    {
        QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
        hash.addData(homePath.first().toUtf8());
        anonUser.append('_').append(hash.result().toHex().mid(0, 7));
    }

    auto id = QStringLiteral("%1@%2").arg(anonUser, hostName);
    return id;
}

void Tracking::submit()
{
    auto doSubmission = []
    {
        auto permission = u::pref(QStringLiteral("tracking/permission")).toString();
        auto identity = u::pref(QStringLiteral("tracking/emailAddress")).toString();

        if(permission == QStringLiteral("rejected"))
            return;

        if(permission == QStringLiteral("anonymous") || identity.isEmpty())
        {
            if(!u::prefExists(QStringLiteral("tracking/anonymousId")) ||
                u::pref(QStringLiteral("tracking/anonymousId")).toString().isEmpty())
            {
                identity = anonymousIdentity();
                u::setPref(QStringLiteral("tracking/anonymousId"), identity);
            }
            else
                identity = u::pref(QStringLiteral("tracking/anonymousId")).toString();
        }

        auto os = QStringLiteral("%1 %2 %3 %4").arg(
            QSysInfo::kernelType(), QSysInfo::kernelVersion(),
            QSysInfo::productType(), QSysInfo::productVersion());

        const json trackingJson =
        {
            {"action",          "submit"},
            {"payload",
            {
                {"email",       identity},
                {"locale",      QLocale::system().name()},
                {"product",     PRODUCT_NAME},
                {"version",     VERSION},
                {"os",          os}
            }
            }
        };

        postToTrackingServer(QString::fromStdString(trackingJson.dump()));
    };

    std::thread fireAndForget(doSubmission);
    fireAndForget.detach();
}
