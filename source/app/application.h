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
    Q_PROPERTY(QAbstractListModel* pluginDetails READ pluginDetails NOTIFY pluginDetailsChanged)

public:
    explicit Application(QObject *parent = nullptr);

    IPlugin* pluginForUrlTypeName(const QString& urlTypeName) const;

    static QString name() { return QCoreApplication::applicationName(); }
    static QString version() { return QCoreApplication::applicationVersion(); }
    static QString copyright() { return QString(COPYRIGHT).replace("(c)", "©"); }

signals:
    void nameFiltersChanged();
    void pluginDetailsChanged();

public slots:
    bool canOpen(const QString& urlTypeName) const;
    bool canOpenAnyOf(const QStringList& urlTypeNames) const;
    QStringList urlTypesOf(const QUrl& url) const;

    static const char* uri() { return _uri; }
    static int majorVersion() { return _majorVersion; }
    static int minorVersion() { return _minorVersion; }

    QString baseFileNameForUrl(const QUrl& url) const { return url.fileName(); }
    QUrl urlForFileName(const QString& fileName) const { return QUrl::fromLocalFile(fileName); }

    bool debugEnabled() const
    {
#ifdef _DEBUG
        return true;
#else
        return false;
#endif
    }

private:
    static const char* _uri;
    static const int _majorVersion = 1;
    static const int _minorVersion = 0;

    std::vector<IPlugin*> _plugins;
    PluginDetailsModel _pluginDetails;

    void loadPlugins();
    void initialisePlugin(IPlugin* plugin);
    void updateNameFilters();

    QStringList _nameFilters;
    QStringList nameFilters() const { return _nameFilters; }

    QAbstractListModel* pluginDetails();
};

#endif // APPLICATION_H
