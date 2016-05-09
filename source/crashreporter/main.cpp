#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QUrl>
#include <QSysInfo>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDebug>

#include <map>
#include <iostream>

#include "report.h"
#include "../app/rendering/openglfunctions.h"

static void uploadReport(const QString& email, const QString& text, const QString& dmpFile)
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    std::map<const char*, QString> fields =
    {
        {"email",   email},
        {"text",    text},
        {"product", PRODUCT_NAME},
        {"version", VERSION},
        {"os",      QString("%1 %2 %3 %4").arg(QSysInfo::kernelType())
                        .arg(QSysInfo::kernelVersion())
                        .arg(QSysInfo::productType())
                        .arg(QSysInfo::productVersion())},
        {"gl",      OpenGLFunctions::info()},
    };

    QCryptographicHash checksum(QCryptographicHash::Algorithm::Md5);

    for(auto& field : fields)
    {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QString("form-data; name=\"%1\"").arg(field.first)));
        part.setBody(field.second.toLatin1());
        multiPart->append(part);

        checksum.addData(field.second.toLatin1());
    }

    // Send a hash of the contents of the report as a (crude) means of filtering bots/crawlers/etc.
    QHttpPart checksumPart;
    checksumPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant(QString("form-data; name=\"checksum\"")));
    checksumPart.setBody(checksum.result().toHex());
    multiPart->append(checksumPart);

    QHttpPart dmpPart;
    dmpPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    dmpPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant("form-data; name=\"dmp\"; filename=\"" +
                               QFileInfo(dmpFile).fileName() + "\""));
    QFile* file = new QFile(dmpFile);
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

    if(app.arguments().size() != 2 || !QFileInfo(app.arguments().at(1)).exists())
    {
        QMessageBox::critical(nullptr, app.applicationName(),
                              QObject::tr("This program is intended for automatically "
                                          "reporting crashes and should not be invoked directly."),
                              QMessageBox::Close);
        return 1;
    }

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
