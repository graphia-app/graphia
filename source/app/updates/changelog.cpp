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

#include "changelog.h"

#include "shared/updates/updates.h"
#include "shared/utils/container.h"
#include "shared/utils/static_block.h"

#include "application.h"

#include <QByteArray>
#include <QFile>
#include <QRegularExpression>
#include <QQmlEngine>

#include <json_helper.h>

ChangeLog::ChangeLog(QObject *parent) :
    QObject(parent)
{
    refresh();
}

void ChangeLog::refresh()
{
    json changeLog = changeLogJson();

    if(changeLog.is_discarded() || !changeLog.is_object())
        return;

    if(!u::contains(changeLog, "text") || !u::contains(changeLog, "images"))
        return;

    if(!_imagesDirectory.isValid())
        return;

    for(const auto& image : changeLog["images"])
    {
        if(!u::contains(image, "filename") || !u::contains(image, "content"))
            continue;

        auto fileName = QString::fromStdString(image["filename"]);
        auto base64EncodedContent = QString::fromStdString(image["content"]);
        auto content = QByteArray::fromBase64(base64EncodedContent.toUtf8());

        QFile imageFile(QStringLiteral("%1/%2").arg(_imagesDirectory.path(), fileName));
        if(!imageFile.open(QIODevice::ReadWrite))
            continue;

        imageFile.write(content);
    }

    _text = QString::fromStdString(changeLog["text"]);

    // Adjust the text so that the images in the text correspond to the files on disk
    auto replacement = QStringLiteral("![\\1](file:///%1/\\2)").arg(_imagesDirectory.path());
    _text = _text.replace(QRegularExpression(QStringLiteral(R"((?:!\[(.*?)\]\(file:(.*?)\)))")), replacement);
    emit textChanged();

    _available = true;
    emit availableChanged();
}

static_block
{
    qmlRegisterType<ChangeLog>(APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "ChangeLog");
}
