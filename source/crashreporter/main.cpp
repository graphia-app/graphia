/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "build_defines.h"

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
#include <QQuickStyle>

#include <iostream>
#include <map>
#include <string>
#include <regex>
#include <chrono>

#include "report.h"
#include "app/rendering/openglfunctions.h"
#include "shared/utils/preferences.h"

#include <google_breakpad/processor/minidump.h>
#include <google_breakpad/processor/process_state.h>
#include <google_breakpad/processor/minidump_processor.h>
#include <google_breakpad/processor/call_stack.h>
#include <google_breakpad/processor/stack_frame.h>
#include <processor/pathname_stripper.h>

using namespace std::chrono_literals;

// clazy:excludeall=lambda-in-connect

static std::string crashedModule(const QString& dmpFile)
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

    const int requestingThread = process_state.requesting_thread();
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

    // Don't count standard libraries etc. when finding the crashed
    // module; often a runtime failure will occur in abort/terminate
    // or similar; this isn't useful information
    std::vector<std::string> skipModules =
    {
        R"(^libc(-[\d\.]+)?.so([\d\.]+)?$)",
        R"(^libstdc\+\+\.so[\d\.]+$)",
        R"(^libsystem_[^\.]+\.dylib$)",
        R"(^libc\+\+abi\.dylib$)",
        R"(^libobjc\.[^\.]+\.dylib$)",
    };

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

        std::smatch match;
        const bool skip = std::any_of(skipModules.begin(), skipModules.end(),
        [&module, &match](const auto& skipModuleRe)
        {
            return std::regex_match(module, match, std::regex(skipModuleRe));
        });

        if(skip)
            module.clear();
    }
    while(module.empty() && frameIndex < frameCount);

    if(module.empty())
    {
        std::cerr << "No module\n";
        return {};
    }

    std::cerr << "Crashed module: " << module << "\n";
    return module;
}

static void uploadReport(const QString& email, const QString& text,
    bool inVideoDriver, const QString& module,
    const QString& dmpFile, const QString& attachmentDir)
{
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    std::map<const char*, QString> const fields =
    {
        {"email",           email},
        {"text",            text},
        {"inVideoDriver",   QVariant(inVideoDriver).toString()},
        {"product",         PRODUCT_NAME},
        {"version",         VERSION},
        {"crashedModule",   module},
        {"os",              QString("%1 %2 %3 %4").arg(QSysInfo::kernelType(),
                                                       QSysInfo::kernelVersion(),
                                                       QSysInfo::productType(),
                                                       QSysInfo::productVersion())},
        {"gl",              OpenGLFunctions::info()},
#ifdef GIT_BRANCH
        {"git_branch",      GIT_BRANCH},
#endif
    };

    for(const auto& field : fields)
    {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QStringLiteral(R"(form-data; name="%1")").arg(field.first)));
        part.setBody(field.second.toLatin1());
        multiPart->append(part);
    }

    QHttpPart dmpPart;
    dmpPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    dmpPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant(QStringLiteral(R"(form-data; name="upload_file_minidump"; filename="%1")")
                               .arg(QFileInfo(dmpFile).fileName())));
    auto* file = new QFile(dmpFile);
    file->open(QIODevice::ReadOnly);
    dmpPart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(dmpPart);

    if(!attachmentDir.isEmpty())
    {
        const QString fingerPrint = QFileInfo(dmpFile).baseName();

        QDirIterator dirIterator(attachmentDir);
        while(dirIterator.hasNext())
        {
            const QString fileName = dirIterator.next();
            const QFileInfo fileInfo(fileName);

            // Skip . and ..
            if(!fileInfo.exists() || !fileInfo.isFile())
                continue;

            QHttpPart attachmentPart;
            attachmentPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
            attachmentPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                     QVariant(QStringLiteral(R"(form-data; name="%1_%2"; filename="%2")")
                                        .arg(fingerPrint, fileInfo.fileName())));
            auto* attachment = new QFile(fileName);
            attachment->open(QIODevice::ReadOnly);
            attachmentPart.setBodyDevice(attachment);
            attachment->setParent(multiPart);
            multiPart->append(attachmentPart);
        }
    }

    auto queryUrl = QUrl(u::getPref(QStringLiteral("servers/crashreports")).toString());

    auto doUpload = [&]
    {
        std::vector<QMetaObject::Connection> connections;
        QNetworkAccessManager manager;
        QEventLoop loop;

        // Handle timeouts
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(1min);

        QNetworkReply* postReply = nullptr;

        // Crash submission occurs in two stages; firstly we ask for a submission URL...
        QNetworkReply* reply = manager.get(QNetworkRequest(queryUrl));
        connections.emplace_back(QObject::connect(reply, &QNetworkReply::finished, [&]
        {
            if(reply->error() == QNetworkReply::NetworkError::NoError)
            {
                const QString urlString = reply->readAll();
                auto submissionUrl = QUrl(urlString.trimmed());

                // ...and if we get one back, we use it to actually submit the crash report
                postReply = manager.post(QNetworkRequest(submissionUrl), multiPart);
                connections.emplace_back(QObject::connect(postReply, &QNetworkReply::finished,
                    &loop, &QEventLoop::quit));
            }
        }));
        // (this is so we can change the submission URL for deployed builds)

        loop.exec();

        // The event loop has exited, perhaps due to timeout; in this case
        // we don't want any pending replies so we disconnect them
        for(auto& connection : connections)
            QObject::disconnect(connection);

        // Timer expired
        if(!timer.isActive())
        {
            reply->abort();

            if(postReply != nullptr)
                postReply->abort();

            return QObject::tr("Timeout. Please check your internet connection.");
        }

        timer.stop();

        if(postReply == nullptr)
            return QObject::tr("Could not retrieve submission URL from \"%1\"").arg(queryUrl.toString());

        if(postReply->error() != QNetworkReply::NetworkError::NoError)
            return QStringLiteral("%1 (%2)").arg(postReply->errorString()).arg(postReply->error());

        return QString();
    };

    bool retry = false;

    do
    {
        if(!queryUrl.isValid())
        {
            QMessageBox::warning(nullptr, QApplication::applicationName(),
                QObject::tr("The query URL is invalid:\n\n\"%1\"")
                .arg(queryUrl.toString()), QMessageBox::Close);

            break;
        }

        auto errorString = doUpload();
        if(!errorString.isEmpty())
        {
            auto clickedButton = QMessageBox::warning(nullptr, QApplication::applicationName(),
                QObject::tr("There was an error while uploading the crash report:\n\n%1")
                .arg(errorString), QMessageBox::Retry | QMessageBox::Close);
            retry = (clickedButton == QMessageBox::Retry);
        }
    } while(retry);

    delete multiPart;
}

int main(int argc, char *argv[])
{
    const QApplication app(argc, argv);

    Q_INIT_RESOURCE(shared);

    QCoreApplication::setOrganizationName(QStringLiteral("Graphia"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("graphia.app"));
    QCoreApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

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

    QIcon mainIcon;
    mainIcon.addFile(QStringLiteral(":/icon.svg"));
    QApplication::setWindowIcon(mainIcon);

    auto module = crashedModule(positional.at(0));
    const auto* videoDriverRegex = R"(^(nvoglv|ig.+icd|ati[og]|libGPUSupport|AppleIntel|AMDRadeon|iris_dri).*)";

    std::smatch match;
    const bool inVideoDriver = std::regex_match(module, match, std::regex(videoDriverRegex));
    auto emailAddress = u::getPref(QStringLiteral("tracking/emailAddress")).toString();

    if(!p.isSet(QStringLiteral("submit")))
    {
        QQuickStyle::setStyle(u::getPref(QStringLiteral("system/uiTheme")).toString());

        QQmlApplicationEngine engine;

        engine.rootContext()->setContextProperty(QStringLiteral("report"), &report);
        engine.rootContext()->setContextProperty(QStringLiteral("glVendor"), OpenGLFunctions::vendor());
        engine.rootContext()->setContextProperty(QStringLiteral("inVideoDriver"), inVideoDriver);
        engine.rootContext()->setContextProperty(QStringLiteral("emailAddress"), emailAddress);

        engine.addImportPath(QStringLiteral("qrc:///qml/"));
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        Q_ASSERT(!engine.rootObjects().empty());

        exitCode = QCoreApplication::exec();
    }
    else
    {
        report._email = emailAddress;
        report._text = p.value(QStringLiteral("description"));
    }

    if(exitCode != 127)
    {
        uploadReport(report._email, report._text, inVideoDriver,
            QString::fromStdString(module), positional.at(0), attachmentsDir);
    }

    return exitCode;
}
