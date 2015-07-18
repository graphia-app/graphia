#include <QApplication>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include <QtGlobal>
#include <QIcon>

#include "application.h"
#include "ui/document.h"
#include "ui/graphquickitem.h"

#include "utils/threadpool.h"

#include "rendering/openglfunctions.h"

int main(int argc, char *argv[])
{
    qDebug() << "1";
    ThreadPool threadPool;
    qDebug() << "2";
    QApplication app(argc, argv);
    qDebug() << "3";

    // Since Qt is responsible for managing OpenGL, we need
    // to give it a hint that we want debug context
    if(qgetenv("OPENGL_DEBUG").toInt() > 0)
        qputenv("QSG_OPENGL_DEBUG", "1");

    qDebug() << "4";
#ifndef Q_OS_LINUX
    qDebug() << "1";
    QIcon::setThemeName("Tango");
#endif

    qDebug() << "5";
    bool hasOpenGLSupport = OpenGLFunctions::checkOpenGLSupport();
    Q_UNUSED(hasOpenGLSupport); //FIXME do something with this

    qDebug() << "6";
    qmlRegisterType<Application>("com.kajeka", 1, 0, "Application");
    qmlRegisterType<Document>("com.kajeka", 1, 0, "Document");
    qmlRegisterType<GraphQuickItem>("com.kajeka", 1, 0, "Graph");

    qDebug() << "7";
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    qDebug() << "8";
    return app.exec();
}
