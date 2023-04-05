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

using namespace Qt::Literals::StringLiterals;
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
        .filePath(u"%1.desktop"_s.arg(Application::name()));
    auto dotDesktopFile = QFile(dotDesktopFilename);

    auto iconsDirname = QDir(genericDirname).filePath(u"icons"_s);
    auto iconsDir = QDir(iconsDirname);
    auto iconFilename = iconsDir.filePath(u"%1.svg"_s.arg(Application::name()));
    auto iconPermissions = QFileDevice::ReadOwner|QFileDevice::WriteOwner|
        QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther;

    auto xdgMimeArguments = QStringList
    {
        u"default"_s,
        u"%1.desktop"_s.arg(Application::name()),
        u"x-scheme-handler/%1"_s.arg(Application::nativeExtension())
    };

    auto success = ((iconsDir.exists() || iconsDir.mkpath(iconsDir.absolutePath())) &&
        dotDesktopFile.open(QIODevice::WriteOnly) && dotDesktopFile.write(dotDesktopFileContent.toUtf8()) >= 0 &&
        (QFileInfo::exists(iconFilename) ||
            (QFile::copy(u":/icon/Icon.svg"_s, iconFilename) &&
            QFile::setPermissions(iconFilename, iconPermissions))) &&
        QProcess::startDetached(u"xdg-mime"_s, xdgMimeArguments)) || false;

    if(!success)
        std::cerr << "Failed to configure for XDG.\n";
#endif
}

static void configureProxy()
{
    QNetworkProxy::ProxyType type = QNetworkProxy::DefaultProxy;
    auto typePref = u::pref(u"proxy/type"_s).toString();

    if(typePref == u"http"_s)
        type = QNetworkProxy::HttpProxy;
    else if(typePref == u"socks5"_s)
        type = QNetworkProxy::Socks5Proxy;

    QNetworkProxy proxy;
    proxy.setType(type);

    if(type != QNetworkProxy::DefaultProxy)
    {
        proxy.setHostName(u::pref(u"proxy/host"_s).toString());
        proxy.setPort(static_cast<quint16>(u::pref(u"proxy/port"_s).toUInt()));
        proxy.setUser(u::pref(u"proxy/username"_s).toString());
        proxy.setPassword(u::pref(u"proxy/password"_s).toString());
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
        if(app.sendMessage(QCoreApplication::arguments().join(u"\n"_s)))
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

    auto dontUpdate = commandLineParser.isSet(u"dontUpdate"_s) ||
        commandLineParser.isSet(u"parameters"_s);

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

    if(commandLineParser.isSet(u"startMaximised"_s))
        u::setPref(u"window/maximised"_s, true);

    if(commandLineParser.isSet(u"skipWelcome"_s) && !u::prefExists(u"tracking/permission"_s))
        u::setPref(u"tracking/permission"_s, u"anonymous"_s);

    QGuiApplication::styleHints()->setMousePressAndHoldInterval(500);

    QIcon mainIcon;
    mainIcon.addFile(u":/icon/Icon512x512.png"_s);
    mainIcon.addFile(u":/icon/Icon256x256.png"_s);
    mainIcon.addFile(u":/icon/Icon128x128.png"_s);
    mainIcon.addFile(u":/icon/Icon64x64.png"_s);
    mainIcon.addFile(u":/icon/Icon32x32.png"_s);
    mainIcon.addFile(u":/icon/Icon16x16.png"_s);
    QApplication::setWindowIcon(mainIcon);

    QIcon::setThemeName(u"Tango"_s);

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
            vendor.replace(u" "_s, u"+"_s);
            const QString driversUrl = uR"(https://www.google.com/search?q=%1+video+driver+download&btnI)"_s.arg(vendor);
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

    if(commandLineParser.isSet(u"parameters"_s))
    {
        installSignalHandlers();

        auto parametersFilename = commandLineParser.value(u"parameters"_s);
        Headless headless(commandLineParser.positionalArguments(), parametersFilename);

        QTimer::singleShot(0, &headless, &Headless::run);
        QObject::connect(&headless, &Headless::done,
            QCoreApplication::instance(), &QCoreApplication::quit);
        QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
            &headless, &Headless::cancel);

        return QCoreApplication::exec();
    }

    u::definePref(u"visuals/defaultNodeColor"_s,                "#0000FF");
    u::definePref(u"visuals/defaultEdgeColor"_s,                "#FFFFFF");
    u::definePref(u"visuals/multiElementColor"_s,               "#FF0000");
    u::definePref(u"visuals/backgroundColor"_s,                 "#C0C0C0");
    u::definePref(u"visuals/highlightColor"_s,                  "#FFFFFF");

    u::definePref(u"visuals/defaultNormalNodeSize"_s,           0.333);
    u::definePref(u"visuals/defaultNormalEdgeSize"_s,           0.25);

    u::definePref(u"visuals/showNodeText"_s,                    QVariant::fromValue(static_cast<int>(TextState::Selected)));
    u::definePref(u"visuals/showEdgeText"_s,                    QVariant::fromValue(static_cast<int>(TextState::Selected)));
    u::definePref(u"visuals/textFont"_s,                        SharedTools::QtSingleApplication::font().family());
    u::definePref(u"visuals/textSize"_s,                        24.0f);
    u::definePref(u"visuals/edgeVisualType"_s,                  QVariant::fromValue(static_cast<int>(EdgeVisualType::Cylinder)));
    u::definePref(u"visuals/textAlignment"_s,                   QVariant::fromValue(static_cast<int>(TextAlignment::Right)));
    u::definePref(u"visuals/showMultiElementIndicators"_s,      true);
    u::definePref(u"visuals/savedGradients"_s,                  Defaults::GRADIENT_PRESETS);
    u::definePref(u"visuals/defaultGradient"_s,                 Defaults::GRADIENT);
    u::definePref(u"visuals/savedPalettes"_s,                   Defaults::PALETTE_PRESETS);
    u::definePref(u"visuals/defaultPalette"_s,                  Defaults::PALETTE);

    u::definePref(u"visuals/projection"_s,                      QVariant::fromValue(static_cast<int>(Projection::Perspective)));

    u::definePref(u"visuals/minimumComponentRadius"_s,          2.0);
    u::definePref(u"visuals/transitionTime"_s,                  1.0);

    u::definePref(u"visuals/disableMultisampling"_s,            false);

    u::definePref(u"misc/maxUndoLevels"_s,                      25);

    u::definePref(u"misc/showGraphMetrics"_s,                   false);
    u::definePref(u"misc/showLayoutSettings"_s,                 false);

    u::definePref(u"misc/focusFoundNodes"_s,                    true);
    u::definePref(u"misc/focusFoundComponents"_s,               true);
    u::definePref(u"misc/stayInComponentMode"_s,                false);

    u::definePref(u"misc/disableHubbles"_s,                     false);

    u::definePref(u"misc/hasSeenTutorial"_s,                    false);

    u::definePref(u"misc/autoBackgroundUpdateCheck"_s,          true);

    u::definePref(u"find/findByAttributeSortLexically"_s,       true);

    u::definePref(u"screenshot/width"_s,                        1920);
    u::definePref(u"screenshot/height"_s,                       1080);
    u::definePref(u"screenshot/path"_s,
        QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString());

    u::definePref(u"servers/redirects"_s,                       "https://redirects.graphia.app");
    u::definePref(u"servers/updates"_s,                         "https://updates.graphia.app");
    u::definePref(u"servers/crashreports"_s,                    "https://crashreports.graphia.app");
    u::definePref(u"servers/tracking"_s,                        "https://tracking.graphia.app");

    u::definePref(u"proxy/type"_s,                              "disabled");

    u::definePref(u"system/uiTheme"_s,                          "Default");

    u::updateOldPrefs();

    const PreferencesWatcher preferencesWatcher;

    QObject::connect(&preferencesWatcher, &PreferencesWatcher::preferenceChanged,
        [](const QString& key, const QVariant&) { if(key.startsWith(u"proxy"_s)) configureProxy(); });

    configureProxy();

    QQuickStyle::setStyle(u::pref(u"system/uiTheme"_s).toString());

    QStringList selectors; // NOLINT misc-const-correctness

#ifdef Q_OS_MACOS
    selectors += u"nativemenu"_s;
#endif

    QQmlApplicationEngine engine;
    auto* qmlFileSelector = new QQmlFileSelector(&engine);
    qmlFileSelector->setExtraSelectors(selectors);

    // Temporary message handler to capture any error output so that
    // it can be shown to the user using a graphical message box
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg)
    {
        const int r = fprintf(stderr, "%s\n", msg.toLocal8Bit().constData()); Q_ASSERT(r >= 0);
        qmlError += u"%1\n"_s.arg(msg);
    });

    engine.addImportPath(u"qrc:///qml"_s);
    engine.load(QUrl(u"qrc:///qml/main.qml"_s));

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
        auto arguments = message.split(u"\n"_s);
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
    CrashHandler c(Application::resolvedExe(u"CrashReporter"_s));
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

    QCoreApplication::setOrganizationName(u"Graphia"_s);
    QCoreApplication::setOrganizationDomain(u"graphia.app"_s);
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
