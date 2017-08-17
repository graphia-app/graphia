#ifndef QMLUTILS_H
#define QMLUTILS_H

#include <QObject>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>

class QmlUtils : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE QString baseFileNameForUrl(const QUrl& url) const { return url.fileName(); }
    Q_INVOKABLE QString baseFileNameForUrlNoExtension(const QUrl& url) const
    {
        auto fi = QFileInfo(url.toLocalFile());
        return fi.baseName();
    }

    Q_INVOKABLE QString fileNameForUrl(const QUrl& url) const { return url.toLocalFile(); }
    Q_INVOKABLE QUrl urlForFileName(const QString& fileName) const { return QUrl::fromLocalFile(fileName); }
    Q_INVOKABLE bool fileUrlExists(const QUrl& url) const { return QFileInfo(url.toLocalFile()).exists(); }

    Q_INVOKABLE QUrl replaceExtension(const QUrl& url, const QString& extension) const
    {
        auto fi = QFileInfo(url.toLocalFile());
        auto replaced = QFileInfo(QString("%1/%2.%3")
                                  .arg(fi.path())
                                  .arg(fi.baseName())
                                  .arg(extension));

        return QUrl::fromLocalFile(replaced.filePath());
    }
};

#endif // QMLUTILS_H
