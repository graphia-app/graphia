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

#include "rendering/openglfunctions.h"

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

    const char* uri = "com.kajeka";
    const int maj = 1;
    const int min = 0;

    qmlRegisterType<Application>(uri, maj, min, "Application");
    qmlRegisterType<Document>(uri, maj, min, "Document");
    qmlRegisterType<GraphQuickItem>(uri, maj, min, "Graph");
    qmlRegisterType<GraphTransformConfiguration>(uri, maj, min, "GraphTransform");
    qmlRegisterType<LayoutSetting>(uri, maj, min, "LayoutSetting");

    ThreadPool threadPool;

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    return app.exec();
}
