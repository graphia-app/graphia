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

#include <google_breakpad/processor/minidump.h>
#include <google_breakpad/processor/process_state.h>
#include <google_breakpad/processor/minidump_processor.h>
#include <google_breakpad/processor/call_stack.h>
#include <google_breakpad/processor/stack_frame.h>
#include <processor/pathname_stripper.h>

static QString crashedModule(const QString& dmpFile)
{
    using google_breakpad::Minidump;
    using google_breakpad::MinidumpMemoryList;
    using google_breakpad::MinidumpThreadList;
    using google_breakpad::MinidumpProcessor;
    using google_breakpad::ProcessState;
    using google_breakpad::CallStack;
    using google_breakpad::StackFrame;
    using google_breakpad::PathnameStripper;

    Minidump dump(dmpFile.toStdString());
    if(!dump.Read())
    {
       std::cerr << "Minidump " << dmpFile.toStdString() << " could not be read\n";
       return {};
    }

    MinidumpProcessor minidump_processor(nullptr, nullptr);
    ProcessState process_state;
    if(minidump_processor.Process(&dump, &process_state) != google_breakpad::PROCESS_OK)
    {
      std::cerr << "MinidumpProcessor::Process failed\n";
      return {};
    }

    int requesting_thread = process_state.requesting_thread();
    if(requesting_thread == -1 || !process_state.crashed())
    {
        std::cerr << "Process has not crashed\n";
        return {};
    }

    const auto* stack = process_state.threads()->at(requesting_thread);
    int frame_count = stack->frames()->size();
    if(frame_count == 0)
    {
        std::cerr << "No stack frames\n";
        return {};
    }

    const auto* frame = stack->frames()->at(0);
    if(!frame->module)
    {
        std::cerr << "No module\n";
        return {};
    }

    auto module = PathnameStripper::File(frame->module->code_file());
    return QString::fromStdString(module);
}

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
    p.addOptions(
    {
        {{"s", "submit"}, QObject::tr("Submit the crash report immediately.")},
        {{"d", "description"}, QObject::tr("A description of the crash."), "description", QString()}
    });
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
        engine.rootContext()->setContextProperty(
            QStringLiteral("glVendor"), OpenGLFunctions::vendor());
        engine.rootContext()->setContextProperty(
            QStringLiteral("crashedModule"), crashedModule(positional.at(0)));

        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

        exitCode = QCoreApplication::exec();
    }
    else
    {
        report._email = preferences.get(QStringLiteral("auth/emailAddress")).toString();
        report._text = p.value(QStringLiteral("description"));
    }

    if(exitCode != 127)
        uploadReport(report._email, report._text, positional.at(0), attachmentsDir);

    return exitCode;
}
