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

#include "rendering/openglfunctions.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Since Qt is responsible for managing OpenGL, we need
    // to give it a hint that we want debug context
    if(qgetenv("OPENGL_DEBUG").toInt() > 0)
        qputenv("QSG_OPENGL_DEBUG", "1");

#ifndef Q_OS_UNIX
    QIcon::setThemeName("Tango");
#endif

    bool hasOpenGLSupport = OpenGLFunctions::checkOpenGLSupport();
    Q_UNUSED(hasOpenGLSupport); //FIXME do something with this

    qmlRegisterType<Application>("com.kajeka", 1, 0, "Application");
    qmlRegisterType<Document>("com.kajeka", 1, 0, "Document");
    qmlRegisterType<GraphQuickItem>("com.kajeka", 1, 0, "Graph");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    return app.exec();
}
