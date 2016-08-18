#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include <QtGlobal>
#include <QIcon>
#include <QMessageBox>

#include "application.h"
#include "ui/document.h"
#include "ui/graphquickitem.h"
#include "ui/graphtransformconfiguration.h"

#include "shared/utils/threadpool.h"
#include "shared/utils/preferences.h"

#include "rendering/openglfunctions.h"

#include "../thirdparty/qtsingleapplication/qtsingleapplication.h"
#include "../thirdparty/SortFilterProxyModel/qqmlsortfilterproxymodel.h"
#include "../thirdparty/breakpad/crashhandler.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    SharedTools::QtSingleApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

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
        QMessageBox::critical(nullptr, QObject::tr("OpenGL support"),
                              QObject::tr("The installed version of OpenGL is insufficient to run %1. "
                                          "Please install the latest video drivers available from "
                                          "your vendor and try again.").arg(Application::name()), QMessageBox::Close);
        return 1;
    }

    const char* uri = Application::uri();
    const int maj = Application::majorVersion();
    const int min = Application::minorVersion();

    qmlRegisterType<Application>                (uri, maj, min, "Application");
    qmlRegisterType<Document>                   (uri, maj, min, "Document");
    qmlRegisterType<GraphQuickItem>             (uri, maj, min, "Graph");
    qmlRegisterType<GraphTransformConfiguration>(uri, maj, min, "GraphTransform");
    qmlRegisterType<LayoutSetting>              (uri, maj, min, "LayoutSetting");
    qmlRegisterType<QmlPreferences>             (uri, maj, min, "Preferences");

    ThreadPool threadPool;
    Preferences preferences;

    preferences.define("visuals/defaultNodeColor",          "#0000FF");
    preferences.define("visuals/defaultEdgeColor",          "#FFFFFF");
    preferences.define("visuals/multiElementColor",         "#FF0000");
    preferences.define("visuals/backgroundColor",           "#C0C0C0");
    preferences.define("visuals/highlightColor",            "#FFFFFF");

    preferences.define("visuals/defaultNodeSize",           0.6, 0.1,  2.0);
    preferences.define("visuals/defaultEdgeSize",           0.2, 0.02, 2.0);

    preferences.define("visuals/minimumComponentRadius",    2.0, 0.05, 15.0);
    preferences.define("visuals/transitionTime",            1.0, 0.1, 5.0);

    preferences.define("misc/showGraphMetrics",             false);
    preferences.define("misc/showLayoutSettings",           false);

    preferences.define("misc/focusFoundNodes",              true);

    QQmlSortFilterProxyModel::initialize();

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));
    Q_ASSERT(!engine.rootObjects().empty());

    QObject* mainWindow = engine.rootObjects().first();
    QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived,
    [mainWindow](const QString& message, QObject*)
    {
        QMetaObject::invokeMethod(mainWindow, "processArguments", Q_ARG(QVariant, message.split("\n")));
    });

#ifndef _DEBUG
    CrashHandler c;
    Q_UNUSED(c);
#endif

    return app.exec();
}
