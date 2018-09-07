#ifndef APPLICATION_H
#define APPLICATION_H

#include "auth/auth.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QRect>
#include <QColor>
#include <QAbstractListModel>
#include <QPluginLoader>
#include <QImage>

#include <vector>
#include <memory>

#include <QCoreApplication>

#ifndef APP_URI
#define APP_URI "uri.missing"
#endif
#ifndef APP_MAJOR_VERSION
#define APP_MAJOR_VERSION -1
#endif
#ifndef APP_MINOR_VERSION
#define APP_MINOR_VERSION -1
#endif

class GraphModel;
class IParser;
class IExporter;
class IPlugin;

struct LoadedPlugin
{
    LoadedPlugin(IPlugin* instance, std::unique_ptr<QPluginLoader> loader) :
        _instance(instance), _loader(std::move(loader))
    {}

    IPlugin* _instance;
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

    Q_INVOKABLE QString nameAtIndex(int row) const { return data(index(row, 0), Name).toString(); }

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

    Q_INVOKABLE QString nameAtIndex(int row) const { return data(index(row, 0), Name).toString(); }

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


    Q_PROPERTY(QStringList nameFilters READ nameFilters NOTIFY nameFiltersChanged)
    Q_PROPERTY(QAbstractListModel* urlTypeDetails READ urlTypeDetails NOTIFY urlTypeDetailsChanged)
    Q_PROPERTY(QAbstractListModel* pluginDetails READ pluginDetails NOTIFY pluginDetailsChanged)

    Q_PROPERTY(bool authenticated READ authenticated NOTIFY authenticatedChanged)
    Q_PROPERTY(QString authenticationMessage READ authenticationMessage NOTIFY authenticationMessageChanged)
    Q_PROPERTY(bool authenticating READ authenticating NOTIFY authenticatingChanged)

    Q_PROPERTY(bool debugEnabled READ debugEnabled CONSTANT)

public:
    static constexpr const char* NativeFileType = "Native";

    explicit Application(QObject *parent = nullptr);
    ~Application();

    IPlugin* pluginForName(const QString& pluginName) const;

    static QString name() { return QCoreApplication::applicationName(); }
    static QString version() { return QCoreApplication::applicationVersion(); }
    static QString copyright() { return QStringLiteral(COPYRIGHT).replace(QStringLiteral("(c)"), QStringLiteral(u"©")); }

    static QString nativeExtension() { return name().toLower(); }

    static QStringList resourceDirectories();

    Q_INVOKABLE bool canOpen(const QString& urlTypeName) const;
    Q_INVOKABLE bool canOpenAnyOf(const QStringList& urlTypeNames) const;
    Q_INVOKABLE QStringList urlTypesOf(const QUrl& url) const;
    Q_INVOKABLE QStringList failureReasons(const QUrl& url) const;

    void registerSaveFileType(std::unique_ptr<IExporter> exporter);
    IExporter& getExporter(int id) { return *_exporters[id].get(); }
    Q_INVOKABLE QStringList saveFileNames();
    Q_INVOKABLE QStringList saveFileUrls();

    Q_INVOKABLE QStringList pluginNames(const QString& urlTypeName) const;
    Q_INVOKABLE QString parametersQmlPathForPlugin(const QString& pluginName) const;

    static const char* uri() { return _uri; }
    static int majorVersion() { return _majorVersion; }
    static int minorVersion() { return _minorVersion; }

    // Returns false if we need to authenticate, but couldn't
    Q_INVOKABLE bool tryToAuthenticateWithCachedCredentials();

    Q_INVOKABLE void authenticate(const QString& email, const QString& password);
    Q_INVOKABLE void signOut();

    Q_INVOKABLE void copyImageToClipboard(const QImage& image);

    Q_INVOKABLE QString resourceFile(const QString& relativePath) const;
    Q_INVOKABLE bool isResourceFile(const QString& path) const;
    Q_INVOKABLE bool isResourceFileUrl(const QUrl& url) const;

    Q_INVOKABLE void crash(int crashType);

    Q_INVOKABLE void reportScopeTimers();

    Q_INVOKABLE void aboutQt() const;

    static QString resolvedExe(const QString& exe);

signals:
    void nameFiltersChanged();
    void pluginDetailsChanged();
    void urlTypeDetailsChanged();

    void authenticatedChanged();
    void authenticationMessageChanged();
    void authenticatingChanged();

private:
    static const char* _uri;
    static const int _majorVersion = APP_MAJOR_VERSION;
    static const int _minorVersion = APP_MINOR_VERSION;

    Auth _auth;

    UrlTypeDetailsModel _urlTypeDetails;

    std::vector<LoadedPlugin> _loadedPlugins;
    PluginDetailsModel _pluginDetails;
    std::vector<std::unique_ptr<IExporter>> _exporters;

    void loadPlugins();
    void initialisePlugin(IPlugin* plugin, std::unique_ptr<QPluginLoader> pluginLoader);
    void updateNameFilters();
    void unloadPlugins();

    QStringList _nameFilters;
    QStringList nameFilters() const { return _nameFilters; }

    QAbstractListModel* urlTypeDetails();
    QAbstractListModel* pluginDetails();

    bool authenticated() const { return _auth.state(); }
    QString authenticationMessage() const { return _auth.message(); }
    bool authenticating() const { return _auth.busy(); }

    bool debugEnabled() const
    {
#ifdef _DEBUG
        return true;
#else
        return false;
#endif
    }
};

#endif // APPLICATION_H
