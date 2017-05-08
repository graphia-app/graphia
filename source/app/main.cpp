#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include <QtGlobal>
#include <QIcon>
#include <QMessageBox>
#include <QStyleHints>
#include <QGuiApplication>
#include <QWidget>
#include <QWindow>
#include <QScreen>
#include <QDir>

#include <iostream>

#include "application.h"
#include "ui/document.h"
#include "ui/graphquickitem.h"
#include "ui/visualisations/defaultgradients.h"

#include "shared/utils/threadpool.h"
#include "shared/utils/preferences.h"

#include "rendering/openglfunctions.h"
#include "rendering/graphrenderer.h"

#include "thirdparty/qtsingleapplication/qtsingleapplication.h"
#include "thirdparty/breakpad/crashhandler.h"

int main(int argc, char *argv[])
{
    SharedTools::QtSingleApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    OpenGLFunctions::setDefaultFormat();

    if(qEnvironmentVariableIsSet("VOGL_CMD_LINE"))
        qDebug() << "Vogl detected; disabling shared OpenGL context (QtWebEngine will not function!)";
    else
        SharedTools::QtSingleApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    SharedTools::QtSingleApplication app(PRODUCT_NAME, argc, argv);

    if(app.isRunning())
    {
        if(app.sendMessage(app.arguments().join("\n")))
            return 0;
    }

    // Wait until the application is active before setting the focus window
    QObject::connect(&app, &SharedTools::QtSingleApplication::applicationStateChanged,
    [&app]
    {
        if(app.activationWindow() == nullptr)
            app.setActivationWindow(QApplication::focusWindow());
    });

    QCoreApplication::setOrganizationName("Kajeka");
    QCoreApplication::setOrganizationDomain("kajeka.com");
    QCoreApplication::setApplicationName(PRODUCT_NAME);
    QCoreApplication::setApplicationVersion(VERSION);

    QGuiApplication::styleHints()->setMousePressAndHoldInterval(200);

    QIcon mainIcon;
    mainIcon.addFile(":/icon/Icon512x512.png");
    mainIcon.addFile(":/icon/Icon256x256.png");
    mainIcon.addFile(":/icon/Icon128x128.png");
    mainIcon.addFile(":/icon/Icon64x64.png");
    mainIcon.addFile(":/icon/Icon32x32.png");
    mainIcon.addFile(":/icon/Icon16x16.png");
    app.setWindowIcon(mainIcon);

    // Since Qt is responsible for managing OpenGL, we need
    // to give it a hint that we want a debug context
    if(qgetenv("OPENGL_DEBUG").toInt() > 0)
        qputenv("QSG_OPENGL_DEBUG", "1");

#ifndef Q_OS_LINUX
    QIcon::setThemeName("Tango");
#endif

    if(!OpenGLFunctions::hasOpenGLSupport())
    {
        QString vendor = OpenGLFunctions::vendor();
        vendor.replace(" ", "+");
        QString driversUrl = QString(R"(http://www.google.com/search?q=%1+video+driver+download&btnI)").arg(vendor);

        QMessageBox messageBox(QMessageBox::Critical, QObject::tr("OpenGL support"),
            QObject::tr("The installed version of OpenGL is insufficient to run %1. "
                        R"(Please install the latest <a href="%2">video drivers</a> available from )" //
                        "your vendor and try again.").arg(Application::name()).arg(driversUrl),
            QMessageBox::Close);

        messageBox.setTextFormat(Qt::RichText);
        messageBox.exec();

        return 1;
    }

    const char* uri = Application::uri();
    const int maj = Application::majorVersion();
    const int min = Application::minorVersion();

    qmlRegisterType<Application>                (uri, maj, min, "Application");
    qmlRegisterType<Document>                   (uri, maj, min, "Document");
    qmlRegisterType<GraphQuickItem>             (uri, maj, min, "Graph");
    qmlRegisterType<LayoutSetting>              (uri, maj, min, "LayoutSetting");
    qmlRegisterType<QmlPreferences>             (uri, maj, min, "Preferences");

    qRegisterMetaType<size_t>("size_t");

    ThreadPoolSingleton threadPool;
    Preferences preferences;

    preferences.define("visuals/defaultNodeColor",              "#0000FF");
    preferences.define("visuals/defaultEdgeColor",              "#FFFFFF");
    preferences.define("visuals/multiElementColor",             "#FF0000");
    preferences.define("visuals/backgroundColor",               "#C0C0C0");
    preferences.define("visuals/highlightColor",                "#FFFFFF");

    preferences.define("visuals/defaultNodeSize",               0.6, 0.1,  3.0);
    preferences.define("visuals/defaultEdgeSize",               0.2, 0.01, 2.0);

    preferences.define("visuals/showNodeText",                  QVariant::fromValue(static_cast<int>(TextState::Selected)));
    preferences.define("visuals/showEdgeText",                  QVariant::fromValue(static_cast<int>(TextState::Selected)));
    preferences.define("visuals/textFont",                      SharedTools::QtSingleApplication::font().family());
    preferences.define("visuals/textSize",                      24.0f);
    preferences.define("visuals/edgeVisualType",                QVariant::fromValue(static_cast<int>(EdgeVisualType::Cylinder)));
    preferences.define("visuals/textAlignment",                 QVariant::fromValue(static_cast<int>(TextAlignment::Right)));
    preferences.define("visuals/showMultiElementIndicators",    true);
   preferences.define("visuals/savedGradients",                 DEFAULT_GRADIENTS);

    preferences.define("visuals/minimumComponentRadius",        2.0, 0.05, 15.0);
    preferences.define("visuals/transitionTime",                1.0, 0.1, 5.0);

    preferences.define("misc/showGraphMetrics",                 false);
    preferences.define("misc/showLayoutSettings",               false);

    preferences.define("misc/focusFoundNodes",                  true);
    preferences.define("misc/focusFoundComponents",             false);
    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:///qml");
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));
    if(engine.rootObjects().empty())
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("The user interface failed to load."),
                              QMessageBox::Close);
        return 2;
    }

    QObject* mainWindow = engine.rootObjects().first();
    QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived,
    [mainWindow](const QString& message, QObject*)
    {
        QMetaObject::invokeMethod(mainWindow, "processArguments", Q_ARG(QVariant, message.split("\n")));
    });

#ifndef _DEBUG
    CrashHandler c;
    c.onCrash([](const QString& directory)
    {
        // Take screenshots of all the open windows
        for(auto* window : QGuiApplication::allWindows())
        {
            if(!window->isVisible())
                continue;

            QString fileName = QDir(directory).filePath(
                QString("%1.png").arg(window->title().replace(" ", "_")));

            std::cerr << "Writing " << fileName.toStdString() << "\n";

            auto screen = window->screen();
            if(screen == nullptr)
                continue;

            auto pixmap = screen->grabWindow(window->winId());
            pixmap.save(fileName, "PNG");
        }
    });
#endif

    return app.exec();
}
