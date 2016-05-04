#include <QApplication>
#include <QQmlApplicationEngine>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Kajeka");
    QCoreApplication::setOrganizationDomain("kajeka.com");
    QCoreApplication::setApplicationName(PRODUCT_NAME);
    QCoreApplication::setApplicationVersion(VERSION);

    QIcon mainIcon;
    mainIcon.addFile(":/icon.svg");
    app.setWindowIcon(mainIcon);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
