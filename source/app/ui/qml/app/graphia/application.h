/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "app/updates/updater.h"

#include "shared/iapplication.h"
#include "shared/utils/qmlenum.h"
#include "shared/utils/downloadqueue.h"

#include <QObject>
#include <QQmlEngine>
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
#include <QtGlobal>

#include <vector>
#include <memory>

class GraphModel;
class IParser;
class ISaverFactory;
class IPlugin;

DEFINE_QML_ENUM(ExitType,
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
    QML_ANONYMOUS

    Q_PROPERTY(QStringList filter MEMBER _filter NOTIFY filterChanged)

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

private:
    const std::vector<LoadedPlugin>* _loadedPlugins;
    QStringList _filter;

signals:
    void filterChanged();
};

class PluginDetailsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(QStringList filter MEMBER _filter NOTIFY filterChanged)

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

private:
    const std::vector<LoadedPlugin>* _loadedPlugins;
    QStringList _filter;

    std::vector<const IPlugin*> filteredPlugins() const;

signals:
    void filterChanged();
};

class Application : public QObject, public IApplication
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString copyright READ copyright CONSTANT)
    Q_PROPERTY(QString nativeExtension READ nativeExtension CONSTANT)
    Q_PROPERTY(QStringList resourceDirectories READ resourceDirectories CONSTANT)
    Q_PROPERTY(QStringList arguments READ arguments CONSTANT)

    Q_PROPERTY(QStringList environment READ environment CONSTANT)
    Q_PROPERTY(QString openGLInfo MEMBER _openGLInfo CONSTANT)

    Q_PROPERTY(QStringList nameFilters READ nameFilters NOTIFY nameFiltersChanged)
    Q_PROPERTY(QStringListModel* loadableExtensions READ loadableExtensions NOTIFY loadableExtensionsChanged)
    Q_PROPERTY(QStringListModel* ambiguousExtensions READ ambiguousExtensions NOTIFY ambiguousExtensionsChanged)
    Q_PROPERTY(QStringListModel* ambiguousUrlTypes READ ambiguousUrlTypes NOTIFY ambiguousUrlTypesChanged)

    Q_PROPERTY(int updateDownloadProgress READ updateDownloadProgress NOTIFY updateDownloadProgressChanged)

    Q_PROPERTY(bool downloadActive READ downloadActive NOTIFY downloadActiveChanged)
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)

    Q_PROPERTY(bool debugEnabled READ debugEnabled CONSTANT)
    Q_PROPERTY(bool runningWasm READ runningWasm CONSTANT)

public:
    static constexpr const char* NativeFileType = "Native";

    explicit Application(QObject *parent = nullptr, bool enableUpdateCheck = true);
    ~Application() override;

    IPlugin* pluginForName(const QString& pluginName) const;

    static QString name() { return QCoreApplication::applicationName(); }
    static QString version() { return QCoreApplication::applicationVersion(); }
    static QString copyright();

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
    Q_INVOKABLE QString qmlModuleForPlugin(const QString& pluginName) const;
    Q_INVOKABLE QString parametersQmlTypeForPlugin(const QString& pluginName,
        const QString& urlType) const;

    Q_INVOKABLE UrlTypeDetailsModel* urlTypeDetailsModel() const;
    Q_INVOKABLE PluginDetailsModel* pluginDetailsModel() const;

    Q_INVOKABLE void checkForUpdates();

    Q_INVOKABLE void copyImageToClipboard(const QImage& image);

    Q_INVOKABLE QString resourceFile(const QString& relativePath) const;
    Q_INVOKABLE bool isResourceFile(const QString& path) const;
    Q_INVOKABLE bool isResourceFileUrl(const QUrl& url) const;

    Q_INVOKABLE bool isNativeLink(const QUrl& url) const;
    Q_INVOKABLE int linkVersionFor(const QUrl& url) const;
    Q_INVOKABLE QStringList linkArgumentsFor(const QUrl& url) const;

    Q_INVOKABLE void crash(int crashType);
    Q_INVOKABLE void testConsoleOutput();

    Q_INVOKABLE void reportScopeTimers();

    Q_INVOKABLE void aboutQt() const;

    Q_INVOKABLE void submitTrackingData() const;

    QString displayTextForTransform(const QString& transform) const override;
    QString displayTextForVisualisation(const QString& visualisation) const override;

    static QString resolvedExe(const QString& exe);

signals:
    void nameFiltersChanged();
    void loadableExtensionsChanged();
    void ambiguousExtensionsChanged();
    void ambiguousUrlTypesChanged();

    void noNewUpdateAvailable(bool existing);
    void newUpdateAvailable();
    void updateDownloadProgressChanged();

    void changeLogStored();

    void downloadActiveChanged();
    void downloadProgressChanged();
    void downloadError(const QUrl& url, const QString& text);
    void downloadComplete(const QUrl& url, const QString& fileName);

private:
    static QString _appDir;

    QString _openGLInfo;

    Updater _updater;

    DownloadQueue _downloadQueue;

    std::vector<LoadedPlugin> _loadedPlugins;
    std::vector<std::unique_ptr<ISaverFactory>> _factories;

    QStringList _nameFilters;
    QStringListModel _loadableExtensions;
    QStringListModel _ambiguousExtensions;
    QStringListModel _ambiguousUrlTypes;

    void loadPlugins();
    bool initialisePlugin(IPlugin* plugin, std::unique_ptr<QPluginLoader> pluginLoader);
    void updateLoadingCapabilities();
    void unloadPlugins();

    QStringList nameFilters() const { return _nameFilters; }
    QStringListModel* loadableExtensions() { return &_loadableExtensions; }
    QStringListModel* ambiguousExtensions() { return &_ambiguousExtensions; }
    QStringListModel* ambiguousUrlTypes() { return &_ambiguousUrlTypes; }

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

    static bool runningWasm()
    {
#ifdef Q_OS_WASM
        return true;
#else
        return false;
#endif
    }
};

#endif // APPLICATION_H
