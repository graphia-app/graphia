#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QRect>
#include <QColor>
#include <QAbstractListModel>

#include <vector>
#include <memory>

#include <QCoreApplication>

class GraphModel;
class IParser;
class IPlugin;

class UrlTypeDetailsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit UrlTypeDetailsModel(const std::vector<IPlugin*>* plugins) :
        _plugins(plugins)
    {}

    enum Roles
    {
        Name = Qt::UserRole + 1,
        IndividualDescription,
        CollectiveDescription
    };

    int rowCount(const QModelIndex&) const;
    QVariant data(const QModelIndex& index, int role) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    QString nameAtIndex(int i) const { return data(index(i), Name).toString(); }

private:
    const std::vector<IPlugin*>* _plugins;
};

class PluginDetailsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PluginDetailsModel(const std::vector<IPlugin*>* plugins) :
        _plugins(plugins)
    {}

    enum Roles
    {
        Name = Qt::UserRole + 1,
        Description,
        ImageSource
    };

    int rowCount(const QModelIndex&) const;
    QVariant data(const QModelIndex& index, int role) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    QString nameAtIndex(int i) const { return data(index(i), Name).toString(); }

private:
    const std::vector<IPlugin*>* _plugins;
};

class Application : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString copyright READ copyright CONSTANT)
    Q_PROPERTY(QStringList nameFilters READ nameFilters NOTIFY nameFiltersChanged)
    Q_PROPERTY(QAbstractListModel* urlTypeDetails READ urlTypeDetails NOTIFY urlTypeDetailsChanged)
    Q_PROPERTY(QAbstractListModel* pluginDetails READ pluginDetails NOTIFY pluginDetailsChanged)

    Q_PROPERTY(bool debugEnabled READ debugEnabled CONSTANT)

public:
    explicit Application(QObject *parent = nullptr);

    IPlugin* pluginForName(const QString& pluginName) const;

    static QString name() { return QCoreApplication::applicationName(); }
    static QString version() { return QCoreApplication::applicationVersion(); }
    static QString copyright() { return QString(COPYRIGHT).replace("(c)", "©"); }

signals:
    void nameFiltersChanged();
    void pluginDetailsChanged();
    void urlTypeDetailsChanged();

public slots:
    bool fileUrlExists(const QUrl& url) const;

    bool canOpen(const QString& urlTypeName) const;
    bool canOpenAnyOf(const QStringList& urlTypeNames) const;
    QStringList urlTypesOf(const QUrl& url) const;

    QStringList pluginNames(const QString& urlTypeName) const;

    static const char* uri() { return _uri; }
    static int majorVersion() { return _majorVersion; }
    static int minorVersion() { return _minorVersion; }

    QString baseFileNameForUrl(const QUrl& url) const { return url.fileName(); }
    QUrl urlForFileName(const QString& fileName) const { return QUrl::fromLocalFile(fileName); }

    void crash();

private:
    static const char* _uri;
    static const int _majorVersion = 1;
    static const int _minorVersion = 0;

    UrlTypeDetailsModel _urlTypeDetails;

    std::vector<IPlugin*> _plugins;
    PluginDetailsModel _pluginDetails;

    void loadPlugins();
    void initialisePlugin(IPlugin* plugin);
    void updateNameFilters();

    QStringList _nameFilters;
    QStringList nameFilters() const { return _nameFilters; }

    QAbstractListModel* urlTypeDetails();
    QAbstractListModel* pluginDetails();

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
