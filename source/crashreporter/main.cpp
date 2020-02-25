/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    MinidumpProcessor minidumpProcessor(nullptr, nullptr);
    ProcessState process_state;
    if(minidumpProcessor.Process(&dump, &process_state) != google_breakpad::PROCESS_OK)
    {
      std::cerr << "MinidumpProcessor::Process failed\n";
      return {};
    }

    int requestingThread = process_state.requesting_thread();
    if(requestingThread == -1 || !process_state.crashed())
    {
        std::cerr << "Process has not crashed\n";
        return {};
    }

    const auto* stack = process_state.threads()->at(static_cast<size_t>(requestingThread));
    auto frameCount = stack->frames()->size();
    if(frameCount == 0)
    {
        std::cerr << "No stack frames\n";
        return {};
    }

    const StackFrame* frame = nullptr;
    size_t frameIndex = 0;
    std::string module;

    do
    {
        frame = stack->frames()->at(frameIndex);
        frameIndex++;

        module = frame->module != nullptr ?
            PathnameStripper::File(frame->module->code_file()) : "";

        // Treat module names that look like pointers as empty
        // It's unclear if this actually happens in practice, but
        // it doesn't hurt to test for it
        if(module.length() >= 2 && module[0] == '0' && module[1] == 'x')
            module.clear();
    }
    while(module.empty() && frameIndex < frameCount);

    if(module.empty())
    {
        std::cerr << "No module\n";
        return {};
    }

    std::cerr << "Crashed module: " << module << "\n";
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

    QUrl url(QStringLiteral("https://crashreports.kajeka.com/"));
    QNetworkRequest request(url);

    auto doUpload = [&]
    {
        QNetworkAccessManager manager;
        QNetworkReply* reply = manager.post(request, multiPart);

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
                if(reply->error() == QNetworkReply::SslHandshakeFailedError)
                    return QStringLiteral("TLS SslHandshakeFailedError");

                return reply->errorString();
            }
        }
        else
        {
            QObject::disconnect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            reply->abort();
            return QObject::tr("Timeout");
        }

        return QString();
    };

    auto errorString = doUpload();
    if(errorString.startsWith(QStringLiteral("TLS")))
    {
        // https failed, so fallback to http
        request.setUrl(QStringLiteral("http://crashreports.kajeka.com/"));
        errorString = doUpload();
    }

    if(!errorString.isEmpty())
    {
        QMessageBox::warning(nullptr, QApplication::applicationName(),
            QObject::tr("There was an error while uploading the crash report:\n%1")
            .arg(errorString), QMessageBox::Close);
    }

    delete multiPart;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Graphia"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("graphia.app"));
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
