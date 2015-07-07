#ifndef APPLICATION_H
#define APPLICATION_H

#include "loading/fileidentifier.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <tuple>
#include <memory>

class GraphModel;
class GraphFileParser;

class Application : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList nameFilters READ nameFilters NOTIFY nameFiltersChanged)

public:
    explicit Application(QObject *parent = 0);

    const QStringList nameFilters() const { return _fileIdentifier.nameFilters(); }

    bool parserAndModelForFile(const QUrl& url, const QString& fileTypeName,
                               std::unique_ptr<GraphFileParser>& graphFileParser,
                               std::shared_ptr<GraphModel>& graphModel) const;

signals:
    void nameFiltersChanged();

public slots:
    bool canOpen(const QString& fileTypeName) const;
    bool canOpenAnyOf(const QStringList& fileTypeNames) const;
    QStringList fileTypesOf(const QUrl& url) const;

    QString name() const { return _name; }
    QString baseFileNameForUrl(const QUrl& url) const { return url.fileName(); }

    bool debugEnabled() const
    {
#ifdef _DEBUG
        return true;
#else
        return false;
#endif
    }

private:
    static const char* _name;
    FileIdentifier _fileIdentifier;
};

#endif // APPLICATION_H
