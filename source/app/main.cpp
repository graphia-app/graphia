#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtGlobal>
#include <QIcon>
#include <QMessageBox>
#include <QStyleHints>
#include <QGuiApplication>
#include <QWidget>
#include <QWindow>
#include <QScreen>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QCommandLineParser>
#include <QProcess>

#include <iostream>

#include "application.h"
#include "ui/document.h"
#include "ui/graphquickitem.h"
#include "ui/visualisations/defaultgradients.h"
#include "ui/visualisations/defaultpalettes.h"
#include "ui/hovermousepassthrough.h"
#include "ui/enrichmentheatmapitem.h"
#include "ui/iconitem.h"

#include "shared/utils/threadpool.h"
#include "shared/utils/preferences.h"
#include "shared/utils/qmlutils.h"
#include "shared/utils/scopetimer.h"

#include "rendering/openglfunctions.h"
#include "rendering/graphrenderer.h"

#include "updates/updater.h"

#include <qtsingleapplication/qtsingleapplication.h>
#include <breakpad/crashhandler.h>

#include "watchdog.h"

static QString resolvedExeName(const QString& baseExeName)
{
#ifdef Q_OS_LINUX
    if(qEnvironmentVariableIsSet("APPIMAGE"))
        return qgetenv("APPIMAGE");
#endif

    return baseExeName;
}

int start(int argc, char *argv[])
{
    SharedTools::QtSingleApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    OpenGLFunctions::setDefaultFormat();

    if(qEnvironmentVariableIsSet("VOGL_CMD_LINE"))
        qDebug() << "Vogl detected; disabling shared OpenGL context (QtWebEngine will not function!)";
    else
        SharedTools::QtSingleApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    SharedTools::QtSingleApplication app(QStringLiteral(PRODUCT_NAME), argc, argv);

    Application::setAppDir(QCoreApplication::applicationDirPath());

    if(app.isRunning())
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

    QCoreApplication::setOrganizationName(QStringLiteral("Kajeka"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kajeka.com"));
    QCoreApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    QCommandLineParser commandLineParser;

    commandLineParser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    commandLineParser.addHelpOption();
    commandLineParser.addOptions(
    {
        {{"u", "dontUpdate"}, QObject::tr("Don't update now, but remind later.")}
    });

    commandLineParser.process(QCoreApplication::arguments());

    Q_INIT_RESOURCE(update_keys);

    if(!commandLineParser.isSet(QStringLiteral("dontUpdate")) && Updater::updateAvailable())
    {
        QStringList restartArguments = QCoreApplication::arguments();
        restartArguments[0] = resolvedExeName(restartArguments.at(0));

        if(Updater::showUpdatePrompt(restartArguments))
        {
            // The updater will restart the application once finished, so quit now
            return 0;
        }
    }

#ifdef Q_OS_MACOS
    // NativeTextRendering generally looks better on MacOS
    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QGuiApplication::styleHints()->setMousePressAndHoldInterval(500);

    QIcon mainIcon;
    mainIcon.addFile(QStringLiteral(":/icon/Icon512x512.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon256x256.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon128x128.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon64x64.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon32x32.png"));
    mainIcon.addFile(QStringLiteral(":/icon/Icon16x16.png"));
    QApplication::setWindowIcon(mainIcon);

#ifndef Q_OS_LINUX
    QIcon::setThemeName("Tango");
#endif

    // Since Qt is responsible for managing OpenGL, we need
    // to give it a hint that we want a debug context
    if(qEnvironmentVariableIntValue("OPENGL_DEBUG") > 0)
        qputenv("QSG_OPENGL_DEBUG", "1");

    if(!OpenGLFunctions::hasOpenGLSupport())
    {
        QString vendor = OpenGLFunctions::vendor();
        vendor.replace(QLatin1String(" "), QLatin1String("+"));
        QString driversUrl = QStringLiteral(R"(http://www.google.com/search?q=%1+video+driver+download&btnI)").arg(vendor);

        QMessageBox messageBox(QMessageBox::Critical, QObject::tr("OpenGL support"),
            QObject::tr("The installed version of OpenGL is insufficient to run %1. "
                        R"(Please install the latest <a href="%2">video drivers</a> available from )"
                        "your vendor and try again.").arg(Application::name(), driversUrl),
            QMessageBox::Close);

        messageBox.setTextFormat(Qt::RichText);
        messageBox.exec();

        return 1;
    }

    const char* uri = Application::uri();
    const int maj = Application::majorVersion();
    const int min = Application::minorVersion();

    qmlRegisterType<Application>                     (uri, maj, min, "Application");
    qmlRegisterType<Document>                        (uri, maj, min, "Document");
    qmlRegisterType<GraphQuickItem>                  (uri, maj, min, "Graph");
    qmlRegisterType<IconItem>                        (uri, maj, min, "NamedIcon");
    qmlRegisterType<QmlPreferences>                  (uri, maj, min, "Preferences");
    qmlRegisterType<HoverMousePassthrough>           (uri, maj, min, "HoverMousePassthrough");
    qmlRegisterType<EnrichmentHeatmapItem>           (uri, maj, min, "EnrichmentHeatmap");
    qmlRegisterUncreatableType<EnrichmentTableModel> (uri, maj, min, "EnrichmentRoles",
                                                      QStringLiteral("Exposed purely for results Enumerator"));

    qmlRegisterSingletonType<QmlUtils>               (uri, maj, min, "QmlUtils", &QmlUtils::qmlInstance);

    qRegisterMetaType<size_t>("size_t");

    ThreadPoolSingleton threadPool;
    Preferences preferences;
    ScopeTimerManager scopeTimerManager;

    preferences.define(QStringLiteral("visuals/defaultNodeColor"),              "#0000FF");
    preferences.define(QStringLiteral("visuals/defaultEdgeColor"),              "#FFFFFF");
    preferences.define(QStringLiteral("visuals/multiElementColor"),             "#FF0000");
    preferences.define(QStringLiteral("visuals/backgroundColor"),               "#C0C0C0");
    preferences.define(QStringLiteral("visuals/highlightColor"),                "#FFFFFF");

    preferences.define(QStringLiteral("visuals/defaultNodeSize"),               1.5, 0.1,  3.0);
    preferences.define(QStringLiteral("visuals/defaultEdgeSize"),               0.5, 0.01, 2.0);

    preferences.define(QStringLiteral("visuals/showNodeText"),                  QVariant::fromValue(static_cast<int>(TextState::Selected)));
    preferences.define(QStringLiteral("visuals/showEdgeText"),                  QVariant::fromValue(static_cast<int>(TextState::Selected)));
    preferences.define(QStringLiteral("visuals/textFont"),                      SharedTools::QtSingleApplication::font().family());
    preferences.define(QStringLiteral("visuals/textSize"),                      24.0f);
    preferences.define(QStringLiteral("visuals/edgeVisualType"),                QVariant::fromValue(static_cast<int>(EdgeVisualType::Cylinder)));
    preferences.define(QStringLiteral("visuals/textAlignment"),                 QVariant::fromValue(static_cast<int>(TextAlignment::Right)));
    preferences.define(QStringLiteral("visuals/showMultiElementIndicators"),    true);
    preferences.define(QStringLiteral("visuals/savedGradients"),                Defaults::GRADIENT_PRESETS);
    preferences.define(QStringLiteral("visuals/defaultGradient"),               Defaults::GRADIENT);
    preferences.define(QStringLiteral("visuals/savedPalettes"),                 Defaults::PALETTE_PRESETS);
    preferences.define(QStringLiteral("visuals/defaultPalette"),                Defaults::PALETTE);

    preferences.define(QStringLiteral("visuals/minimumComponentRadius"),        2.0, 0.05, 15.0);
    preferences.define(QStringLiteral("visuals/transitionTime"),                1.0, 0.1, 5.0);

    preferences.define(QStringLiteral("misc/maxUndoLevels"),                    25);

    preferences.define(QStringLiteral("misc/showGraphMetrics"),                 false);
    preferences.define(QStringLiteral("misc/showLayoutSettings"),               false);

    preferences.define(QStringLiteral("misc/focusFoundNodes"),                  true);
    preferences.define(QStringLiteral("misc/focusFoundComponents"),             true);

    preferences.define(QStringLiteral("misc/disableHubbles"),                   false);

    preferences.define(QStringLiteral("misc/webSearchEngineUrl"),               "https://www.google.com/#q=%1");

    preferences.define(QStringLiteral("misc/hasSeenTutorial"),                  false);

    preferences.define(QStringLiteral("misc/autoBackgroundUpdateCheck"),        true);

    preferences.define(QStringLiteral("screenshot/width"),                      1920);
    preferences.define(QStringLiteral("screenshot/height"),                     1080);
    preferences.define(QStringLiteral("screenshot/path"),
        QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString());

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:///qml"));
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));
    if(engine.rootObjects().empty())
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("The user interface failed to load."),
                              QMessageBox::Close);
        return 2;
    }

    auto rootObjects = engine.rootObjects();
    QObject* mainWindow = rootObjects.first();
    QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived,
    mainWindow, [mainWindow](const QString& message, QObject*)
    {
        QMetaObject::invokeMethod(mainWindow, "processArguments",
            Q_ARG(QVariant, message.split(QStringLiteral("\n"))));
    });

    int qmlExitCode = 0;
    QObject::connect(&engine, &QQmlApplicationEngine::exit,
        [&qmlExitCode](int code) { qmlExitCode = code; });

    Watchdog watchDog;

    // Poke the watch dog every now and again so that it doesn't break/crash us
    QTimer keepAliveTimer;
    QObject::connect(&keepAliveTimer, &QTimer::timeout, &watchDog, &Watchdog::reset);
    keepAliveTimer.start(1000);

#ifndef _DEBUG
    CrashHandler c(Application::resolvedExe(QStringLiteral("CrashReporter")));
    c.onCrash([](const QString& directory)
    {
        // Take screenshots of all the open windows
        const auto windows = QGuiApplication::allWindows();
        for(auto* window : windows)
        {
            if(!window->isVisible())
                continue;

            QString fileName = QDir(directory).filePath(
                QStringLiteral("%1.png").arg(window->title().replace(QLatin1String(" "), QLatin1String("_"))));

            std::cerr << "Writing " << fileName.toStdString() << "\n";

            auto screen = window->screen();
            if(screen == nullptr)
                continue;

            auto pixmap = screen->grabWindow(window->winId());
            pixmap.save(fileName, "PNG");
        }
    });
#endif

    auto exitCode = QCoreApplication::exec();
    return qmlExitCode != 0 ? qmlExitCode : exitCode;
}

int main(int argc, char *argv[])
{
    // The "real" main is separate to limit the scope of QtSingleApplication,
    // otherwise a restart causes the exiting instance to get activated
    auto exitCode = start(argc, argv);

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
