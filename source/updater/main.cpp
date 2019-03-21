#include "installer.h"

#include "shared/updates/updates.h"
#include "shared/utils/preferences.h"
#include "shared/utils/container.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QRegularExpression>

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
    SharedTools::QtSingleApplication app(QStringLiteral(PRODUCT_NAME), argc, argv);

    auto arguments = QApplication::arguments();

    if(arguments.size() <= 1)
        return {};

    auto unquotedArguments = arguments.mid(1);
    unquotedArguments.replaceInStrings(QRegularExpression(QStringLiteral("^\"(.*)\"$")),
        QStringLiteral("\\1"));
    auto exe = unquotedArguments.at(0);

    QApplication::setOrganizationName(QStringLiteral("Kajeka"));
    QApplication::setOrganizationDomain(QStringLiteral("kajeka.com"));
    QApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QApplication::setApplicationVersion(QStringLiteral(VERSION));

    Q_INIT_RESOURCE(update_keys);

    qmlRegisterType<QmlPreferences>(APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "Preferences");
    Preferences preferences;

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

        QQmlApplicationEngine engine;

        QString version = u::contains(update, "version") ?
            QString::fromStdString(update["version"]) :
            QObject::tr("Unknown");

        QString changeLog = u::contains(update, "changeLog") ?
            QString::fromStdString(update["changeLog"]) :
            QObject::tr("No release notes available.");

        QTemporaryDir imagesDir;
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

            engine.rootContext()->setContextProperty(
                QStringLiteral("imagesLocation"), imagesDir.path());
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

        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

        QApplication::exec();
    }

    return unquotedArguments;
}

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
