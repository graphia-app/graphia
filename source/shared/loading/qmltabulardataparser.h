/* Copyright © 2013-2023 Graphia Technologies Ltd.
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
#include "shared/loading/tabulardatamodel.h"
#include "shared/attributes/valuetype.h"

#include <QObject>
#include <QFutureWatcher>
#include <QAbstractListModel>
#include <QVariantMap>

#include <memory>
#include <vector>
#include <atomic>

class QmlTabularDataParser;
class QAbstractTableModel;

DEFINE_QML_ENUM(
    Q_GADGET, HeaderModelType,
    Rows, Columns);

class QmlTabularDataHeaderModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit QmlTabularDataHeaderModel(const QmlTabularDataParser* parser,
        ValueType valueTypes = ValueType::All, const QStringList& skip = {},
        HeaderModelType headerModelType = HeaderModelType::Rows);

    enum Roles
    {
        Index = Qt::UserRole + 1,
    };

    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& modelIndex, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int indexFor(const QModelIndex& modelIndex) const;
    Q_INVOKABLE QModelIndex modelIndexOf(int roleIndex) const;

private:
    const QmlTabularDataParser* _parser = nullptr;
    HeaderModelType _type = HeaderModelType::Rows;
    std::vector<size_t> _indices;
};

class QmlTabularDataParser : public QObject, virtual public Cancellable, virtual public Progressable
{
    friend class QmlTabularDataHeaderModel;

    Q_OBJECT

    Q_PROPERTY(std::shared_ptr<TabularData> data MEMBER _dataPtr NOTIFY dataChanged)
    Q_PROPERTY(QAbstractTableModel* model READ tableModel NOTIFY dataLoaded)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(int progress MEMBER _progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(bool complete MEMBER _complete NOTIFY completeChanged)
    Q_PROPERTY(bool failed MEMBER _failed NOTIFY failedChanged)
    Q_PROPERTY(QString failureReason MEMBER _failureReason NOTIFY failureReasonChanged)

private:
    QFutureWatcher<void> _dataParserWatcher;
    std::shared_ptr<TabularData> _dataPtr = nullptr;
    TabularDataModel _model;

    std::vector<TypeIdentity> _columnTypeIdentities;
    std::vector<TypeIdentity> _rowTypeIdentities;

    int _progress = -1;
    std::atomic<Cancellable*> _cancellableParser = nullptr;
    bool _complete = false;
    bool _failed = false;
    QString _failureReason;

    void updateTypes();

public:
    struct MatrixTypeResult
    {
        bool _result = true;
        QString _reason;

        explicit operator bool() const { return _result; }
    };

protected:
    const TabularData& tabularData() const { return *_dataPtr; }

    // This is called from a worker thread
    virtual MatrixTypeResult onParseComplete() { return {}; }

public:
    QmlTabularDataParser();
    ~QmlTabularDataParser() override;

    Q_INVOKABLE bool parse(const QUrl& fileUrl);
    Q_INVOKABLE void cancelParse();
    Q_INVOKABLE void reset();

    Q_INVOKABLE QmlTabularDataHeaderModel* rowHeaders(
        int _valueTypes = static_cast<int>(ValueType::All),
        const QStringList& skip = {}) const;

    Q_INVOKABLE QmlTabularDataHeaderModel* columnHeaders(
        int _valueTypes = static_cast<int>(ValueType::All),
        const QStringList& skip = {}) const;

    bool busy() const { return _dataParserWatcher.isRunning(); }

    QAbstractTableModel* tableModel() { return &_model; }

signals:
    void dataChanged();

    void busyChanged();
    void progressChanged();
    void completeChanged();
    void failedChanged();
    void failureReasonChanged();

    void dataLoaded();

private slots:
    void onDataLoaded();
};

#endif // QMLTABULARDATAPARSER_H
