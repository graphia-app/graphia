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

#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QtGlobal>
#include <QIcon>
#include <QMessageBox>
#include <QStyleHints>
#include <QGuiApplication>
#include <QWidget>
#include <QWindow>
#include <QScreen>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTimer>
#include <QCommandLineParser>
#include <QProcess>
#include <QSettings>
#include <QNetworkProxy>
#include <QQmlFileSelector>
#include <QtWebEngineQuick>

#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>

#include "application.h"
#include "preferences.h"
#include "preferenceswatcher.h"
#include "headless.h"

#include "shared/utils/threadpool.h"
#include "shared/utils/scopetimer.h"
#include "shared/utils/macosfileopeneventfilter.h"
#include "shared/utils/debugger.h"
#include "shared/utils/apppathname.h"
#include "shared/utils/static_block.h"
#include "shared/utils/thread.h"
#include "shared/utils/console.h"
#include "shared/utils/consolecapture.h"
#include "shared/utils/signalhandling.h"
#include "shared/ui/visualisations/defaultgradients.h"
#include "shared/ui/visualisations/defaultpalettes.h"

#include "rendering/openglfunctions.h"
#include "rendering/graphrenderer.h"

#include "updates/updater.h"

#include <qtsingleapplication/qtsingleapplication.h>
#include <breakpad/crashhandler.h>

#include "watchdog.h"

using namespace std::chrono_literals;

static QString resolvedExeName(const QString& baseExeName)
{
#ifdef Q_OS_LINUX
    if(qEnvironmentVariableIsSet("APPIMAGE"))
        return qgetenv("APPIMAGE");
#endif

    return baseExeName;
}

static void configureXDG()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    auto dotDesktopFileContent = QStringLiteral(
        "[Desktop Entry]\n"
        "Name=%1\n"
        "Comment=Visualise and analyse graphs\n"
        "Exec=%2 %U\n"
        "Icon=%1.svg\n"
        "Terminal=false\n"
        "Type=Application\n"
        "Encoding=UTF-8\n"
        "Categories=Application;Graphics;Science;\n"
        "StartupWMClass=%1\n"
        "MimeType=x-scheme-handler/%3\n"
        "X-KDE-Protocols=%3;\n")
        .arg(Application::name(), u::appPathName(),
        Application::nativeExtension());

    auto applicationsDirname = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    auto genericDirname = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if(applicationsDirname.isEmpty() || genericDirname.isEmpty())
    {
        std::cerr << "Could not determine XDG directories.\n";
        return;
    }

    auto dotDesktopFilename = QDir(applicationsDirname)
        .filePath(QStringLiteral("%1.desktop").arg(Application::name()));
    auto dotDesktopFile = QFile(dotDesktopFilename);

    auto iconsDirname = QDir(genericDirname).filePath(QStringLiteral("icons"));
    auto iconsDir = QDir(iconsDirname);
    auto iconFilename = iconsDir.filePath(QStringLiteral("%1.svg").arg(Application::name()));
    auto iconPermissions = QFileDevice::ReadOwner|QFileDevice::WriteOwner|
        QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther;

    auto xdgMimeArguments = QStringList
    {
        QStringLiteral("default"),
        QStringLiteral("%1.desktop").arg(Application::name()),
        QStringLiteral("x-scheme-handler/%1").arg(Application::nativeExtension())
    };

    auto success = ((iconsDir.exists() || iconsDir.mkpath(iconsDir.absolutePath())) &&
        dotDesktopFile.open(QIODevice::WriteOnly) && dotDesktopFile.write(dotDesktopFileContent.toUtf8()) >= 0 &&
        (QFileInfo::exists(iconFilename) ||
            (QFile::copy(QStringLiteral(":/icon/Icon.svg"), iconFilename) &&
            QFile::setPermissions(iconFilename, iconPermissions))) &&
        QProcess::startDetached(QStringLiteral("xdg-mime"), xdgMimeArguments)) || false;

    if(!success)
        std::cerr << "Failed to configure for XDG.\n";
#endif
}

static void configureProxy()
{
    QNetworkProxy::ProxyType type = QNetworkProxy::DefaultProxy;
    auto typePref = u::pref(QStringLiteral("proxy/type")).toString();

    if(typePref == QStringLiteral("http"))
        type = QNetworkProxy::HttpProxy;
    else if(typePref == QStringLiteral("socks5"))
        type = QNetworkProxy::Socks5Proxy;

    QNetworkProxy proxy;
    proxy.setType(type);

    if(type != QNetworkProxy::DefaultProxy)
    {
        proxy.setHostName(u::pref(QStringLiteral("proxy/host")).toString());
        proxy.setPort(static_cast<quint16>(u::pref(QStringLiteral("proxy/port")).toUInt()));
        proxy.setUser(u::pref(QStringLiteral("proxy/username")).toString());
        proxy.setPassword(u::pref(QStringLiteral("proxy/password")).toString());
    }

    QNetworkProxy::setApplicationProxy(proxy);
}

static QString qmlError;

int start(int argc, char *argv[], ConsoleOutputFiles& consoleOutputFiles)
{
    if(u::currentThreadName().isEmpty())
        u::setCurrentThreadName(QStringLiteral(PRODUCT_NAME));

    SharedTools::QtSingleApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    SharedTools::QtSingleApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QQuickWindow::setDefaultAlphaBuffer(true);

    QtWebEngineQuick::initialize();

    // Without this setting, rendering artefacts appear on systems with
    // non-integral scaling factors (e.g. using high DPI monitors on Windows)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::Round);

    SharedTools::QtSingleApplication app(QStringLiteral(PRODUCT_NAME), argc, argv);

    Application::setAppDir(QCoreApplication::applicationDirPath());

    if(!u::isDebuggerPresent() && app.isRunning())
    {
        if(app.sendMessage(QCoreApplication::arguments().join(QStringLiteral("\n"))))
            return 0;
    }

    // Wait until the application is active before setting the focus window
    QObject::connect(&app, &SharedTools::QtSingleApplication::applicationStateChanged,
    [&app]
    {
        if(app.activationWindow() == nullptr)
            app.setActivationWindow(QApplication::focusWindow());
    });

    QCommandLineParser commandLineParser;

    commandLineParser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    commandLineParser.addHelpOption();
    commandLineParser.addOptions(
    {
        {{"u", "dontUpdate"}, QObject::tr("Don't update now, but remind later.")},
        {{"m", "startMaximised"}, QObject::tr("Put the application window in maximised state.")},
        {{"w", "skipWelcome"}, QObject::tr("Don't show the welcome screen on first start.")},
        {{"p", "parameters"}, QObject::tr("Run in headless mode, using parameters from <file>."), "file"},
    });

    commandLineParser.process(QCoreApplication::arguments());

    Q_INIT_RESOURCE(update_keys);

    auto dontUpdate = commandLineParser.isSet(QStringLiteral("dontUpdate")) ||
        commandLineParser.isSet(QStringLiteral("parameters"));

    if(!dontUpdate && Updater::updateAvailable())
    {
        QStringList restartArguments = QCoreApplication::arguments();
        restartArguments[0] = resolvedExeName(restartArguments.at(0));

        if(Updater::showUpdatePrompt(restartArguments))
        {
            // The updater will restart the application once finished, so quit now
            return 0;
        }
    }

    if(commandLineParser.isSet(QStringLiteral("startMaximised")))
        u::setPref(QStringLiteral("window/maximised"), true);

    if(commandLineParser.isSet(QStringLiteral("skipWelcome")) && !u::prefExists(QStringLiteral("tracking/permission")))
        u::setPref(QStringLiteral("tracking/permission"), QStringLiteral("anonymous"));

    QGuiApplication::styleHints()->setMousePressAndHoldInterval(500);

    QIcon mainIcon;
    mainIcon.addFile(QStringLiteral(":/icon/Icon512x512.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon256x256.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon128x128.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon64x64.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon32x32.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon16x16.png"));
    QApplication::setWindowIcon(mainIcon);

    QIcon::setThemeName(QStringLiteral("Tango"));

    // Since Qt is responsible for managing OpenGL, we need
    // to give it a hint that we want a debug context
    if(qEnvironmentVariableIntValue("OPENGL_DEBUG") > 0)
        qputenv("QSG_OPENGL_DEBUG", "1");

    if(!OpenGLFunctions::hasOpenGLSupport())
    {
        QString message;

        QString vendor = OpenGLFunctions::vendor();
        if(!vendor.isEmpty())
        {
            vendor.replace(QStringLiteral(" "), QStringLiteral("+"));
            const QString driversUrl = QStringLiteral(R"(https://www.google.com/search?q=%1+video+driver+download&btnI)").arg(vendor);
            message = QObject::tr("The installed version of OpenGL is insufficient to run %1. "
                R"(Please install the latest <a href="%2">video drivers</a> available from )"
                "your vendor and try again.").arg(Application::name(), driversUrl);
        }
        else
            message = QObject::tr("OpenGL is not available. %1 will not run.").arg(Application::name());

        QMessageBox messageBox(QMessageBox::Critical, QObject::tr("OpenGL support"), message, QMessageBox::Close);

        messageBox.setTextFormat(Qt::RichText);
        messageBox.exec();

        return 1;
    }

    configureXDG();

    qRegisterMetaType<size_t>("size_t");

    execute_static_blocks();

    const ThreadPoolSingleton threadPool;
    const ScopeTimerManager scopeTimerManager;

    if(commandLineParser.isSet(QStringLiteral("parameters")))
    {
        installSignalHandlers();

        auto parametersFilename = commandLineParser.value(QStringLiteral("parameters"));
        Headless headless(commandLineParser.positionalArguments(), parametersFilename);

        QTimer::singleShot(0, &headless, &Headless::run);
        QObject::connect(&headless, &Headless::done,
            QCoreApplication::instance(), &QCoreApplication::quit);
        QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
            &headless, &Headless::cancel);

        return QCoreApplication::exec();
    }

    u::definePref(QStringLiteral("visuals/defaultNodeColor"),               "#0000FF");
    u::definePref(QStringLiteral("visuals/defaultEdgeColor"),               "#FFFFFF");
    u::definePref(QStringLiteral("visuals/multiElementColor"),              "#FF0000");
    u::definePref(QStringLiteral("visuals/backgroundColor"),                "#C0C0C0");
    u::definePref(QStringLiteral("visuals/highlightColor"),                 "#FFFFFF");

    u::definePref(QStringLiteral("visuals/defaultNormalNodeSize"),          0.333);
    u::definePref(QStringLiteral("visuals/defaultNormalEdgeSize"),          0.25);

    u::definePref(QStringLiteral("visuals/showNodeText"),                   QVariant::fromValue(static_cast<int>(TextState::Selected)));
    u::definePref(QStringLiteral("visuals/showEdgeText"),                   QVariant::fromValue(static_cast<int>(TextState::Selected)));
    u::definePref(QStringLiteral("visuals/textFont"),                       SharedTools::QtSingleApplication::font().family());
    u::definePref(QStringLiteral("visuals/textSize"),                       24.0f);
    u::definePref(QStringLiteral("visuals/edgeVisualType"),                 QVariant::fromValue(static_cast<int>(EdgeVisualType::Cylinder)));
    u::definePref(QStringLiteral("visuals/textAlignment"),                  QVariant::fromValue(static_cast<int>(TextAlignment::Right)));
    u::definePref(QStringLiteral("visuals/showMultiElementIndicators"),     true);
    u::definePref(QStringLiteral("visuals/savedGradients"),                 Defaults::GRADIENT_PRESETS);
    u::definePref(QStringLiteral("visuals/defaultGradient"),                Defaults::GRADIENT);
    u::definePref(QStringLiteral("visuals/savedPalettes"),                  Defaults::PALETTE_PRESETS);
    u::definePref(QStringLiteral("visuals/defaultPalette"),                 Defaults::PALETTE);

    u::definePref(QStringLiteral("visuals/projection"),                     QVariant::fromValue(static_cast<int>(Projection::Perspective)));

    u::definePref(QStringLiteral("visuals/minimumComponentRadius"),         2.0);
    u::definePref(QStringLiteral("visuals/transitionTime"),                 1.0);

    u::definePref(QStringLiteral("visuals/disableMultisampling"),           false);

    u::definePref(QStringLiteral("misc/maxUndoLevels"),                     25);

    u::definePref(QStringLiteral("misc/showGraphMetrics"),                  false);
    u::definePref(QStringLiteral("misc/showLayoutSettings"),                false);

    u::definePref(QStringLiteral("misc/focusFoundNodes"),                   true);
    u::definePref(QStringLiteral("misc/focusFoundComponents"),              true);
    u::definePref(QStringLiteral("misc/stayInComponentMode"),               false);

    u::definePref(QStringLiteral("misc/disableHubbles"),                    false);

    u::definePref(QStringLiteral("misc/hasSeenTutorial"),                   false);

    u::definePref(QStringLiteral("misc/autoBackgroundUpdateCheck"),         true);

    u::definePref(QStringLiteral("screenshot/width"),                       1920);
    u::definePref(QStringLiteral("screenshot/height"),                      1080);
    u::definePref(QStringLiteral("screenshot/path"),
        QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString());

    u::definePref(QStringLiteral("servers/redirects"),                      "https://redirects.graphia.app");
    u::definePref(QStringLiteral("servers/updates"),                        "https://updates.graphia.app");
    u::definePref(QStringLiteral("servers/crashreports"),                   "https://crashreports.graphia.app");
    u::definePref(QStringLiteral("servers/tracking"),                       "https://tracking.graphia.app");

    u::definePref(QStringLiteral("proxy/type"),                             "disabled");

    u::definePref(QStringLiteral("system/uiTheme"),                         "Default");

    u::updateOldPrefs();

    const PreferencesWatcher preferencesWatcher;

    QObject::connect(&preferencesWatcher, &PreferencesWatcher::preferenceChanged,
        [](const QString& key, const QVariant&) { if(key.startsWith(QStringLiteral("proxy"))) configureProxy(); });

    configureProxy();

    QQuickStyle::setStyle(u::pref(QStringLiteral("system/uiTheme")).toString());

    QStringList selectors; // NOLINT misc-const-correctness

#ifdef Q_OS_MACOS
    selectors += QStringLiteral("nativemenu");
#endif

    QQmlApplicationEngine engine;
    auto* qmlFileSelector = new QQmlFileSelector(&engine);
    qmlFileSelector->setExtraSelectors(selectors);

    // Temporary message handler to capture any error output so that
    // it can be shown to the user using a graphical message box
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg)
    {
        const int r = fprintf(stderr, "%s\n", msg.toLocal8Bit().constData()); Q_ASSERT(r >= 0);
        qmlError += QStringLiteral("%1\n").arg(msg);
    });

    engine.addImportPath(QStringLiteral("qrc:///qml"));
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    qInstallMessageHandler(nullptr);

    if(engine.rootObjects().empty())
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("The user interface failed to load:\n\n%1").arg(qmlError),
            QMessageBox::Close);
        return 2;
    }

    auto rootObjects = engine.rootObjects();
    QObject* mainWindow = rootObjects.first();
    QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived,
    mainWindow, [mainWindow](const QString& message, QObject*)
    {
        auto arguments = message.split(QStringLiteral("\n"));
        arguments.pop_front(); // Executable

        QMetaObject::invokeMethod(mainWindow, "processArguments", Q_ARG(QVariant, arguments));
    });

    MacOsFileOpenEventFilter macOsfileOpenEventFilter;
    app.installEventFilter(&macOsfileOpenEventFilter);
    QObject::connect(&macOsfileOpenEventFilter, &MacOsFileOpenEventFilter::externalOpen,
    mainWindow, [mainWindow](const QString& argument)
    {
        QMetaObject::invokeMethod(mainWindow, "processArguments", Q_ARG(QVariant, QStringList{argument}));
    });

    int qmlExitCode = 0;
    QObject::connect(&engine, &QQmlApplicationEngine::exit,
        [&qmlExitCode](int code) { qmlExitCode = code; });

    const Watchdog watchDog;

    // Poke the watch dog every now and again so that it doesn't break/crash us
    QTimer keepAliveTimer;
    QObject::connect(&keepAliveTimer, &QTimer::timeout, &watchDog, &Watchdog::reset);
    keepAliveTimer.start(1s);

#ifndef _DEBUG
    CrashHandler c(Application::resolvedExe(QStringLiteral("CrashReporter")));
    c.onCrash([mainWindow, &consoleOutputFiles](const QString& directory)
    {
        QVariant state;

        bool success = QMetaObject::invokeMethod(mainWindow, "currentState",
            Qt::DirectConnection, Q_RETURN_ARG(QVariant, state));

        if(success)
        {
            QFile file(QDir(directory).filePath("state.txt"));
            std::cerr << "Writing " << file.fileName().toStdString() << "\n";

            file.open(QIODevice::ReadWrite);
            QTextStream stream(&file);
            stream << state.toString();
            file.close();
        }
        else
        {
            auto index = mainWindow->metaObject()->indexOfMethod("currentState()");
            std::cerr << "Failed to invoke 'currentState' (" << index << ")\n";
        }

        for(const auto& consoleOutputFile : consoleOutputFiles)
        {
            consoleOutputFile->close();
            auto fileInfo = QFileInfo(consoleOutputFile->filename());

            // No point in submitting it if it's empty
            if(fileInfo.size() == 0)
                continue;

            QFile::copy(consoleOutputFile->filename(), QDir(directory).filePath(fileInfo.fileName()));
        }

        auto settingsFileName = u::settingsFileName();

        if(!settingsFileName.isEmpty())
            QFile::copy(settingsFileName, QDir(directory).filePath("settings.txt"));
    });
#else
    Q_UNUSED(consoleOutputFiles);
#endif

    auto exitCode = QCoreApplication::exec();
    return qmlExitCode != 0 ? qmlExitCode : exitCode;
}

int main(int argc, char *argv[])
{
    u::setAppPathName(argv[0]);

    QCoreApplication::setOrganizationName(QStringLiteral("Graphia"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("graphia.app"));
    QCoreApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    auto mode = enableConsoleMode();

    auto consoleOutputFiles = captureConsoleOutput(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));

    // The "real" main is separate to limit the scope of QtSingleApplication,
    // otherwise a restart causes the exiting instance to get activated
    auto exitCode = start(argc, argv, consoleOutputFiles);

    restoreConsoleMode(mode);

    if(static_cast<ExitType>(exitCode) == ExitType::Restart)
    {
        auto exeName = resolvedExeName(argv[0]);

        if(Updater::updateAvailable() && Updater::showUpdatePrompt({exeName}))
        {
            // If there is an update available, save a bit of time by
            // skipping the restart and starting the updater directly
            std::cerr << "Restarting to install update...\n";
        }
        else
        {
            std::cerr << "Restarting " << exeName.toStdString() << "...\n";
            if(!QProcess::startDetached(exeName, {}))
                std::cerr << "  ...failed\n";
        }
    }

    return exitCode;
}
