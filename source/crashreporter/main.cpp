#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QSysInfo>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDebug>
#include <QTimer>
#include <QDirIterator>
#include <QCommandLineParser>

#include <iostream>
#include <map>

#include "report.h"
#include "app/rendering/openglfunctions.h"
#include "shared/utils/preferences.h"

static void uploadReport(const QString& email, const QString& text,
                         const QString& dmpFile, const QString& attachmentDir)
{
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    std::map<const char*, QString> fields =
    {
        {"email",   email},
        {"text",    text},
        {"product", PRODUCT_NAME},
        {"version", VERSION},
        {"os",      QString("%1 %2 %3 %4").arg(QSysInfo::kernelType(),
                                               QSysInfo::kernelVersion(),
                                               QSysInfo::productType(),
                                               QSysInfo::productVersion())},
        {"gl",      OpenGLFunctions::info()},
    };

    QCryptographicHash checksum(QCryptographicHash::Algorithm::Md5);

    for(auto& field : fields)
    {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QStringLiteral(R"(form-data; name="%1")").arg(field.first)));
        part.setBody(field.second.toLatin1());
        multiPart->append(part);

        checksum.addData(field.second.toLatin1());
    }

    // Send a hash of the contents of the report as a (crude) means of filtering bots/crawlers/etc.
    QHttpPart checksumPart;
    checksumPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant(QStringLiteral(R"(form-data; name="checksum")")));
    checksumPart.setBody(checksum.result().toHex());
    multiPart->append(checksumPart);

    QHttpPart dmpPart;
    dmpPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    dmpPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant(QStringLiteral(R"(form-data; name="dmp"; filename="%1")")
                               .arg(QFileInfo(dmpFile).fileName())));
    auto* file = new QFile(dmpFile);
    file->open(QIODevice::ReadOnly);
    dmpPart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(dmpPart);

    QDirIterator dirIterator(attachmentDir);
    int fileIndex = 0;
    while(dirIterator.hasNext())
    {
        QString fileName = dirIterator.next();
        QFileInfo fileInfo(fileName);

        // Skip . and ..
        if(!fileInfo.exists() || !fileInfo.isFile())
            continue;

        QHttpPart attachmentPart;
        attachmentPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
        attachmentPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                 QVariant(QStringLiteral(R"(form-data; name="attachment%1"; filename="%2")")
                                    .arg(fileIndex++).arg(QFileInfo(fileName).fileName())));
        auto* attachment = new QFile(fileName);
        attachment->open(QIODevice::ReadOnly);
        attachmentPart.setBodyDevice(attachment);
        attachment->setParent(multiPart);
        multiPart->append(attachmentPart);
    }

    QUrl url(QStringLiteral("http://crashreports.kajeka.com/"));
    QNetworkRequest request(url);

    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.post(request, multiPart);
    multiPart->setParent(reply);

    QTimer timer;
    timer.setSingleShot(true);

    // Need a QEventLoop to drive upload
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(60000);
    loop.exec();

    if(timer.isActive())
    {
        timer.stop();

        if(reply->error() > 0)
        {
            QMessageBox::warning(nullptr, QApplication::applicationName(),
                                 QObject::tr("There was an error while uploading the crash "
                                             "report:\n%1").arg(reply->errorString()),
                                 QMessageBox::Close);
        }
    }
    else
    {
        QObject::disconnect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        reply->abort();
        QMessageBox::warning(nullptr, QApplication::applicationName(),
                             QObject::tr("Timed out while uploading the crash report. "
                                         "Please check your internet connection."),
                             QMessageBox::Close);
    }

    delete multiPart;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Kajeka"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kajeka.com"));
    QCoreApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    qmlRegisterType<QmlPreferences>(APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "Preferences");
    Preferences preferences;

    QCommandLineParser p;

    p.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    p.addHelpOption();
    p.addOption({{"s", "submit"}, QObject::tr("Submit the crash report immediately.")});
    p.addPositionalArgument(QStringLiteral("FILE"), QObject::tr("The crash report file."));
    p.addPositionalArgument(QStringLiteral("ATTACHMENTS"),
        QObject::tr("The attachments directory."), QStringLiteral("[ATTACHMENTS]"));

    p.process(QApplication::arguments());
    auto positional = p.positionalArguments();

    if(positional.isEmpty() || !QFileInfo::exists(positional.at(0)))
    {
        QMessageBox::critical(nullptr, QCoreApplication::applicationName(),
                              QObject::tr("This program is intended for automatically "
                                          "reporting crashes and should not be invoked directly."),
                              QMessageBox::Close);
        return 1;
    }

    QString attachmentsDir;
    if(positional.size() >= 2 && QFileInfo(positional.at(1)).isDir())
        attachmentsDir = positional.at(1);

    int exitCode = 0;
    Report report;

    if(!p.isSet(QStringLiteral("submit")))
    {
        QIcon mainIcon;
        mainIcon.addFile(QStringLiteral(":/icon.svg"));
        QApplication::setWindowIcon(mainIcon);

        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty(QStringLiteral("report"), &report);
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

        exitCode = QCoreApplication::exec();
    }
    else
    {
        report._email = preferences.get(QStringLiteral("auth/emailAddress")).toString();
        report._text = QObject::tr("Submitted directly with no user intervention.");
    }

    if(exitCode != 127)
        uploadReport(report._email, report._text, positional.at(0), attachmentsDir);

    return exitCode;
}
