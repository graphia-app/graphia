/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef _DEBUG
#include "app/tracking.h"
#endif

#include "app/preferences.h"
#include "app/preferences_legacy.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/container_combine.h"
#include "shared/utils/container.h"
#include "shared/utils/fatalerror.h"
#include "shared/utils/thread.h"
#include "shared/utils/scopetimer.h"
#include "shared/utils/msvcwarningsuppress.h"
#include "shared/utils/static_block.h"

#include "app/loading/graphmlsaver.h"
#include "app/loading/jsongraphsaver.h"
#include "app/loading/gmlsaver.h"
#include "app/loading/pairwisesaver.h"
#include "app/loading/nativesaver.h"
#include "app/loading/nativeloader.h"

#include "app/attributes/attribute.h"

#include "app/graph/graphmodel.h"

#include "app/rendering/openglfunctions.h"

#include "app/transform/graphtransformconfigparser.h"
#include "app/transform/transforms/filtertransform.h"
#include "app/transform/transforms/edgecontractiontransform.h"
#include "app/transform/transforms/mcltransform.h"
#include "app/transform/transforms/louvaintransform.h"
#include "app/transform/transforms/pageranktransform.h"
#include "app/transform/transforms/eccentricitytransform.h"
#include "app/transform/transforms/betweennesstransform.h"
#include "app/transform/transforms/contractbyattributetransform.h"
#include "app/transform/transforms/separatebyattributetransform.h"
#include "app/transform/transforms/knntransform.h"
#include "app/transform/transforms/percentnntransform.h"
#include "app/transform/transforms/edgereductiontransform.h"
#include "app/transform/transforms/forwardmultielementattributetransform.h"
#include "app/transform/transforms/spanningtreetransform.h"
#include "app/transform/transforms/attributesynthesistransform.h"
#include "app/transform/transforms/combineattributestransform.h"
#include "app/transform/transforms/conditionalattributetransform.h"
#include "app/transform/transforms/averageattributetransform.h"
#include "app/transform/transforms/typecasttransform.h"
#include "app/transform/transforms/removeleavestransform.h"

#include "app/ui/visualisations/visualisationconfigparser.h"
#include "app/ui/visualisations/colorvisualisationchannel.h"
#include "app/ui/visualisations/sizevisualisationchannel.h"
#include "app/ui/visualisations/sharedtextvisualisationchannel.h"
#include "app/ui/visualisations/textvisualisationchannel.h"
#include "app/ui/visualisations/textcolorvisualisationchannel.h"
#include "app/ui/visualisations/textsizevisualisationchannel.h"

#include "../crashhandler.h"

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include <QVariantMap>
#include <QApplication>
#include <QClipboard>

#include <iostream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <map>
#include <cstddef>

#ifdef Q_OS_WASM
#include "shared/utils/qrcextract.h"
#endif

using namespace Qt::Literals::StringLiterals;

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

static std::vector<UrlType> urlTypesForPlugins(const std::vector<LoadedPlugin>& plugins, const QStringList& filter = {})
{
    std::vector<UrlType> fileTypes;

    for(const auto& plugin : plugins)
    {
        const auto urlTypeNames = plugin._interface->loadableUrlTypeNames();
        for(const auto& urlTypeName : urlTypeNames)
        {
            if(!filter.empty() && !filter.contains(urlTypeName))
                continue;

            const UrlType fileType =
            {
                ._name = urlTypeName,
                ._individualDescription = plugin._interface->individualDescriptionForUrlTypeName(urlTypeName),
                ._collectiveDescription = plugin._interface->collectiveDescriptionForUrlTypeName(urlTypeName),
                ._extensions = plugin._interface->extensionsForUrlTypeName(urlTypeName)
            };

            fileTypes.emplace_back(fileType);
        }
    }

    // Sort by collective description
    std::ranges::sort(fileTypes,
    [](const auto& a, const auto& b)
    {
        return a._collectiveDescription.compare(b._collectiveDescription, Qt::CaseInsensitive) < 0;
    });

    fileTypes.erase(std::ranges::unique(fileTypes).begin(), fileTypes.end());

    return fileTypes;
}

Application::Application(QObject *parent, bool enableUpdateCheck) :
    QObject(parent),
    _openGLInfo(OpenGLFunctions::info())
{
#ifndef Q_OS_WASM
    connect(&_updater, &Updater::noNewUpdateAvailable, this, &Application::noNewUpdateAvailable);
    connect(&_updater, &Updater::updateDownloaded, this, &Application::newUpdateAvailable);
    connect(&_updater, &Updater::progressChanged, this, &Application::updateDownloadProgressChanged);
    connect(&_updater, &Updater::changeLogStored, this, &Application::changeLogStored);

    if(enableUpdateCheck)
        _updater.enableAutoBackgroundCheck();
#else
    Q_UNUSED(enableUpdateCheck);
#endif

    connect(&_downloadQueue, &DownloadQueue::idleChanged, this, &Application::downloadActiveChanged);
    connect(&_downloadQueue, &DownloadQueue::progressChanged, this, &Application::downloadProgressChanged);
    connect(&_downloadQueue, &DownloadQueue::complete, this, &Application::downloadComplete);
    connect(&_downloadQueue, &DownloadQueue::error, this, &Application::downloadError);

    registerSaverFactory(std::make_unique<NativeSaverFactory>());
    registerSaverFactory(std::make_unique<GraphMLSaverFactory>());
    registerSaverFactory(std::make_unique<GMLSaverFactory>());
    registerSaverFactory(std::make_unique<PairwiseSaverFactory>());
    registerSaverFactory(std::make_unique<JSONGraphSaverFactory>());

    auto addTransformFactory = [this](std::unique_ptr<GraphTransformFactory> factory)
    {
        auto transformName = factory->name();
        _graphTransformFactories.emplace(transformName, std::move(factory));
    };

    addTransformFactory(std::make_unique<FilterTransformFactory>(ElementType::Node, false));
    addTransformFactory(std::make_unique<FilterTransformFactory>(ElementType::Edge, false));
    addTransformFactory(std::make_unique<FilterTransformFactory>(ElementType::Component, false));
    addTransformFactory(std::make_unique<FilterTransformFactory>(ElementType::Node, true));
    addTransformFactory(std::make_unique<FilterTransformFactory>(ElementType::Edge, true));
    addTransformFactory(std::make_unique<FilterTransformFactory>(ElementType::Component, true));
    addTransformFactory(std::make_unique<EdgeContractionTransformFactory>());
    addTransformFactory(std::make_unique<MCLTransformFactory>());
    addTransformFactory(std::make_unique<LouvainTransformFactory>());
    addTransformFactory(std::make_unique<WeightedLouvainTransformFactory>());
    addTransformFactory(std::make_unique<PageRankTransformFactory>());
    addTransformFactory(std::make_unique<EccentricityTransformFactory>());
    addTransformFactory(std::make_unique<BetweennessTransformFactory>());
    addTransformFactory(std::make_unique<ContractByAttributeTransformFactory>());
    addTransformFactory(std::make_unique<SeparateByAttributeTransformFactory>());
    addTransformFactory(std::make_unique<ConditionalAttributeTransformFactory>(ElementType::Node));
    addTransformFactory(std::make_unique<ConditionalAttributeTransformFactory>(ElementType::Edge));
    addTransformFactory(std::make_unique<KNNTransformFactory>());
    addTransformFactory(std::make_unique<PercentNNTransformFactory>());
    addTransformFactory(std::make_unique<EdgeReductionTransformFactory>());
    addTransformFactory(std::make_unique<SpanningTreeTransformFactory>());
    addTransformFactory(std::make_unique<AttributeSynthesisTransformFactory>());
    addTransformFactory(std::make_unique<CombineAttributesTransformFactory>());
    addTransformFactory(std::make_unique<ForwardMultiElementAttributeTransformFactory>());
    addTransformFactory(std::make_unique<AverageAttributeTransformFactory>());
    addTransformFactory(std::make_unique<TypeCastTransformFactory>());
    addTransformFactory(std::make_unique<RemoveLeavesTransformFactory>());
    addTransformFactory(std::make_unique<RemoveBranchesTransformFactory>());

    auto addVisualisationChannelFactory = [this](std::unique_ptr<VisualisationChannelFactory> factory)
    {
        auto channelName = factory->name();
        _visualisationChannelFactories.emplace(channelName, std::move(factory));
    };

    addVisualisationChannelFactory(std::make_unique<ColorVisualisationChannelFactory>());
    addVisualisationChannelFactory(std::make_unique<SizeVisualisationChannelFactory>());
    addVisualisationChannelFactory(std::make_unique<SharedTextVisualisationChannelFactory>());
    addVisualisationChannelFactory(std::make_unique<TextVisualisationChannelFactory>());
    addVisualisationChannelFactory(std::make_unique<TextColorVisualisationChannelFactory>());
    addVisualisationChannelFactory(std::make_unique<TextSizeVisualisationChannelFactory>());

#ifdef Q_OS_WASM
    u::qrcExtract(":/wasm_resource_files", QStandardPaths::writableLocation(
        QStandardPaths::StandardLocation::AppDataLocation) + "/resources");
#endif

    loadPlugins();
    collectAttributeDeclarations();
    upgradeLegacyPreferences(this);
}

Application::~Application() = default;

IPlugin* Application::pluginForName(const QString& pluginName) const
{
    auto pluginIt = std::ranges::find_if(_loadedPlugins,
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
            QStandardPaths::StandardLocation::AppDataLocation) + u"/resources"_s
    };

#ifdef APP_SOURCE_DIR
    // Add the source code directory as resources, to ease debugging
    resourceDirs.append(QStringLiteral(APP_SOURCE_DIR));
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

    resourceDirs.append(usrDir.absolutePath() + u"/share/"_s + name());
#endif

    return resourceDirs;
}

bool Application::canOpen(const QString& urlTypeName) const
{
    if(urlTypeName == NativeFileType)
        return true;

    return std::ranges::any_of(_loadedPlugins,
    [&urlTypeName](const auto& loadedPlugin)
    {
        return loadedPlugin._interface->loadableUrlTypeNames().contains(urlTypeName);
    });
}

bool Application::canOpenAnyOf(const QStringList& urlTypeNames) const
{
    return std::ranges::any_of(urlTypeNames,
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

    pluginFileTypes.erase(std::ranges::remove_if(pluginFileTypes,
    [&](const auto& type)
    {
        return type._collectiveDescription != description || type._extensions != extensions;
    }).begin(), pluginFileTypes.end());

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
    auto factoryIt = std::ranges::find_if(_factories,
    [&name](const auto& factory)
    {
        return factory->name() == name;
    });

    if(factoryIt != _factories.end())
        return factoryIt->get();

    return nullptr;
}

const GraphTransformFactory* Application::transformFactory(const QString& transformName) const
{
    if(!u::contains(_graphTransformFactories, transformName))
        return nullptr;

    return _graphTransformFactories.at(transformName).get();
}

const VisualisationChannelFactory* Application::visualisationChannelFactory(const QString& channelName) const
{
    if(!u::contains(_visualisationChannelFactories, channelName))
        return nullptr;

    return _visualisationChannelFactories.at(channelName).get();
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
        const bool willLoad = std::ranges::any_of(urlTypeNames,
        [&urlTypeName](const QString& loadableUrlTypeName)
        {
            return loadableUrlTypeName.compare(urlTypeName) == 0;
        });

        if(willLoad)
            viablePluginNames.append(loadedPlugin._interface->name());
    }

    return viablePluginNames;
}

QString Application::pluginDisplayName(const QString& pluginName) const
{
    const auto* plugin = pluginForName(pluginName);
    return plugin != nullptr ? plugin->displayName() : pluginName;
}

QString Application::qmlModuleForPlugin(const QString& pluginName) const
{
    auto* plugin = pluginForName(pluginName);

    if(plugin != nullptr)
        return plugin->qmlModule();

    return {};
}

QString Application::parametersQmlTypeForPlugin(const QString& pluginName,
    const QString& urlType) const
{
    auto* plugin = pluginForName(pluginName);

    if(plugin != nullptr)
        return plugin->parametersQmlType(urlType);

    return {};
}

UrlTypeDetailsModel* Application::urlTypeDetailsModel() const
{
    return new UrlTypeDetailsModel(&_loadedPlugins);
}

PluginDetailsModel* Application::pluginDetailsModel() const
{
    return new PluginDetailsModel(&_loadedPlugins);
}

void Application::checkForUpdates()
{
#ifndef Q_OS_WASM
    if(Updater::updateStatus() != u"installed"_s)
        Updater::resetUpdateStatus();

    _updater.startBackgroundUpdateCheck();
#endif
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
    const QString canonicalPath = QFileInfo(path).canonicalFilePath();

    if(canonicalPath.isEmpty())
        return false;

    const auto& dirs = resourceDirectories();

    // canonicalFilePath is empty if the directory doesn't exist, and is the directory
    // itself, where canonicalPath would be the directory that contains it
    return std::ranges::any_of(dirs, [&canonicalPath](const auto& resourceDirectory)
    {
        const QString canonicalResourceDirectory = QFileInfo(resourceDirectory).canonicalFilePath();
        return !canonicalResourceDirectory.isEmpty() &&
            canonicalPath.startsWith(canonicalResourceDirectory + u'/');
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

    arguments.erase(std::ranges::remove_if(arguments, // clazy:exclude=strict-iterators
    [](const auto& argument)
    {
        return argument.isEmpty();
    }).begin(), arguments.end()); // clazy:exclude=strict-iterators

    std::ranges::transform(arguments, arguments.begin(),
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

const AttributeDeclaration* Application::attributeDeclaration(const QString& name, const IPlugin* plugin) const
{
    auto find = [&name](const AttributeDeclarations& declarations) -> const AttributeDeclaration*
    {
        auto it = std::ranges::find_if(declarations,
            [&name](const auto& declaredAttribute) { return declaredAttribute._name == name; });

        return it != declarations.end() ? &*it : nullptr;
    };

    if(const auto* declaredAttribute = find(_declaredAttributes))
        return declaredAttribute;

    if(u::contains(_pluginDeclaredAttributes, plugin))
        return find(_pluginDeclaredAttributes.at(plugin));

    return nullptr;
}

bool Application::attributeIsDeclared(const QString& name) const
{
    return attributeDeclaration(name, nullptr) != nullptr || std::ranges::any_of(_loadedPlugins,
        [this, &name](const auto& loadedPlugin) { return attributeDeclaration(name, loadedPlugin._interface) != nullptr; });
}

QString Application::attributeDisplayName(const QString& name, const IPlugin* plugin) const
{
    return Attribute::fullyQualifiedDisplayName(name, [this, plugin](const QString& attributeName)
    {
        const auto* declaredAttribute = attributeDeclaration(attributeName, plugin);
        return declaredAttribute != nullptr && !declaredAttribute->_displayName.isEmpty() ?
            declaredAttribute->_displayName : attributeName;
    });
}

QString Application::displayTextForTransform(const QString& transform, const IPlugin* plugin) const
{
    return GraphTransformConfigParser::parseForDisplay(transform, _graphTransformFactories,
        [this, plugin](const QString& attributeName) { return attributeDisplayName(attributeName, plugin); });
}

QString Application::displayTextForVisualisation(const QString& visualisation, const IPlugin* plugin) const
{
    return VisualisationConfigParser::parseForDisplay(visualisation, _visualisationChannelFactories,
        [this, plugin](const QString& attributeName) { return attributeDisplayName(attributeName, plugin); });
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
        Application::_appDir + u"/plugins"_s,
        QStandardPaths::writableLocation(
            QStandardPaths::StandardLocation::AppDataLocation) + u"/plugins"_s
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

    pluginsDirs.emplace_back(usrDir.absolutePath() + u"/lib/"_s + name() + u"/plugins"_s);
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

    auto* qObjectPlugin = dynamic_cast<QObject*>(plugin);
    if(qObjectPlugin == nullptr)
        return false;

    qObjectPlugin->setParent(this);

    const auto pluginName = plugin->name();
    const QString fileName = pluginLoader != nullptr ?
        QFileInfo(pluginLoader->fileName()).fileName() :
        QString(u"Static"_s);

    const bool pluginNameAlreadyUsed = std::ranges::any_of(_loadedPlugins,
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
    std::vector<UrlType> nativeFileTypes
    {
        {
            ._name = NativeFileType,
            ._individualDescription = QObject::tr("%1 File").arg(name()),
            ._collectiveDescription = QObject::tr("%1 Files").arg(name()),
            ._extensions = {nativeExtension()}
        }
    };

    auto pluginFileTypes = urlTypesForPlugins(_loadedPlugins);
    auto fileTypes = u::combine(nativeFileTypes, pluginFileTypes);

    QString extensions;
    bool second = false;

    for(const auto& fileType : fileTypes)
    {
        for(const auto& extension : fileType._extensions)
        {
            if(second)
                extensions += u" "_s;
            else
                second = true;

            extensions += u"*."_s + extension;
        }
    }

    // The extensions must stay in brackets, as that's how file dialogs find them
    QString description = QObject::tr("All Files (%1)").arg(extensions);

    _nameFilters.clear();
    _nameFilters.append(description);

    for(const auto& fileType : fileTypes)
    {
        description = fileType._collectiveDescription + u" ("_s;
        second = false;

        for(const auto& extension : fileType._extensions)
        {
            if(second)
                description += u" "_s;
            else
                second = true;

            description += u"*."_s + extension;
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

    std::map<QString, int> extensionUrlTypeCounts;
    for(const auto& fileType : fileTypes)
    {
        for(const auto& extension : fileType._extensions)
            extensionUrlTypeCounts[extension]++;
    }

    QStringList ambiguousExtensions;
    for(const auto& [extension, count] : extensionUrlTypeCounts)
    {
        if(count > 1)
            ambiguousExtensions.append(extension);
    }

    _ambiguousExtensions.setStringList(ambiguousExtensions);
    emit ambiguousExtensionsChanged();
    emit hasAmbiguousExtensionsChanged();

    std::map<QString, int> urlTypePluginCounts;
    for(const auto& loadedPlugin : _loadedPlugins)
    {
        auto urlTypeNames = loadedPlugin._interface->loadableUrlTypeNames();

        for(const auto& urlTypeName : std::as_const(urlTypeNames))
            urlTypePluginCounts[urlTypeName]++;
    }

    QStringList ambiguousUrlTypes;
    for(const auto& [urlTypeName, count] : urlTypePluginCounts)
    {
        if(count > 1)
            ambiguousUrlTypes.append(urlTypeName);
    }

    _ambiguousUrlTypes.setStringList(ambiguousUrlTypes);
    emit ambiguousUrlTypesChanged();
    emit hasAmbiguousUrlTypesChanged();
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

void Application::collectAttributeDeclarations()
{
    u::append(_declaredAttributes, GraphModel::attributeDeclarations());

    for(const auto& [transformName, factory] : _graphTransformFactories)
        u::append(_declaredAttributes, factory->attributeDeclarations());

    for(const auto& [plugin, _] : _loadedPlugins)
        _pluginDeclaredAttributes[plugin] = plugin->attributeDeclarations();
}

int Application::updateDownloadProgress() const
{
#ifdef Q_OS_WASM
    return 0;
#else
    return _updater.progress();
#endif
}

int UrlTypeDetailsModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(urlTypesForPlugins(*_loadedPlugins, _filter).size());
}

QVariant UrlTypeDetailsModel::data(const QModelIndex& index, int role) const
{
    auto urlTypes = urlTypesForPlugins(*_loadedPlugins, _filter);

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
    return static_cast<int>(filteredPlugins().size());
}

QVariant PluginDetailsModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();

    if(row < 0 || row >= rowCount(index))
        return {};

    const auto* plugin = filteredPlugins().at(static_cast<size_t>(row));

    switch(role)
    {
    case Qt::DisplayRole:
    case DisplayName:
        return plugin->displayName();

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
    roles[DisplayName] = "displayName";
    roles[Description] = "description";
    roles[ImageSource] = "imageSource";
    return roles;
}

std::vector<const IPlugin*> PluginDetailsModel::filteredPlugins() const
{
    std::vector<const IPlugin*> plugins;

    for(const auto& plugin : *_loadedPlugins)
    {
        if(!_filter.empty() && !_filter.contains(plugin._interface->name()))
            continue;

        plugins.push_back(plugin._interface);
    }

    return plugins;
}
