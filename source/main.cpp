#include <QApplication>
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

#include "utils/threadpool.h"
#include "utils/preferences.h"

#include "rendering/openglfunctions.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Kajeka");
    QCoreApplication::setOrganizationDomain("kajeka.com");
    QCoreApplication::setApplicationName("GraphTool");

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
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    return app.exec();
}
