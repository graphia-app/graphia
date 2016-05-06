#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QUrl>

#include "report.h"

static void uploadReport(const QString& email, const QString& text, const QString& dmpFile)
{
    QFile* file = new QFile(dmpFile);

    if(!file->exists())
    {
        delete file;
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart emailPart;
    emailPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"email\""));
    emailPart.setBody(email.toLatin1());
    multiPart->append(emailPart);

    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"text\""));
    textPart.setBody(text.toLatin1());
    multiPart->append(textPart);

    QHttpPart versionPart;
    versionPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"version\""));
    versionPart.setBody(VERSION);
    multiPart->append(versionPart);

    QHttpPart dmpPart;
    dmpPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    dmpPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant("form-data; name=\"dmp\"; filename=\"" +
                               QFileInfo(dmpFile).completeBaseName() + "\""));
    file->open(QIODevice::ReadOnly);
    dmpPart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(dmpPart);

    QUrl url("http://crashreports.kajeka.com/");
    QNetworkRequest request(url);

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request, multiPart);
    multiPart->setParent(reply);

    // Need a QEventLoop to drive upload
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    delete multiPart;
}

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
    Report report;
    engine.rootContext()->setContextProperty("report", &report);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    int exitCode = app.exec();

    uploadReport(report._email, report._text, app.arguments().at(1));

    return exitCode;
}
