#include "application.h"

#include "shared/plugins/iplugin.h"

#include "shared/utils/utils.h"

#include <QPluginLoader>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>

#include <cmath>
#include <memory>
#include <iostream>

const char* Application::_uri = "com.kajeka";

Application::Application(QObject *parent) :
    QObject(parent),
    _urlTypeDetails(&_plugins),
    _pluginDetails(&_plugins)
{
    loadPlugins();
}

IPlugin* Application::pluginForName(const QString& pluginName) const
{
    for(auto plugin : _plugins)
    {
        if(plugin->name().compare(pluginName) == 0)
            return plugin;
    }

    return nullptr;
}

bool Application::fileUrlExists(const QUrl& url) const
{
    return QFileInfo(url.toLocalFile()).exists();
}

bool Application::canOpen(const QString& urlTypeName) const
{
    return std::any_of(_plugins.begin(), _plugins.end(),
    [&urlTypeName](auto plugin)
    {
        return plugin->loadableUrlTypeNames().contains(urlTypeName);
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
    QStringList urlTypeNames;

    for(auto plugin : _plugins)
        urlTypeNames.append(plugin->identifyUrl(url));

    urlTypeNames.removeDuplicates();

    return urlTypeNames;
}

QStringList Application::pluginNames(const QString& urlTypeName) const
{
    QStringList viablePluginNames;

    for(auto plugin : _plugins)
    {
        auto urlTypeNames = plugin->loadableUrlTypeNames();
        bool willLoad = std::any_of(urlTypeNames.begin(), urlTypeNames.end(),
        [&urlTypeName](const QString& loadableUrlTypeName)
        {
            return loadableUrlTypeName.compare(urlTypeName) == 0;
        });

        if(willLoad)
            viablePluginNames.append(plugin->name());
    }

    return viablePluginNames;
}

void Application::crash()
{
#ifndef __clang_analyzer__
    std::cerr << "Application::crash() invoked!\n";

    int* p = nullptr;
    *p = 123;
#endif
}

void Application::loadPlugins()
{
    std::vector<QString> pluginsDirs =
    {
        qApp->applicationDirPath() + "/plugins",
        QStandardPaths::writableLocation(
            QStandardPaths::StandardLocation::AppDataLocation) + "/plugins"
    };

#ifdef Q_OS_MAC
    QDir dotAppDir(qApp->applicationDirPath());

    // Within the bundle itself
    dotAppDir.cdUp();
    pluginsDirs.emplace_back(dotAppDir.absolutePath() + "/PlugIns");

    // Adjacent to the .app file
    dotAppDir.cdUp();
    dotAppDir.cdUp();
    pluginsDirs.emplace_back(dotAppDir.absolutePath() + "/plugins");
#endif

    for(auto& pluginsDir : pluginsDirs)
    {
        if(pluginsDir.isEmpty() || !QDir(pluginsDir).exists())
            continue;

        QDir pluginsQDir(pluginsDir);

        for(auto& fileName : pluginsQDir.entryList(QDir::Files))
        {
            if(!QLibrary::isLibrary(fileName))
                continue;

            QPluginLoader pluginLoader(pluginsQDir.absoluteFilePath(fileName));
            QObject* plugin = pluginLoader.instance();
            if(!pluginLoader.isLoaded())
            {
                QMessageBox::warning(nullptr, QObject::tr("Plugin Load Failed"),
                    QObject::tr("The plugin \"%1\" failed to load. The reported error is:\n%2")
                                     .arg(fileName)
                                     .arg(pluginLoader.errorString()), QMessageBox::Ok);
            }

            if(plugin)
            {
                auto* iplugin = qobject_cast<IPlugin*>(plugin);
                if(iplugin)
                {
                    bool pluginNameAlreadyUsed = std::any_of(_plugins.begin(), _plugins.end(),
                    [pluginName = iplugin->name()](auto loadedPlugin)
                    {
                        return loadedPlugin->name().compare(pluginName, Qt::CaseInsensitive) == 0;
                    });

                    if(pluginNameAlreadyUsed)
                    {
                        qDebug() << "WARNING: not loading plugin" << iplugin->name() <<
                                    "as a plugin of the same name is already loaded";
                        pluginLoader.unload();
                        continue;
                    }

                    initialisePlugin(iplugin);
                }
            }
        }
    }

    updateNameFilters();
}

void Application::initialisePlugin(IPlugin* plugin)
{
    _plugins.push_back(plugin);
    emit urlTypeDetailsChanged();
    emit pluginDetailsChanged();
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

static std::vector<UrlType> urlTypesForPlugins(const std::vector<IPlugin*>& plugins)
{
    std::vector<UrlType> fileTypes;

    for(auto plugin : plugins)
    {
        for(auto& urlTypeName : plugin->loadableUrlTypeNames())
        {
            UrlType fileType = {urlTypeName,
                                plugin->individualDescriptionForUrlTypeName(urlTypeName),
                                plugin->collectiveDescriptionForUrlTypeName(urlTypeName),
                                plugin->extensionsForUrlTypeName(urlTypeName)};
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
    std::vector<UrlType> fileTypes = urlTypesForPlugins(_plugins);

    QString description = QObject::tr("All Files (");
    bool second = false;

    for(auto fileType : fileTypes)
    {
        for(auto extension : fileType._extensions)
        {
            if(second)
                description += " ";
            else
                second = true;

            description += "*." + extension;
        }
    }

    description += ")";

    _nameFilters.clear();
    _nameFilters.append(description);

    for(auto fileType : fileTypes)
    {
        description = fileType._collectiveDescription + " (";
        second = false;

        for(auto extension : fileType._extensions)
        {
            if(second)
                description += " ";
            else
                second = true;

            description += "*." + extension;
        }

        description += ")";

        _nameFilters.append(description);
    }

    emit nameFiltersChanged();
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
    return static_cast<int>(urlTypesForPlugins(*_plugins).size());
}

QVariant UrlTypeDetailsModel::data(const QModelIndex& index, int role) const
{
    auto urlTypes = urlTypesForPlugins(*_plugins);

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
    return static_cast<int>(_plugins->size());
}

QVariant PluginDetailsModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();

    if(row < 0 || row >= rowCount(index))
        return QVariant(QVariant::Invalid);

    auto* plugin = _plugins->at(row);

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
                .arg(plugin->description()).arg(urlTypes);
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
