/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "installer.h"

#include "shared/updates/updates.h"
#include "shared/utils/container.h"
#include "shared/utils/preferences.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QRegularExpression>
#include <QQuickStyle>

#include <QDebug>

#include <iostream>

#include <qtsingleapplication/qtsingleapplication.h>

static QString existingInstallation(const QString& exe)
{
#if defined(Q_OS_WIN)
    return QFileInfo(exe).absoluteDir().absolutePath();
#elif defined(Q_OS_LINUX)
    if(qEnvironmentVariableIsSet("APPIMAGE"))
        return qgetenv("APPIMAGE");

    return QFileInfo(exe).absoluteDir().absolutePath();
#elif defined(Q_OS_MAC)
    auto exeDir = QFileInfo(exe).absoluteDir();
    exeDir.cdUp(); // Contents
    exeDir.cdUp(); // .app
    return exeDir.absolutePath();
#else
    static_assert(!"Unknown OS");
#endif
}

QStringList showUpdater(int argc, char *argv[])
{
    const SharedTools::QtSingleApplication app(QStringLiteral(PRODUCT_NAME), argc, argv);

    auto arguments = QApplication::arguments();

    if(arguments.size() <= 1)
        return {};

    auto unquotedArguments = arguments.mid(1);
    unquotedArguments.replaceInStrings(QRegularExpression(QStringLiteral("^\"(.*)\"$")),
        QStringLiteral(R"(\1)"));
    auto exe = unquotedArguments.at(0);

    QApplication::setOrganizationName(QStringLiteral("Graphia"));
    QApplication::setOrganizationDomain(QStringLiteral("graphia.app"));
    QApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QApplication::setApplicationVersion(QStringLiteral(VERSION));

    Q_INIT_RESOURCE(shared);
    Q_INIT_RESOURCE(update_keys);

    QString status;
    auto update = latestUpdateJson(&status);
    if(update.is_object() && (status.isEmpty() || status == QStringLiteral("failed")))
    {
        QIcon mainIcon;
        mainIcon.addFile(QStringLiteral(":/Icon512x512.png"));
        mainIcon.addFile(QStringLiteral(":/Icon256x256.png"));
        mainIcon.addFile(QStringLiteral(":/Icon128x128.png"));
        mainIcon.addFile(QStringLiteral(":/Icon64x64.png"));
        mainIcon.addFile(QStringLiteral(":/Icon32x32.png"));
        mainIcon.addFile(QStringLiteral(":/Icon16x16.png"));
        QApplication::setWindowIcon(mainIcon);

        QQuickStyle::setStyle(u::getPref(QStringLiteral("system/uiTheme")).toString());

        QQmlApplicationEngine engine;

        const QString version = u::contains(update, "version") ?
            QString::fromStdString(update["version"]) :
            QObject::tr("Unknown");

        const QString changeLog = u::contains(update, "changeLog") ?
            QString::fromStdString(update["changeLog"]) :
            QObject::tr("No release notes available.");

        const QTemporaryDir imagesDir;
        engine.rootContext()->setContextProperty(
            QStringLiteral("imagesLocation"), imagesDir.path());

        if(u::contains(update, "images") && imagesDir.isValid())
        {
            for(const auto& image : update["images"])
            {
                if(!u::contains(image, "filename") || !u::contains(image, "content"))
                    continue;

                auto fileName = QString::fromStdString(image["filename"]);
                auto base64EncodedContent = QString::fromStdString(image["content"]);
                auto content = QByteArray::fromBase64(base64EncodedContent.toUtf8());

                QFile imageFile(QStringLiteral("%1/%2").arg(imagesDir.path(), fileName));
                if(!imageFile.open(QIODevice::ReadWrite))
                    continue;

                imageFile.write(content);
            }
        }

        engine.rootContext()->setContextProperty(
            QStringLiteral("updatesLocation"), updatesLocation());

        engine.rootContext()->setContextProperty(
            QStringLiteral("version"), version);

        engine.rootContext()->setContextProperty(
            QStringLiteral("changeLog"), changeLog);

        Installer installer(update, version, existingInstallation(exe));
        engine.rootContext()->setContextProperty(
            QStringLiteral("installer"), &installer);

        engine.addImportPath(QStringLiteral("qrc:///qml/"));
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        Q_ASSERT(!engine.rootObjects().empty());

        QApplication::exec();
    }

    return unquotedArguments;
}

// NOLINTNEXTLINE bugprone-exception-escape
int main(int argc, char *argv[])
{
    QStringList arguments = showUpdater(argc, argv);
    if(arguments.isEmpty())
    {
        std::cerr << "Updater started without arguments.\n";
        return 1;
    }

    // Whatever the user chose, don't show them the updater again
    arguments.append(QStringLiteral("--dontUpdate"));

    std::cerr << "Restarting " << arguments.join(' ').toStdString() << "\n";
    QProcess::startDetached(arguments.at(0), arguments.mid(1));

    return 0;
}
