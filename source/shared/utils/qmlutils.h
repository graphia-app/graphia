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
    Q_INVOKABLE QString fileNameForUrl(const QUrl& url) const { return url.toLocalFile(); }
    Q_INVOKABLE QUrl urlForFileName(const QString& fileName) const { return QUrl::fromLocalFile(fileName); }
    Q_INVOKABLE bool fileUrlExists(const QUrl& url) const { return QFileInfo(url.toLocalFile()).exists(); }
};

#endif // QMLUTILS_H
