#include "application.h"
#include "crashtype.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/fatalerror.h"
#include "shared/utils/thread.h"
#include "shared/utils/scopetimer.h"
#include "shared/utils/preferences.h"

#include "loading/loader.h"

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QClipboard>

#include <cmath>
#include <memory>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

const char* Application::_uri = APP_URI;

Application::Application(QObject *parent) :
    QObject(parent),
    _urlTypeDetails(&_loadedPlugins),
    _pluginDetails(&_loadedPlugins)
{
    connect(&_auth, &Auth::stateChanged, [this]
    {
        if(_auth.state())
            loadPlugins();
    });

    connect(&_auth, &Auth::stateChanged, this, &Application::authenticatedChanged);
    connect(&_auth, &Auth::messageChanged, this, &Application::authenticationMessageChanged);
    connect(&_auth, &Auth::busyChanged, this, &Application::authenticatingChanged);
}

IPlugin* Application::pluginForName(const QString& pluginName) const
{
    for(const auto& loadedPlugin : _loadedPlugins)
    {
        if(loadedPlugin._instance->name().compare(pluginName) == 0)
            return loadedPlugin._instance;
    }

    return nullptr;
}

#ifdef Q_OS_MACOS
#include <corefoundation/CFBundle.h>
#endif

QStringList Application::resourceDirectories()
{
    QStringList resourceDirs
    {
        QCoreApplication::applicationDirPath(),
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
    QDir usrDir(QCoreApplication::applicationDirPath());
    usrDir.cdUp();

    resourceDirs.append(usrDir.absolutePath() + "/share/" + name());
#endif

    return resourceDirs;
}

bool Application::canOpen(const QString& urlTypeName) const
{
    if(!_auth.state())
    {
        // We should never get here unless somebody is trying to
        // crack the auth system (by messing with the QML?)
        return false;
    }

    if(urlTypeName == NativeFileType)
        return true;

    return std::any_of(_loadedPlugins.begin(), _loadedPlugins.end(),
    [&urlTypeName](const auto& loadedPlugin)
    {
        return loadedPlugin._instance->loadableUrlTypeNames().contains(urlTypeName);
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
        urlTypeNames.append(loadedPlugin._instance->identifyUrl(url));

    urlTypeNames.removeDuplicates();

    return urlTypeNames;
}

QStringList Application::failureReasons(const QUrl& url) const
{
    QStringList failureReasons;
    failureReasons.reserve(static_cast<int>(_loadedPlugins.size()));

    for(const auto& loadedPlugin : _loadedPlugins)
    {
        auto failureReason = loadedPlugin._instance->failureReason(url);

        if(!failureReason.isEmpty())
            failureReasons.append(failureReason);
    }

    failureReasons.removeDuplicates();

    return failureReasons;
}

QStringList Application::pluginNames(const QString& urlTypeName) const
{
    QStringList viablePluginNames;

    for(const auto& loadedPlugin : _loadedPlugins)
    {
        auto urlTypeNames = loadedPlugin._instance->loadableUrlTypeNames();
        bool willLoad = std::any_of(urlTypeNames.begin(), urlTypeNames.end(),
        [&urlTypeName](const QString& loadableUrlTypeName)
        {
            return loadableUrlTypeName.compare(urlTypeName) == 0;
        });

        if(willLoad)
            viablePluginNames.append(loadedPlugin._instance->name());
    }

    return viablePluginNames;
}

QString Application::parametersQmlPathForPlugin(const QString& pluginName) const
{
    auto plugin = pluginForName(pluginName);

    if(plugin != nullptr)
        return plugin->parametersQmlPath();

    return {};
}

bool Application::tryToAuthenticateWithCachedCredentials()
{
    if(!u::pref("auth/rememberMe").toBool())
        return false;

    if(!_auth.state() && _auth.expired())
        return _auth.sendRequestUsingCachedCredentials();

    return true;
}

void Application::authenticate(const QString& email, const QString& password)
{
    _auth.sendRequest(email, password);
}

void Application::signOut()
{
    _auth.reset();
    unloadPlugins();
}

void Application::copyImageToClipboard(const QImage& image)
{
    QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
}

QString Application::resourceFile(const QString& relativePath) const
{
    for(const auto& resourceDirectory : resourceDirectories())
    {
        auto resolvedPath = QDir(resourceDirectory).filePath(relativePath);

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

    for(const auto& resourceDirectory : resourceDirectories())
    {
        QString canonicalResourceDirectory = QFileInfo(resourceDirectory).canonicalPath();

        if(canonicalPath.startsWith(canonicalResourceDirectory))
            return true;
    }

    return false;
}

bool Application::isResourceFileUrl(const QUrl& url) const
{
    return isResourceFile(url.toLocalFile());
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
        u::setCurrentThreadName(QStringLiteral("DeadlockThread"));

        std::unique_lock<std::mutex> lockA(a);
        std::this_thread::sleep_for(1s);
        std::unique_lock<std::mutex> lockB(b);
    });

    std::unique_lock<std::mutex> lockB(b);
    std::this_thread::sleep_for(1s);
    std::unique_lock<std::mutex> lockA(a);

    t.join();
}

static void hitch()
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(35s);
}

void Application::crash(int crashType)
{
    std::cerr << "Application::crash() invoked!\n";

    auto _crashType = static_cast<CrashType>(crashType);

    switch(_crashType)
    {
    default:
        break;

    case CrashType::NullPtrDereference:
    {
        int* p = nullptr;
        // cppcheck-suppress nullPointer
        *p = 0;
        break;
    }

    case CrashType::CppException:
        throw;
        break;

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
    }
}

void Application::reportScopeTimers()
{
    ScopeTimerManager::instance()->reportToQDebug();
}

QString Application::resolvedExe(const QString& exe)
{
    QString fullyQualifiedExe(
        QCoreApplication::applicationDirPath() +
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
    QDir dotAppDir(QCoreApplication::applicationDirPath());
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
        QCoreApplication::applicationDirPath() + "/plugins",
        QStandardPaths::writableLocation(
            QStandardPaths::StandardLocation::AppDataLocation) + "/plugins"
    };

#if defined(Q_OS_MACOS)
    QDir dotAppDir(QCoreApplication::applicationDirPath());

    // Within the bundle itself
    dotAppDir.cdUp();
    pluginsDirs.emplace_back(dotAppDir.absolutePath() + "/PlugIns");

    // Adjacent to the .app file
    dotAppDir.cdUp();
    dotAppDir.cdUp();
    pluginsDirs.emplace_back(dotAppDir.absolutePath() + "/plugins");
#elif defined(Q_OS_LINUX)
    // Add the LSB location for the plugins
    QDir usrDir(QCoreApplication::applicationDirPath());
    usrDir.cdUp();

    pluginsDirs.emplace_back(usrDir.absolutePath() + "/lib/" + name() + "/plugins");
#endif

    for(auto& pluginsDir : pluginsDirs)
    {
        if(pluginsDir.isEmpty() || !QDir(pluginsDir).exists())
            continue;

        std::cerr << "Loading plugins from " << pluginsDir.toStdString() << "\n";

        QDir pluginsQDir(pluginsDir);
        auto fileNames = pluginsQDir.entryList(QDir::Files);

        if(fileNames.empty())
        {
            std::cerr << "  ...none found\n";
            continue;
        }

        for(auto& fileName : fileNames)
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

            if(plugin != nullptr)
            {
                auto* iplugin = qobject_cast<IPlugin*>(plugin);
                if(iplugin != nullptr)
                {
                    if(!_auth.pluginAllowed(iplugin->name()))
                    {
                        pluginLoader->unload();
                        continue;
                    }

                    bool pluginNameAlreadyUsed = std::any_of(_loadedPlugins.begin(), _loadedPlugins.end(),
                    [pluginName = iplugin->name()](const auto& loadedPlugin)
                    {
                        return loadedPlugin._instance->name().compare(pluginName, Qt::CaseInsensitive) == 0;
                    });

                    if(pluginNameAlreadyUsed)
                    {
                        std::cerr << "  ..." << QFileInfo(fileName).fileName().toStdString() <<
                            " (" << iplugin->name().toStdString() <<
                            ") is already loaded from a different location\n";
                        pluginLoader->unload();
                        continue;
                    }

                    initialisePlugin(iplugin, std::move(pluginLoader));

                    std::cerr << "  ..." << QFileInfo(fileName).fileName().toStdString() <<
                        " (" << iplugin->name().toStdString() << ") loaded successfully\n";
                }
            }
        }
    }

    // Force event processing here so that we initialise any qmlenum.h based enums
    // that were created in plugins
    QCoreApplication::processEvents();

    updateNameFilters();
}

void Application::initialisePlugin(IPlugin* plugin, std::unique_ptr<QPluginLoader> pluginLoader)
{
    _loadedPlugins.emplace_back(plugin, std::move(pluginLoader));
    _urlTypeDetails.update();
    _pluginDetails.update();
}

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
        for(auto& urlTypeName : plugin._instance->loadableUrlTypeNames())
        {
            UrlType fileType = {urlTypeName,
                                plugin._instance->individualDescriptionForUrlTypeName(urlTypeName),
                                plugin._instance->collectiveDescriptionForUrlTypeName(urlTypeName),
                                plugin._instance->extensionsForUrlTypeName(urlTypeName)};
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

void Application::updateNameFilters()
{
    // Initialise with native file type
    std::vector<UrlType> fileTypes{{NativeFileType, QString("%1 File").arg(name()),
        QString("%1 Files").arg(name()), {nativeExtension()}}};

    auto pluginFileTypes = urlTypesForPlugins(_loadedPlugins);
    fileTypes.insert(fileTypes.end(), pluginFileTypes.begin(), pluginFileTypes.end());

    QString description = QObject::tr("All Files (");
    bool second = false;

    for(const auto& fileType : fileTypes)
    {
        for(const auto& extension : fileType._extensions)
        {
            if(second)
                description += QLatin1String(" ");
            else
                second = true;

            description += "*." + extension;
        }
    }

    description += QLatin1String(")");

    _nameFilters.clear();
    _nameFilters.append(description);

    for(const auto& fileType : fileTypes)
    {
        description = fileType._collectiveDescription + " (";
        second = false;

        for(const auto& extension : fileType._extensions)
        {
            if(second)
                description += QLatin1String(" ");
            else
                second = true;

            description += "*." + extension;
        }

        description += QLatin1String(")");

        _nameFilters.append(description);
    }

    emit nameFiltersChanged();
}

void Application::unloadPlugins()
{
    for(const auto& loadedPlugin : _loadedPlugins)
        loadedPlugin._loader->unload();

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

    int row = index.row();

    if(row < 0 || row >= static_cast<int>(urlTypes.size()))
        return QVariant(QVariant::Invalid);

    auto& urlType = urlTypes.at(row);

    switch(role)
    {
    case Name:                  return urlType._name;
    case IndividualDescription: return urlType._individualDescription;
    case CollectiveDescription: return urlType._collectiveDescription;
    default: break;
    }

    return QVariant(QVariant::Invalid);
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
    int row = index.row();

    if(row < 0 || row >= rowCount(index))
        return QVariant(QVariant::Invalid);

    auto* plugin = _loadedPlugins->at(row)._instance;

    switch(role)
    {
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

    return QVariant(QVariant::Invalid);
}

QHash<int, QByteArray> PluginDetailsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[Description] = "description";
    roles[ImageSource] = "imageSource";
    return roles;
}
