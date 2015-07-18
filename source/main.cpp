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
    std::cout << "1";
    ThreadPool threadPool;
    std::cout << "2";
    QApplication app(argc, argv);
    std::cout << "3";

    // Since Qt is responsible for managing OpenGL, we need
    // to give it a hint that we want debug context
    if(qgetenv("OPENGL_DEBUG").toInt() > 0)
        qputenv("QSG_OPENGL_DEBUG", "1");

    std::cout << "4";
#ifndef Q_OS_LINUX
    std::cout << "1";
    QIcon::setThemeName("Tango");
#endif

    std::cout << "5";
    bool hasOpenGLSupport = OpenGLFunctions::checkOpenGLSupport();
    Q_UNUSED(hasOpenGLSupport); //FIXME do something with this

    std::cout << "6";
    qmlRegisterType<Application>("com.kajeka", 1, 0, "Application");
    qmlRegisterType<Document>("com.kajeka", 1, 0, "Document");
    qmlRegisterType<GraphQuickItem>("com.kajeka", 1, 0, "Graph");

    std::cout << "7";
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    std::cout << "8";
    return app.exec();
}
