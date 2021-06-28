/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef QMLTABULARDATAPARSER_H
#define QMLTABULARDATAPARSER_H

#include "shared/utils/cancellable.h"
#include "shared/utils/progressable.h"
#include "shared/utils/typeidentity.h"
#include "shared/loading/tabulardata.h"
#include "shared/attributes/valuetype.h"

#include <QObject>
#include <QFutureWatcher>
#include <QQmlEngine>
#include <QCoreApplication>
#include <QAbstractListModel>
#include <QVariantMap>

#include <memory>
#include <vector>
#include <atomic>

class QmlTabularDataParser;

class QmlTabularDataHeaderModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit QmlTabularDataHeaderModel(const QmlTabularDataParser* parser,
        ValueType valueTypes = ValueType::All, const QStringList& skip = {});

    enum Roles
    {
        ColumnIndex = Qt::UserRole + 1,
    };

    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int columnIndexFor(const QModelIndex& index) const;
    Q_INVOKABLE QModelIndex indexOf(int columnIndex) const;

private:
    const QmlTabularDataParser* _parser = nullptr;
    std::vector<size_t> _columnIndices;
};

class QmlTabularDataParser : public QObject, virtual public Cancellable, virtual public Progressable
{
    friend class QmlTabularDataHeaderModel;

    Q_OBJECT

    Q_PROPERTY(std::shared_ptr<TabularData> data MEMBER _dataPtr NOTIFY dataChanged)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(int progress MEMBER _progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(bool complete MEMBER _complete NOTIFY completeChanged)
    Q_PROPERTY(bool failed MEMBER _failed NOTIFY failedChanged)

private:
    QFutureWatcher<void> _dataParserWatcher;
    std::shared_ptr<TabularData> _dataPtr = nullptr;

    std::vector<TypeIdentity> _columnTypeIdentities;

    int _progress = -1;
    std::atomic<Cancellable*> _cancellableParser = nullptr;
    bool _complete = false;
    bool _failed = false;

    void updateColumnTypes();

protected:
    const TabularData& tabularData() const { return *_dataPtr; }

    // This is called from a worker thread
    virtual bool onParseComplete() { return true; }

public:
    QmlTabularDataParser();
    ~QmlTabularDataParser() override;

    Q_INVOKABLE bool parse(const QUrl& fileUrl);
    Q_INVOKABLE void cancelParse();
    Q_INVOKABLE void reset();

    Q_INVOKABLE QmlTabularDataHeaderModel* headers(
        int _valueTypes = static_cast<int>(ValueType::All),
        const QStringList& skip = {}) const;

    bool busy() const { return _dataParserWatcher.isRunning(); }

    static void registerQmlType()
    {
        static bool initialised = false;
        if(initialised)
            return;
        initialised = true;
        qmlRegisterType<QmlTabularDataParser>(APP_URI, APP_MAJOR_VERSION,
            APP_MINOR_VERSION, "TabularDataParser");

        qmlRegisterInterface<QmlTabularDataHeaderModel>("TabularDataHeaderModel"
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            , APP_MAJOR_VERSION
#endif
            );
    }

signals:
    void dataChanged();

    void busyChanged();
    void progressChanged();
    void completeChanged();
    void failedChanged();

    void dataLoaded();

private slots:
    void onDataLoaded();
};

static void tabularDataParserInitialiser()
{
    QmlTabularDataParser::registerQmlType();
}

Q_COREAPP_STARTUP_FUNCTION(tabularDataParserInitialiser)

#endif // QMLTABULARDATAPARSER_H
