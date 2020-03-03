#ifndef QMLUTILS_H
#define QMLUTILS_H

#include "shared/utils/thread.h"
#include "shared/utils/utils.h"
#include "shared/utils/string.h"
#include "shared/utils/color.h"

#include <QObject>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QCollator>

class QQmlEngine;
class QJSEngine;

class QmlUtils : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QmlUtils)

public:
    QmlUtils() = default;

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString baseFileNameForUrl(const QUrl& url) const { return url.fileName(); }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString baseFileNameForUrlNoExtension(const QUrl& url) const
    {
        auto fi = QFileInfo(url.toLocalFile());
        return fi.baseName();
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString fileNameForUrl(const QUrl& url) const { return url.toLocalFile(); }
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QUrl urlForFileName(const QString& fileName) const { return QUrl::fromLocalFile(fileName); }
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QUrl urlForUserInput(const QString& userInput) const { return QUrl::fromUserInput(userInput); }
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool fileExists(const QString& fileName) const { return QFileInfo::exists(fileName); }
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool fileUrlExists(const QUrl& url) const { return QFileInfo::exists(url.toLocalFile()); }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QUrl replaceExtension(const QUrl& url, const QString& extension) const
    {
        auto fi = QFileInfo(url.toLocalFile());
        auto replaced = QFileInfo(QStringLiteral("%1/%2.%3")
                                  .arg(fi.path(),
                                       fi.baseName(),
                                       extension));

        return QUrl::fromLocalFile(replaced.filePath());
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString currentThreadName() const { return u::currentThreadName(); }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE bool urlIsValid(const QString& urlString) const
    {
        QUrl url = QUrl::fromUserInput(urlString);
        return url.isValid();
    }

    // QML JS comparelocale doesn't include numeric implementation...
    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE int localeCompareStrings(const QString& left, const QString& right, bool numeric = true)
    {
        _collator.setNumericMode(numeric);
        return _collator.compare(left, right);
    }

    static QObject* qmlInstance(QQmlEngine*, QJSEngine*)
    {
        return new QmlUtils;
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString formatNumberScientific(double value)
    {
        return u::formatNumberScientific(value);
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QString formatNumberSIPostfix(double value)
    {
        return u::formatNumberSIPostfix(value);
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QColor contrastingColor(const QColor& color)
    {
        return u::contrastingColor(color);
    }

    // NOLINTNEXTLINE readability-convert-member-functions-to-static
    Q_INVOKABLE QColor colorForString(const QString& string)
    {
        return u::colorForString(string);
    }

private:
    QCollator _collator;
};

#endif // QMLUTILS_H
