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

#include "shared/utils/qmlutils.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QString>
#include <QSettings>
#include <QIcon>

#include <json_helper.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Graphia"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("graphia.app"));
    QCoreApplication::setApplicationName(QStringLiteral("Graphia Update Editor"));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
    QSettings::setDefaultFormat(QSettings::Format::IniFormat);

    qmlRegisterSingletonType<QmlUtils>(APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION,
        "QmlUtils", &QmlUtils::qmlInstance);

    QIcon mainIcon;
    mainIcon.addFile(QStringLiteral(":/Icon512x512.png"));
    mainIcon.addFile(QStringLiteral(":/Icon256x256.png"));
    mainIcon.addFile(QStringLiteral(":/Icon128x128.png"));
    mainIcon.addFile(QStringLiteral(":/Icon64x64.png"));
    mainIcon.addFile(QStringLiteral(":/Icon32x32.png"));
    mainIcon.addFile(QStringLiteral(":/Icon16x16.png"));
    QApplication::setWindowIcon(mainIcon);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return QCoreApplication::exec();
}
