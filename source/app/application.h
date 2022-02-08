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

#ifndef APPLICATION_H
#define APPLICATION_H

#include "updates/updater.h"

#include "shared/utils/qmlenum.h"
#include "shared/utils/downloadqueue.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QRect>
#include <QColor>
#include <QAbstractListModel>
#include <QPluginLoader>
#include <QImage>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QStringListModel>

#include <vector>
#include <memory>

#ifndef APP_URI
#define APP_URI "uri.missing" // NOLINT cppcoreguidelines-macro-usage
#endif
#ifndef APP_MAJOR_VERSION
#define APP_MAJOR_VERSION (-1) // NOLINT cppcoreguidelines-macro-usage
#endif
#ifndef APP_MINOR_VERSION
#define APP_MINOR_VERSION (-1) // NOLINT cppcoreguidelines-macro-usage
#endif

class GraphModel;
class IParser;
class ISaverFactory;
class IPlugin;

DEFINE_QML_ENUM(
    Q_GADGET, ExitType,
    NormalExit = 0,
    Restart = 127);

struct LoadedPlugin
{
    LoadedPlugin(IPlugin* plugin, std::unique_ptr<QPluginLoader> loader) :
        _interface(plugin), _loader(std::move(loader))
    {}

    IPlugin* _interface;
    std::unique_ptr<QPluginLoader> _loader;
};

class UrlTypeDetailsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit UrlTypeDetailsModel(const std::vector<LoadedPlugin>* loadedPlugins) :
        _loadedPlugins(loadedPlugins)
    {}

    enum Roles
    {
        Name = Qt::UserRole + 1,
        IndividualDescription,
        CollectiveDescription
    };

    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void update() { emit layoutChanged(); }

private:
    const std::vector<LoadedPlugin>* _loadedPlugins;
};

class PluginDetailsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PluginDetailsModel(const std::vector<LoadedPlugin>* loadedPlugins) :
        _loadedPlugins(loadedPlugins)
    {}

    enum Roles
    {
        Name = Qt::UserRole + 1,
        Description,
        ImageSource
    };

    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void update() { emit layoutChanged(); }

private:
    const std::vector<LoadedPlugin>* _loadedPlugins;
};

class Application : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString copyright READ copyright CONSTANT)
    Q_PROPERTY(QString nativeExtension READ nativeExtension CONSTANT)
    Q_PROPERTY(QStringList resourceDirectories READ resourceDirectories CONSTANT)
    Q_PROPERTY(QStringList arguments READ arguments CONSTANT)

    Q_PROPERTY(QStringList environment READ environment CONSTANT)

    Q_PROPERTY(QStringList nameFilters READ nameFilters NOTIFY nameFiltersChanged)
    Q_PROPERTY(QStringListModel* loadableExtensions READ loadableExtensions NOTIFY loadableExtensionsChanged)
    Q_PROPERTY(QAbstractListModel* urlTypeDetails READ urlTypeDetails NOTIFY urlTypeDetailsChanged)
    Q_PROPERTY(QAbstractListModel* pluginDetails READ pluginDetails NOTIFY pluginDetailsChanged)

    Q_PROPERTY(int updateDownloadProgress READ updateDownloadProgress NOTIFY updateDownloadProgressChanged)

    Q_PROPERTY(bool downloadActive READ downloadActive NOTIFY downloadActiveChanged)
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)

    Q_PROPERTY(bool debugEnabled READ debugEnabled CONSTANT)

public:
    static constexpr const char* NativeFileType = "Native";

    explicit Application(QObject *parent = nullptr);
    ~Application() override;

    IPlugin* pluginForName(const QString& pluginName) const;

    static QString name() { return QCoreApplication::applicationName(); }
    static QString version() { return QCoreApplication::applicationVersion(); }
    static QString copyright() { return QStringLiteral(COPYRIGHT).replace(QStringLiteral("(c)"), QStringLiteral(u"©")); }

    static QString nativeExtension() { return name().toLower(); }

    static void setAppDir(const QString& appDir) { Application::_appDir = appDir; }

    static QStringList resourceDirectories();
    static QStringList arguments() { return QCoreApplication::arguments(); }

    static QStringList environment() { return QProcessEnvironment::systemEnvironment().toStringList(); }

    Q_INVOKABLE bool canOpen(const QString& urlTypeName) const;
    Q_INVOKABLE bool canOpenAnyOf(const QStringList& urlTypeNames) const;
    Q_INVOKABLE QStringList urlTypesOf(const QUrl& url) const;
    Q_INVOKABLE QStringList urlTypesFor(const QString& extension) const;
    Q_INVOKABLE QString descriptionForUrlType(const QString& urlType) const;
    Q_INVOKABLE QString urlTypeFor(const QString& description, const QStringList& extensions) const;
    Q_INVOKABLE QStringList failureReasons(const QUrl& url) const;

    Q_INVOKABLE void download(const QUrl& url);
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void resumeDownload();
    Q_INVOKABLE bool downloaded(const QUrl& url);

    void registerSaverFactory(std::unique_ptr<ISaverFactory> saver);
    ISaverFactory* saverFactoryByName(const QString& name);
    Q_INVOKABLE QVariantList saverFileTypes();

    Q_INVOKABLE QObject* qmlPluginForName(const QString& pluginName) const;
    Q_INVOKABLE QStringList pluginNames(const QString& urlTypeName) const;
    Q_INVOKABLE QString parametersQmlPathForPlugin(const QString& pluginName,
        const QString& urlType) const;

    static const char* uri() { return _uri; }
    static int majorVersion() { return _majorVersion; }
    static int minorVersion() { return _minorVersion; }

    Q_INVOKABLE void checkForUpdates();

    Q_INVOKABLE void copyImageToClipboard(const QImage& image);

    Q_INVOKABLE QString resourceFile(const QString& relativePath) const;
    Q_INVOKABLE bool isResourceFile(const QString& path) const;
    Q_INVOKABLE bool isResourceFileUrl(const QUrl& url) const;

    Q_INVOKABLE bool isNativeLink(const QUrl& url) const;
    Q_INVOKABLE int linkVersionFor(const QUrl& url) const;
    Q_INVOKABLE QStringList linkArgumentsFor(const QUrl& url) const;

    Q_INVOKABLE void crash(int crashType);

    Q_INVOKABLE void reportScopeTimers();

    Q_INVOKABLE void aboutQt() const;

    Q_INVOKABLE void submitTrackingData() const;

    static QString resolvedExe(const QString& exe);

signals:
    void nameFiltersChanged();
    void loadableExtensionsChanged();
    void pluginDetailsChanged();
    void urlTypeDetailsChanged();

    void noNewUpdateAvailable(bool existing);
    void newUpdateAvailable();
    void updateDownloadProgressChanged();

    void changeLogStored();

    void downloadActiveChanged();
    void downloadProgressChanged();
    void downloadError(const QUrl& url, const QString& text);
    void downloadComplete(const QUrl& url, const QString& fileName);

private:
    static const char* const _uri;
    static const int _majorVersion = APP_MAJOR_VERSION;
    static const int _minorVersion = APP_MINOR_VERSION;

    static QString _appDir;

    Updater _updater;

    DownloadQueue _downloadQueue;

    UrlTypeDetailsModel _urlTypeDetails;

    std::vector<LoadedPlugin> _loadedPlugins;
    PluginDetailsModel _pluginDetails;
    std::vector<std::unique_ptr<ISaverFactory>> _factories;

    QStringList _nameFilters;
    QStringListModel _loadableExtensions;

    void loadPlugins();
    void initialisePlugin(IPlugin* plugin, std::unique_ptr<QPluginLoader> pluginLoader);
    void updateLoadingCapabilities();
    void unloadPlugins();

    QStringList nameFilters() const { return _nameFilters; }
    QStringListModel* loadableExtensions() { return &_loadableExtensions; }

    QAbstractListModel* urlTypeDetails();
    QAbstractListModel* pluginDetails();

    int updateDownloadProgress() const { return _updater.progress(); }

    bool downloadActive() const { return !_downloadQueue.idle(); }
    int downloadProgress() const { return _downloadQueue.progress(); }

    static bool debugEnabled()
    {
#ifdef _DEBUG
        return true;
#else
        return false;
#endif
    }
};

#endif // APPLICATION_H
