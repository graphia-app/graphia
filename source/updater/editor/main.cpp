/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "shared/utils/qmlutils.h"
#include "shared/utils/preferences.h"
#include "shared/utils/static_block.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QString>
#include <QSettings>
#include <QIcon>
#include <QQuickStyle>

#include <json_helper.h>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    const QApplication app(argc, argv);

    Q_INIT_RESOURCE(shared);

    execute_static_blocks();

    QCoreApplication::setOrganizationName(u"Graphia"_s);
    QCoreApplication::setOrganizationDomain(u"graphia.app"_s);
    QCoreApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
    QSettings::setDefaultFormat(QSettings::Format::IniFormat);

    qmlRegisterSingletonType<QmlUtils>(APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION,
        "QmlUtils", &QmlUtils::qmlInstance);

    QIcon mainIcon;
    mainIcon.addFile(u":/Icon512x512.png"_s);
    mainIcon.addFile(u":/Icon256x256.png"_s);
    mainIcon.addFile(u":/Icon128x128.png"_s);
    mainIcon.addFile(u":/Icon64x64.png"_s);
    mainIcon.addFile(u":/Icon32x32.png"_s);
    mainIcon.addFile(u":/Icon16x16.png"_s);
    QApplication::setWindowIcon(mainIcon);

    QQuickStyle::setStyle(u::getPref(u"system/uiTheme"_s).toString());

    QQmlApplicationEngine engine;
    engine.addImportPath(u"qrc:///qml/"_s);
    engine.load(QUrl(u"qrc:/main.qml"_s));
    Q_ASSERT(!engine.rootObjects().empty());

    return QCoreApplication::exec();
}
