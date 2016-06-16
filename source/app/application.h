#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QRect>
#include <QColor>

#include <vector>
#include <map>
#include <memory>

#include <QCoreApplication>

class GraphModel;
class IParser;
class IPlugin;

class Application : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString copyright READ copyright CONSTANT)
    Q_PROPERTY(QStringList nameFilters READ nameFilters NOTIFY nameFiltersChanged)
    Q_PROPERTY(QStringList pluginNames READ pluginNames NOTIFY pluginNamesChanged)

public:
    explicit Application(QObject *parent = nullptr);

    IPlugin* pluginForUrlTypeName(const QString& urlTypeName) const;

    static QString name() { return QCoreApplication::applicationName(); }
    static QString version() { return QCoreApplication::applicationVersion(); }
    static QString copyright() { return QString(COPYRIGHT).replace("(c)", "©"); }

signals:
    void nameFiltersChanged();
    void pluginNamesChanged();

public slots:
    bool canOpen(const QString& urlTypeName) const;
    bool canOpenAnyOf(const QStringList& urlTypeNames) const;
    QStringList urlTypesOf(const QUrl& url) const;

    static const char* uri() { return _uri; }
    static int majorVersion() { return _majorVersion; }
    static int minorVersion() { return _minorVersion; }

    QString baseFileNameForUrl(const QUrl& url) const { return url.fileName(); }
    QUrl urlForFileName(const QString& fileName) const { return QUrl::fromLocalFile(fileName); }

    QString descriptionForPluginName(const QString& pluginName) const;
    QString imageSourceForPluginName(const QString& pluginName) const;

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

    std::map<QString, IPlugin*> _plugins;

    void loadPlugins();
    void initialisePlugin(IPlugin* plugin);
    void updateNameFilters();

    QStringList _nameFilters;
    QStringList nameFilters() const { return _nameFilters; }

    QStringList pluginNames() const;
};

#endif // APPLICATION_H
