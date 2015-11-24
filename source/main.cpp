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

#include "utils/threadpool.h"

#include "rendering/openglfunctions.h"

#include "ui/preferences.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication app(argc, argv);

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
                              QObject::tr("The version of OpenGL found is insufficient to run %1. "
                                          "Please install the latest video drivers available from "
                                          "your vendor.").arg(Application::name()), QMessageBox::Close);
        return 1;
    }

    qmlRegisterType<Application>("com.kajeka", 1, 0, "Application");
    qmlRegisterType<Document>("com.kajeka", 1, 0, "Document");
    qmlRegisterType<GraphQuickItem>("com.kajeka", 1, 0, "Graph");
    qmlRegisterType<LayoutParam>("com.kajeka", 1, 0, "LayoutParam");

    ThreadPool threadPool;
    Preferences preferences;

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    return app.exec();
}
