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

#include "qmltabulardataparser.h"

#include "shared/loading/xlsxtabulardataparser.h"
#include "shared/loading/matlabfileparser.h"

#include "shared/utils/scope_exit.h"
#include "shared/utils/container.h"
#include "shared/utils/static_block.h"

#include <QFileInfo>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QQmlEngine>

#include <map>

QmlTabularDataHeaderModel::QmlTabularDataHeaderModel(const QmlTabularDataParser* parser,
    ValueType valueTypes, const QStringList& skip) :
    _parser(parser)
{
    _columnIndices.reserve(_parser->_dataPtr->numColumns());

    for(size_t column = 0; column < _parser->_dataPtr->numColumns(); column++)
    {
        auto type = _parser->_columnTypeIdentities.at(column).type();

        if(type == TypeIdentity::Type::String && !(valueTypes & ValueType::String))
            continue;

        if(type == TypeIdentity::Type::Int && !(valueTypes & ValueType::Int))
            continue;

        if(type == TypeIdentity::Type::Float && !(valueTypes & ValueType::Float))
            continue;

        const auto& value = _parser->_dataPtr->valueAt(column, 0);

        if(value.isEmpty())
            continue;

        if(!skip.isEmpty() && skip.contains(value))
            continue;

        _columnIndices.emplace_back(column);
    }
}

int QmlTabularDataHeaderModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_columnIndices.size());
}

QVariant QmlTabularDataHeaderModel::data(const QModelIndex& index, int role) const
{
    // Rows in the model correspond to columns in the data table
    auto columnIndicesIndex = static_cast<size_t>(index.row());

    if(columnIndicesIndex >= _columnIndices.size())
        return {};

    auto columnIndex = _columnIndices.at(columnIndicesIndex);

    if(role == Qt::DisplayRole)
        return _parser->_dataPtr->valueAt(columnIndex, 0);

    if(role == Roles::ColumnIndex)
        return {static_cast<int>(columnIndex)};

    return {};
}

QHash<int, QByteArray> QmlTabularDataHeaderModel::roleNames() const
{
    auto names = QAbstractItemModel::roleNames();

    names[Roles::ColumnIndex] = "columnIndex";

    return names;
}

int QmlTabularDataHeaderModel::columnIndexFor(const QModelIndex& index) const
{
    return data(index, Roles::ColumnIndex).toInt();
}

QModelIndex QmlTabularDataHeaderModel::indexOf(int columnIndex) const
{
    auto row = u::indexOf(_columnIndices, columnIndex);

    if(row < 0)
        return {};

    return index(row);
}

QmlTabularDataParser::QmlTabularDataParser() // NOLINT modernize-use-equals-default
{
    connect(&_dataParserWatcher, &QFutureWatcher<void>::started, this, &QmlTabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &QmlTabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &QmlTabularDataParser::onDataLoaded);

    setProgressFn([this](int progress)
    {
        if(progress != _progress)
        {
            _progress = progress;
            emit progressChanged();
        }
    });

    _dataPtr = std::make_shared<TabularData>();
}

QmlTabularDataParser::~QmlTabularDataParser() // NOLINT modernize-use-equals-default
{
    _dataParserWatcher.waitForFinished();
}

void QmlTabularDataParser::updateColumnTypes()
{
    _columnTypeIdentities = _dataPtr->typeIdentities(this);
}

bool QmlTabularDataParser::parse(const QUrl& fileUrl)
{
    reset();

    QFuture<void> future = QtConcurrent::run([this, fileUrl]
    {
        if(fileUrl.isEmpty())
            return;

        auto tryToParseUsing = [fileUrl, this](auto&& parser)
        {
            _dataPtr->reset();
            _cancellableParser = &parser;
            auto atExit = std::experimental::make_scope_exit([this] { _cancellableParser = nullptr; });

            if(!parser.canLoad(fileUrl))
                return false;

            parser.setProgressFn([this](int progress) { setProgress(progress); });

            if(!parser.parse(fileUrl))
                return false;

            _dataPtr = std::make_shared<TabularData>(std::move(parser.tabularData()));
            updateColumnTypes();
            emit dataChanged();

            return true;
        };

        std::map<QString, std::function<bool()>> parsers =
        {
            {QStringLiteral("csv"),     [&tryToParseUsing]{ return tryToParseUsing(CsvFileParser()); }},
            {QStringLiteral("tsv"),     [&tryToParseUsing]{ return tryToParseUsing(TsvFileParser()); }},
            {QStringLiteral("ssv"),     [&tryToParseUsing]{ return tryToParseUsing(SsvFileParser()); }},
            {QStringLiteral("xlsx"),    [&tryToParseUsing]{ return tryToParseUsing(XlsxTabularDataParser()); }},
            {QStringLiteral("mat"),     [&tryToParseUsing]{ return tryToParseUsing(MatLabFileParser()); }}
        };

        auto extension = QFileInfo(fileUrl.toLocalFile()).suffix();
        bool success = false;

        if(u::contains(parsers, extension))
        {
            const auto& tryParse = parsers.at(extension);
            success = tryParse();
        }

        if(!success)
        {
            // Try all the other parsers, in case the user used the wrong extension
            for(const auto& [parserExtension, tryParse] : parsers)
            {
                if(parserExtension == extension)
                    continue;

                success = tryParse();
                if(success)
                    break;
            }
        }

        setProgress(-1);
        auto result = onParseComplete();

        if(!result)
        {
            _dataPtr->reset();
            _failureReason = result._reason;
        }
    });

    _dataParserWatcher.setFuture(future);
    return true;
}

void QmlTabularDataParser::cancelParse()
{
    if(_cancellableParser != nullptr)
        _cancellableParser.load()->cancel();

    cancel();
}

void QmlTabularDataParser::reset()
{
    if(!_dataPtr->empty())
    {
        _dataPtr->reset();
        emit dataChanged();
    }

    if(_progress != -1)
    {
        _progress = -1;
        emit progressChanged();
    }

    if(_complete)
    {
        _complete = false;
        emit completeChanged();
    }

    if(_failed)
    {
        _failed = false;
        emit failedChanged();
    }
}

QmlTabularDataHeaderModel* QmlTabularDataParser::headers(int _valueTypes, const QStringList& skip) const
{
    Q_ASSERT(_dataPtr != nullptr);
    if(_dataPtr == nullptr)
        return {};

    auto valueTypes = static_cast<ValueType>(_valueTypes);

    // Caller takes ownership (usually QML/JS)
    return new QmlTabularDataHeaderModel(this, valueTypes, skip);
}

void QmlTabularDataParser::onDataLoaded()
{
    if(_dataPtr->empty())
    {
        if(!_failureReason.isEmpty())
            emit failureReasonChanged();

        _failed = true;
        emit failedChanged();
    }
    else
        emit dataLoaded();

    _complete = true;
    emit completeChanged();
}

static_block
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
