/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "tabulardataparser.h"

#include "shared/loading/xlsxtabulardataparser.h"
#include "shared/loading/matlabfileparser.h"

#include "shared/utils/scope_exit.h"
#include "shared/utils/container.h"

#include <QFileInfo>
#include <QFuture>
#include <QtConcurrentRun>
#include <QQmlEngine>

#include <map>

using namespace Qt::Literals::StringLiterals;

TabularDataHeaderModel::TabularDataHeaderModel(const TabularDataParser* parser,
    ValueType valueTypes, const QStringList& skip, HeaderModelType headerModelType) :
    _parser(parser), _type(headerModelType)
{
    const size_t num = _type == HeaderModelType::Rows ?
        _parser->_dataPtr->numColumns() :
        _parser->_dataPtr->numRows();

    _indices.reserve(num);

    for(size_t index = 0; index < num; index++)
    {
        auto type = _type == HeaderModelType::Rows ?
            _parser->_columnTypeIdentities.at(index).type() :
            _parser->_rowTypeIdentities.at(index).type();

        if(type == TypeIdentity::Type::String && !(valueTypes & ValueType::String))
            continue;

        if(type == TypeIdentity::Type::Int && !(valueTypes & ValueType::Int))
            continue;

        if(type == TypeIdentity::Type::Float && !(valueTypes & ValueType::Float))
            continue;

        const auto& value = _type == HeaderModelType::Rows ?
            _parser->_dataPtr->valueAt(index, 0) :
            _parser->_dataPtr->valueAt(0, index);

        if(value.isEmpty())
            continue;

        if(!skip.isEmpty() && skip.contains(value))
            continue;

        _indices.emplace_back(index);
    }
}

int TabularDataHeaderModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_indices.size());
}

QVariant TabularDataHeaderModel::data(const QModelIndex& modelIndex, int role) const
{
    auto indicesIndex = static_cast<size_t>(modelIndex.row());

    if(indicesIndex >= _indices.size())
        return {};

    auto index = _indices.at(indicesIndex);

    if(role == Qt::DisplayRole)
    {
        return _type == HeaderModelType::Rows ?
            _parser->_dataPtr->valueAt(index, 0) :
            _parser->_dataPtr->valueAt(0, index);
    }

    if(role == Roles::Index)
        return {static_cast<int>(index)};

    return {};
}

QHash<int, QByteArray> TabularDataHeaderModel::roleNames() const
{
    auto names = QAbstractItemModel::roleNames();

    names[Roles::Index] = "indexRole";

    return names;
}

int TabularDataHeaderModel::indexFor(const QModelIndex& modelIndex) const
{
    return data(modelIndex, Roles::Index).toInt();
}

QModelIndex TabularDataHeaderModel::modelIndexOf(int roleIndex) const
{
    auto row = u::indexOf(_indices, roleIndex);

    if(row < 0)
        return {};

    return index(row);
}

TabularDataParser::TabularDataParser()
{
    connect(&_dataParserWatcher, &QFutureWatcher<void>::started, this, &TabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &TabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &TabularDataParser::onDataLoaded);

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

TabularDataParser::~TabularDataParser()
{
    _dataParserWatcher.waitForFinished();
}

void TabularDataParser::updateTypes()
{
    _columnTypeIdentities = _dataPtr->columnTypeIdentities(this);
    _rowTypeIdentities = _dataPtr->rowTypeIdentities(this);
}

bool TabularDataParser::parse(const QUrl& fileUrl)
{
    reset();

    const QFuture<void> future = QtConcurrent::run([this, fileUrl]
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
            {
                _failureReason = parser.failureReason();
                return false;
            }

            _dataPtr = std::make_shared<TabularData>(std::move(parser.tabularData()));
            updateTypes();
            emit dataChanged();

            return true;
        };

        std::map<QString, std::function<bool()>> parsers =
        {
            {u"csv"_s,     [&tryToParseUsing]{ return tryToParseUsing(CsvFileParser()); }},
            {u"tsv"_s,     [&tryToParseUsing]{ return tryToParseUsing(TsvFileParser()); }},
            {u"ssv"_s,     [&tryToParseUsing]{ return tryToParseUsing(SsvFileParser()); }},
            {u"xlsx"_s,    [&tryToParseUsing]{ return tryToParseUsing(XlsxTabularDataParser()); }},
            {u"mat"_s,     [&tryToParseUsing]{ return tryToParseUsing(MatLabFileParser()); }},
            {u"txt"_s,     [&tryToParseUsing]{ return tryToParseUsing(TxtFileParser()); }}
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

        if(!success)
        {
            _dataPtr->reset();

            if(_failureReason.isEmpty())
            {
                _failureReason = tr("Cannot identify tabular file type. "
                    "Please ensure the file contains a consistent number of columns.");
            }
        }
        else
        {
            auto result = onParseComplete();

            if(!result)
            {
                _dataPtr->reset();
                _failureReason = result._reason;
            }
        }
    });

    _dataParserWatcher.setFuture(future);
    return true;
}

void TabularDataParser::cancelParse()
{
    if(_cancellableParser != nullptr)
        _cancellableParser.load()->cancel();

    cancel();
}

void TabularDataParser::reset()
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

TabularDataHeaderModel* TabularDataParser::rowHeaders(int _valueTypes, const QStringList& skip) const
{
    Q_ASSERT(_dataPtr != nullptr);
    if(_dataPtr == nullptr)
        return {};

    auto valueTypes = static_cast<ValueType>(_valueTypes);

    // Caller takes ownership (usually QML/JS)
    return new TabularDataHeaderModel(this, valueTypes, skip, HeaderModelType::Rows);
}

TabularDataHeaderModel* TabularDataParser::columnHeaders(int _valueTypes, const QStringList& skip) const
{
    Q_ASSERT(_dataPtr != nullptr);
    if(_dataPtr == nullptr)
        return {};

    auto valueTypes = static_cast<ValueType>(_valueTypes);

    // Caller takes ownership (usually QML/JS)
    return new TabularDataHeaderModel(this, valueTypes, skip, HeaderModelType::Columns);
}

void TabularDataParser::onDataLoaded()
{
    if(_dataPtr->empty())
    {
        if(!_failureReason.isEmpty())
            emit failureReasonChanged();

        _failed = true;
        emit failedChanged();
    }
    else
    {
        _model.setTabularData(*_dataPtr);
        emit dataLoaded();
    }

    _complete = true;
    emit completeChanged();
}
