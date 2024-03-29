/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "application.h"
#include "crashtype.h"
#include "../crashhandler.h"
#include "tracking.h"
#include "preferences.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/container_combine.h"
#include "shared/utils/fatalerror.h"
#include "shared/utils/thread.h"
#include "shared/utils/scopetimer.h"
#include "shared/utils/msvcwarningsuppress.h"
#include "shared/utils/static_block.h"

#include "loading/graphmlsaver.h"
#include "loading/jsongraphsaver.h"
#include "loading/gmlsaver.h"
#include "loading/pairwisesaver.h"
#include "loading/nativesaver.h"
#include "loading/nativeloader.h"

#include "rendering/openglfunctions.h"

#include "transform/graphtransformconfigparser.h"
#include "ui/visualisations/visualisationconfigparser.h"

#include <QString>
#include <QStringList>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QQmlEngine>

#include <cmath>
#include <memory>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>

using namespace Qt::Literals::StringLiterals;

const char* const Application::_uri = APP_URI;
QString Application::_appDir = u"."_s;

struct UrlType
{
    QString _name;
    QString _individualDescription;
    QString _collectiveDescription;
    QStringList _extensions;

    bool operator==(const UrlType& other) const
    {
        return _name == other._name &&
               _individualDescription == other._individualDescription &&
               _collectiveDescription == other._collectiveDescription &&
               _extensions == other._extensions;
    }
};

static std::vector<UrlType> urlTypesForPlugins(const std::vector<LoadedPlugin>& plugins)
{
    std::vector<UrlType> fileTypes;

    for(const auto& plugin : plugins)
    {
        const auto urlTypeNames = plugin._interface->loadableUrlTypeNames();
        for(const auto& urlTypeName : urlTypeNames)
        {
            const UrlType fileType =
            {
                urlTypeName,
                plugin._interface->individualDescriptionForUrlTypeName(urlTypeName),
                plugin._interface->collectiveDescriptionForUrlTypeName(urlTypeName),
                plugin._interface->extensionsForUrlTypeName(urlTypeName)
            };

            fileTypes.emplace_back(fileType);
        }
    }

    // Sort by collective description
    std::sort(fileTypes.begin(), fileTypes.end(),
    [](const auto& a, const auto& b)
    {
        return a._collectiveDescription.compare(b._collectiveDescription, Qt::CaseInsensitive) < 0;
    });

    fileTypes.erase(std::unique(fileTypes.begin(), fileTypes.end()), fileTypes.end());

    return fileTypes;
}

Application::Application(QObject *parent, bool enableUpdateCheck) :
    QObject(parent),
    _openGLInfo(OpenGLFunctions::info()),
    _urlTypeDetails(&_loadedPlugins),
    _pluginDetails(&_loadedPlugins)
{
    connect(&_updater, &Updater::noNewUpdateAvailable, this, &Application::noNewUpdateAvailable);
    connect(&_updater, &Updater::updateDownloaded, this, &Application::newUpdateAvailable);
    connect(&_updater, &Updater::progressChanged, this, &Application::updateDownloadProgressChanged);
    connect(&_updater, &Updater::changeLogStored, this, &Application::changeLogStored);

    connect(&_downloadQueue, &DownloadQueue::idleChanged, this, &Application::downloadActiveChanged);
    connect(&_downloadQueue, &DownloadQueue::progressChanged, this, &Application::downloadProgressChanged);
    connect(&_downloadQueue, &DownloadQueue::complete, this, &Application::downloadComplete);
    connect(&_downloadQueue, &DownloadQueue::error, this, &Application::downloadError);

    registerSaverFactory(std::make_unique<NativeSaverFactory>());
    registerSaverFactory(std::make_unique<GraphMLSaverFactory>());
    registerSaverFactory(std::make_unique<GMLSaverFactory>());
    registerSaverFactory(std::make_unique<PairwiseSaverFactory>());
    registerSaverFactory(std::make_unique<JSONGraphSaverFactory>());

    if(enableUpdateCheck)
        _updater.enableAutoBackgroundCheck();

    loadPlugins();
}

Application::~Application() = default;

IPlugin* Application::pluginForName(const QString& pluginName) const
{
    auto pluginIt = std::find_if(_loadedPlugins.begin(), _loadedPlugins.end(),
    [&pluginName](const auto& loadedPlugin)
    {
        return loadedPlugin._interface->name().compare(pluginName) == 0;
    });

    if(pluginIt != _loadedPlugins.end())
        return pluginIt->_interface;

    return nullptr;
}

QString Application::copyright() { return QStringLiteral(COPYRIGHT).replace(u"(c)"_s, u"©"_s); }

#ifdef Q_OS_MACOS
#include <corefoundation/CFBundle.h>
#endif

QStringList Application::resourceDirectories()
{
    QStringList resourceDirs
    {
        Application::_appDir,
        QStandardPaths::writableLocation(
            QStandardPaths::StandardLocation::AppDataLocation) + "/resources"
    };

#ifdef SOURCE_DIR
    // Add the source code directory as resources, to ease debugging
    resourceDirs.append(QStringLiteral(SOURCE_DIR));
#endif

#ifdef Q_OS_MACOS
    CFURLRef resourcesURLRef = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    CFURLRef absoluteResourcesURLRef = CFURLCopyAbsoluteURL(resourcesURLRef);
    CFStringRef pathCFString = CFURLCopyFileSystemPath(absoluteResourcesURLRef, kCFURLPOSIXPathStyle);

    QString path = CFStringGetCStringPtr(pathCFString, CFStringGetSystemEncoding());

    CFRelease(pathCFString);
    CFRelease(absoluteResourcesURLRef);
    CFRelease(resourcesURLRef);

    resourceDirs.append(path);
#elif defined(Q_OS_LINUX)
    QDir usrDir(Application::_appDir);
    usrDir.cdUp();

    resourceDirs.append(usrDir.absolutePath() + "/share/" + name());
#endif

    return resourceDirs;
}

bool Application::canOpen(const QString& urlTypeName) const
{
    if(urlTypeName == NativeFileType)
        return true;

    return std::any_of(_loadedPlugins.begin(), _loadedPlugins.end(),
    [&urlTypeName](const auto& loadedPlugin)
    {
        return loadedPlugin._interface->loadableUrlTypeNames().contains(urlTypeName);
    });
}

bool Application::canOpenAnyOf(const QStringList& urlTypeNames) const
{
    return std::any_of(urlTypeNames.begin(), urlTypeNames.end(),
    [this](const QString& urlTypeName)
    {
        return canOpen(urlTypeName);
    });
}

QStringList Application::urlTypesOf(const QUrl& url) const
{
    if(Loader::canOpen(url))
        return {NativeFileType};

    QStringList urlTypeNames;

    for(const auto& loadedPlugin : _loadedPlugins)
        urlTypeNames.append(loadedPlugin._interface->identifyUrl(url));

    urlTypeNames.removeDuplicates();

    return urlTypeNames;
}

QStringList Application::urlTypesFor(const QString& extension) const
{
    QStringList urlTypeNames;

    for(const auto& loadedPlugin : _loadedPlugins)
    {
        const auto loadableUrlTypeNames = loadedPlugin._interface->loadableUrlTypeNames();
        for(const auto& urlTypeName : loadableUrlTypeNames)
        {
            if(u::contains(loadedPlugin._interface->extensionsForUrlTypeName(urlTypeName), extension))
                urlTypeNames.append(urlTypeName);
        }
    }

    urlTypeNames.removeDuplicates();

    return urlTypeNames;
}

QString Application::descriptionForUrlType(const QString& urlType) const
{
    for(const auto& loadedPlugin : _loadedPlugins)
    {
        if(u::contains(loadedPlugin._interface->loadableUrlTypeNames(), urlType))
            return loadedPlugin._interface->collectiveDescriptionForUrlTypeName(urlType);
    }

    return {};
}

QString Application::urlTypeFor(const QString& description, const QStringList& extensions) const
{
    auto pluginFileTypes = urlTypesForPlugins(_loadedPlugins);

    pluginFileTypes.erase(std::remove_if(pluginFileTypes.begin(), pluginFileTypes.end(),
    [&](const auto& type)
    {
        return type._collectiveDescription != description || type._extensions != extensions;
    }), pluginFileTypes.end());

    if(pluginFileTypes.size() == 1)
        return pluginFileTypes.at(0)._name;

    if(pluginFileTypes.size() > 1)
        qDebug() << "Ambiguous identification of" << description << extensions;

    return {};
}

QStringList Application::failureReasons(const QUrl& url) const
{
    QStringList failureReasons;
    failureReasons.reserve(static_cast<int>(_loadedPlugins.size()));

    for(const auto& loadedPlugin : _loadedPlugins)
    {
        auto failureReason = loadedPlugin._interface->failureReason(url);

        if(!failureReason.isEmpty())
            failureReasons.append(failureReason);
    }

    failureReasons.removeDuplicates();

    return failureReasons;
}

void Application::download(const QUrl& url)
{
    _downloadQueue.add(url);
}

void Application::cancelDownload()
{
    _downloadQueue.cancel();
}

void Application::resumeDownload()
{
    _downloadQueue.resume();
}

bool Application::downloaded(const QUrl& url)
{
    return _downloadQueue.downloaded(url);
}

void Application::registerSaverFactory(std::unique_ptr<ISaverFactory> saver)
{
    _factories.emplace_back(std::move(saver));
}

ISaverFactory* Application::saverFactoryByName(const QString& name)
{
    auto factoryIt = std::find_if(_factories.begin(), _factories.end(),
    [&name](const auto& factory)
    {
        return factory->name() == name;
    });

    if(factoryIt != _factories.end())
        return factoryIt->get();

    return nullptr;
}

QVariantList Application::saverFileTypes()
{
    QVariantList saverData;
    for(auto& saver : _factories)
    {
        QVariantMap map;
        map.insert(u"name"_s, saver->name());
        map.insert(u"extension"_s, saver->extension());
        saverData.push_back(map);
    }
    return saverData;
}

QObject* Application::qmlPluginForName(const QString& pluginName) const
{
    auto* plugin = pluginForName(pluginName);

    if(plugin != nullptr)
        return plugin->ptr();

    return {};
}

QStringList Application::pluginNames(const QString& urlTypeName) const
{
    QStringList viablePluginNames;

    for(const auto& loadedPlugin : _loadedPlugins)
    {
        auto urlTypeNames = loadedPlugin._interface->loadableUrlTypeNames();
        const bool willLoad = std::any_of(urlTypeNames.begin(), urlTypeNames.end(),
        [&urlTypeName](const QString& loadableUrlTypeName)
        {
            return loadableUrlTypeName.compare(urlTypeName) == 0;
        });

        if(willLoad)
            viablePluginNames.append(loadedPlugin._interface->name());
    }

    return viablePluginNames;
}

QString Application::parametersQmlPathForPlugin(const QString& pluginName,
    const QString& urlType) const
{
    auto* plugin = pluginForName(pluginName);

    if(plugin != nullptr)
        return plugin->parametersQmlPath(urlType);

    return {};
}

void Application::checkForUpdates()
{
    if(Updater::updateStatus() != u"installed"_s)
        Updater::resetUpdateStatus();

    _updater.startBackgroundUpdateCheck();
}

void Application::copyImageToClipboard(const QImage& image)
{
    QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
}

QString Application::resourceFile(const QString& relativePath) const
{
    const auto& dirs = resourceDirectories();
    for(const auto& dir : dirs)
    {
        auto resolvedPath = QDir(dir).filePath(relativePath);

        if(QFileInfo::exists(resolvedPath))
            return resolvedPath;
    }

#ifndef _DEBUG
        std::cerr << "Failed to resolve " << relativePath.toStdString() << "\n";
#endif

    return {};
}

bool Application::isResourceFile(const QString& path) const
{
    QString canonicalPath = QFileInfo(path).canonicalPath();

    const auto& dirs = resourceDirectories();

    return std::any_of(dirs.begin(), dirs.end(), [&canonicalPath](const auto& resourceDirectory)
    {
        const QString canonicalResourceDirectory = QFileInfo(resourceDirectory).canonicalPath();
        return canonicalPath.startsWith(canonicalResourceDirectory);
    });
}

bool Application::isResourceFileUrl(const QUrl& url) const
{
    return isResourceFile(url.toLocalFile());
}

bool Application::isNativeLink(const QUrl& url) const
{
    return url.scheme() == nativeExtension();
}

int Application::linkVersionFor(const QUrl& url) const
{
    if(!isNativeLink(url))
        return -1;

    auto host = url.host();

    if(host.isEmpty() || host.at(0) != 'v')
        return -1;

    host = host.mid(1);
    bool success = false;
    auto version = host.toInt(&success);

    if(!success)
        return -1;

    return version;
}

QStringList Application::linkArgumentsFor(const QUrl& url) const
{
    Q_ASSERT(isNativeLink(url));
    if(!isNativeLink(url))
        return {};

    auto path = url.path(QUrl::FullyEncoded);
    auto arguments = path.split('/');

    arguments.erase(std::remove_if(arguments.begin(), arguments.end(), // clazy:exclude=strict-iterators
    [](const auto& argument)
    {
        return argument.isEmpty();
    }), arguments.end()); // clazy:exclude=strict-iterators

    std::transform(arguments.begin(), arguments.end(), arguments.begin(),
    [](const auto& argument)
    {
        return QUrl::fromPercentEncoding(argument.toUtf8());
    });

    return arguments;
}

#if defined(Q_OS_WIN32)
#include <Windows.h>
#endif

static void infiniteLoop()
{
    while(true)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }
}

static void deadlock()
{
    using namespace std::chrono_literals;

    std::mutex a, b;

    std::thread t([&]
    {
        u::setCurrentThreadName(u"DeadlockThread"_s);

        const std::unique_lock<std::mutex> lockA(a);
        std::this_thread::sleep_for(1s);
        const std::unique_lock<std::mutex> lockB(b);
    });

    const std::unique_lock<std::mutex> lockB(b);
    std::this_thread::sleep_for(1s);
    const std::unique_lock<std::mutex> lockA(a);

    t.join();
}

static void hitch()
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(35s);
}

static void silentCrashSubmit()
{
    CrashHandler::instance()->submitMinidump(u"Silent Test Crash Submit"_s);
}

void Application::crash(int crashType)
{
    std::cerr << "Application::crash() invoked!\n";

    auto _crashType = normaliseQmlEnum<CrashType>(crashType);

    switch(_crashType)
    {
    default:
        break;

    case CrashType::NullPtrDereference:
    {
        int* p = nullptr;
        MSVC_WARNING_SUPPRESS_NEXTLINE(6011)
        *p = 0; // NOLINT clang-analyzer-core.NullDereference
        break;
    }

    case CrashType::CppException:
    {
        struct TestException {};
        throw TestException();
    }

    case CrashType::StdException:
    {
        std::vector<int> v;
        auto unused = v.at(10000);
        Q_UNUSED(unused);
        break;
    }

    case CrashType::FatalError:
        FATAL_ERROR(FatalErrorTest);
        break;

    case CrashType::InfiniteLoop:
        infiniteLoop();
        break;

    case CrashType::Deadlock:
        deadlock();
        break;

    case CrashType::Hitch:
        hitch();
        break;

#if defined(Q_OS_WIN32)
    case CrashType::Win32Exception:
    case CrashType::Win32ExceptionNonContinuable:
        RaiseException(EXCEPTION_ILLEGAL_INSTRUCTION,
                       _crashType == CrashType::Win32ExceptionNonContinuable ?
                       EXCEPTION_NONCONTINUABLE : 0, NULL, NULL);
        break;
#endif

    case CrashType::SilentSubmit:
        silentCrashSubmit();
        break;
    }
}

void Application::testConsoleOutput()
{
    std::cout << "std::cout\n";
    std::cerr << "std::cerr\n";

    fprintf(stdout, "stdout\n"); fflush(stdout); // NOLINT cert-err33-c
    fprintf(stderr, "stderr\n"); fflush(stderr); // NOLINT cert-err33-c

    qInfo() << "qInfo()";
    qWarning() << "qWarning()";
    qDebug() << "qDebug()";
    qCritical() << "qCritical()";

#if defined(Q_OS_WIN32)
    OutputDebugString(L"OutputDebugString\n");
#endif
}

void Application::reportScopeTimers()
{
    ScopeTimerManager::instance()->reportToQDebug();
}

void Application::aboutQt() const
{
    QMessageBox::aboutQt(nullptr);
}

void Application::submitTrackingData() const
{
#ifndef _DEBUG
    Tracking::submit();
#endif
}

QString Application::displayTextForTransform(const QString& transform) const
{
    return GraphTransformConfigParser::parseForDisplay(transform);
}

QString Application::displayTextForVisualisation(const QString& visualisation) const
{
    return VisualisationConfigParser::parseForDisplay(visualisation);
}

QString Application::resolvedExe(const QString& exe)
{
    QString fullyQualifiedExe(
        Application::_appDir +
        QDir::separator() + exe);

#ifdef Q_OS_WIN
    fullyQualifiedExe += ".exe";
#endif

#ifndef _DEBUG
    std::cerr << "Resolved executable " << fullyQualifiedExe.toStdString() <<
        " from " << exe.toStdString() << "\n";
#endif

    if(QFileInfo::exists(fullyQualifiedExe))
        return fullyQualifiedExe;

#ifdef Q_OS_MACOS
    // We might be debugging, in which case the exe might be outside the .app
    QDir dotAppDir(Application::_appDir);
    dotAppDir.cdUp();
    dotAppDir.cdUp();
    dotAppDir.cdUp();

    fullyQualifiedExe = dotAppDir.absolutePath() + QDir::separator() + exe;

    if(QFileInfo::exists(fullyQualifiedExe))
        return fullyQualifiedExe;
#endif

    return {};
}

void Application::loadPlugins()
{
    std::vector<QString> pluginsDirs =
    {
        Application::_appDir + "/plugins",
        QStandardPaths::writableLocation(
            QStandardPaths::StandardLocation::AppDataLocation) + "/plugins"
    };

#if defined(Q_OS_MACOS)
    QDir dotAppDir(Application::_appDir);

    // Within the bundle itself
    dotAppDir.cdUp();
    pluginsDirs.emplace_back(dotAppDir.absolutePath() + "/PlugIns");

    // Adjacent to the .app file
    dotAppDir.cdUp();
    dotAppDir.cdUp();
    pluginsDirs.emplace_back(dotAppDir.absolutePath() + "/plugins");
#elif defined(Q_OS_LINUX)
    // Add the LSB location for the plugins
    QDir usrDir(Application::_appDir);
    usrDir.cdUp();

    pluginsDirs.emplace_back(usrDir.absolutePath() + "/lib/" + name() + "/plugins");
#endif

    for(const auto& pluginsDir : pluginsDirs)
    {
        if(pluginsDir.isEmpty() || !QDir(pluginsDir).exists())
            continue;

        std::cerr << "Loading dynamic plugins from " << pluginsDir.toStdString() << "...\n";

        const QDir pluginsQDir(pluginsDir);
        const auto fileNames = pluginsQDir.entryList(QDir::Files);

        if(fileNames.empty())
        {
            std::cerr << "  ...none found\n";
            continue;
        }

        for(const auto& fileName : fileNames)
        {
            if(!QLibrary::isLibrary(fileName))
            {
                std::cerr << "  ..." << QFileInfo(fileName).fileName().toStdString() <<
                    " is not a library, skipping\n";
                continue;
            }

            auto pluginLoader = std::make_unique<QPluginLoader>(pluginsQDir.absoluteFilePath(fileName));
            QObject* plugin = pluginLoader->instance();
            if(!pluginLoader->isLoaded())
            {
                std::cerr << "  ..." << QFileInfo(fileName).fileName().toStdString() <<
                    " failed to load: " << pluginLoader->errorString().toStdString() << "\n";

                QMessageBox::warning(nullptr, QObject::tr("Plugin Load Failed"),
                    QObject::tr("The plugin \"%1\" failed to load. The reported error is:\n%2")
                    .arg(fileName, pluginLoader->errorString()), QMessageBox::Ok);

                continue;
            }

            auto* iplugin = dynamic_cast<IPlugin*>(plugin);
            initialisePlugin(iplugin, std::move(pluginLoader));
        }
    }

    int numLoadedStaticPlugins = 0;
    std::cerr << "Loading static plugins...\n";

    for(auto* staticPluginInstance : QPluginLoader::staticInstances()) // clazy:exclude=range-loop-detach
    {
        auto* iplugin = dynamic_cast<IPlugin*>(staticPluginInstance);
        if(initialisePlugin(iplugin, nullptr))
            numLoadedStaticPlugins++;
    }

    if(numLoadedStaticPlugins == 0)
        std::cerr << "  ...none found\n";

    // Force event processing here so that we initialise any qmlenum.h based enums
    // that were created in plugins
    QCoreApplication::processEvents();

    updateLoadingCapabilities();
}

bool Application::initialisePlugin(IPlugin* plugin, std::unique_ptr<QPluginLoader> pluginLoader)
{
    if(plugin == nullptr)
        return false;

    const auto pluginName = plugin->name();
    const QString fileName = pluginLoader != nullptr ?
        QFileInfo(pluginLoader->fileName()).fileName() :
        QString(u"Static"_s);

    const bool pluginNameAlreadyUsed = std::any_of(_loadedPlugins.begin(), _loadedPlugins.end(),
    [&pluginName](const auto& loadedPlugin)
    {
        return loadedPlugin._interface->name().compare(pluginName, Qt::CaseInsensitive) == 0;
    });

    if(pluginNameAlreadyUsed)
    {
        auto message = QObject::tr("  ...%1 (%2) is already loaded from a different location\n")
            .arg(pluginName, fileName);

        std::cerr << message.toStdString();

        if(pluginLoader != nullptr)
            pluginLoader->unload();

        return false;
    }

    plugin->initialise(this);
    _loadedPlugins.emplace_back(plugin, std::move(pluginLoader));
    _urlTypeDetails.update();
    _pluginDetails.update();

    auto message = QObject::tr("  ...%1 (%2)\n")
        .arg(pluginName, fileName);

    std::cerr << message.toStdString();

    // Plugin might have static_blocks
    execute_static_blocks();

    return true;
}

void Application::updateLoadingCapabilities()
{
    // Initialise with native file type
    std::vector<UrlType> nativeFileTypes{{NativeFileType, QString("%1 File").arg(name()),
        QString("%1 Files").arg(name()), {nativeExtension()}}};

    auto pluginFileTypes = urlTypesForPlugins(_loadedPlugins);
    auto fileTypes = u::combine(nativeFileTypes, pluginFileTypes);

    QString description = QObject::tr("All Files (");
    bool second = false;

    for(const auto& fileType : fileTypes)
    {
        for(const auto& extension : fileType._extensions)
        {
            if(second)
                description += u" "_s;
            else
                second = true;

            description += "*." + extension;
        }
    }

    description += u")"_s;

    _nameFilters.clear();
    _nameFilters.append(description);

    for(const auto& fileType : fileTypes)
    {
        description = fileType._collectiveDescription + " (";
        second = false;

        for(const auto& extension : fileType._extensions)
        {
            if(second)
                description += u" "_s;
            else
                second = true;

            description += "*." + extension;
        }

        description += u")"_s;

        _nameFilters.append(description);
    }

    emit nameFiltersChanged();

    QStringList loadableExtensions;
    for(const auto& fileType : fileTypes)
    {
        for(const auto& extension : fileType._extensions)
            loadableExtensions.append(extension);
    }

    u::removeDuplicates(loadableExtensions);
    _loadableExtensions.setStringList(loadableExtensions);

    emit loadableExtensionsChanged();
}

void Application::unloadPlugins()
{
    for(auto& loadedPlugin : _loadedPlugins)
    {
        if(loadedPlugin._loader != nullptr)
            loadedPlugin._loader->unload();
        else
            delete loadedPlugin._interface;

        loadedPlugin._interface = nullptr;
    }

    _loadedPlugins.clear();
}

QAbstractListModel* Application::urlTypeDetails()
{
    return &_urlTypeDetails;
}

QAbstractListModel* Application::pluginDetails()
{
    return &_pluginDetails;
}

int UrlTypeDetailsModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(urlTypesForPlugins(*_loadedPlugins).size());
}

QVariant UrlTypeDetailsModel::data(const QModelIndex& index, int role) const
{
    auto urlTypes = urlTypesForPlugins(*_loadedPlugins);

    const int row = index.row();

    if(row < 0 || row >= static_cast<int>(urlTypes.size()))
        return {};

    auto& urlType = urlTypes.at(static_cast<size_t>(row));

    switch(role)
    {
    case Name:                  return urlType._name;
    case IndividualDescription: return urlType._individualDescription;
    case Qt::DisplayRole:
    case CollectiveDescription: return urlType._collectiveDescription;
    default: break;
    }

    return {};
}

QHash<int, QByteArray> UrlTypeDetailsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[IndividualDescription] = "individualDescription";
    roles[CollectiveDescription] = "collectiveDescription";
    return roles;
}

int PluginDetailsModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_loadedPlugins->size());
}

QVariant PluginDetailsModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();

    if(row < 0 || row >= rowCount(index))
        return {};

    auto* plugin = _loadedPlugins->at(static_cast<size_t>(row))._interface;

    switch(role)
    {
    case Qt::DisplayRole:
    case Name:
        return plugin->name();

    case Description:
    {
        QString urlTypes;
        for(auto& loadbleUrlTypeName : plugin->loadableUrlTypeNames())
        {
            if(!urlTypes.isEmpty())
                urlTypes += tr(", ");

            urlTypes += plugin->collectiveDescriptionForUrlTypeName(loadbleUrlTypeName);
        }

        if(urlTypes.isEmpty())
            urlTypes = tr("None");

        return QString(tr("%1\n\nSupported data types: %2"))
                .arg(plugin->description(), urlTypes);
    }

    case ImageSource:
        return plugin->imageSource();

    default:
        break;
    }

    return {};
}

QHash<int, QByteArray> PluginDetailsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[Description] = "description";
    roles[ImageSource] = "imageSource";
    return roles;
}

static_block
{
    qmlRegisterType<Application>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "Application");
}
