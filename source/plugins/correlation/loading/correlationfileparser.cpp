/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "correlationfileparser.h"

#include "correlationplugin.h"

#include "shared/graph/igraphmodel.h"

#include "shared/loading/xlsxtabulardataparser.h"

#include <QObject>

#include <utility>

using namespace Qt::Literals::StringLiterals;

CorrelationFileParser::CorrelationFileParser(CorrelationPluginInstance* plugin, const QString& urlTypeName,
                                             TabularData& tabularData, QRect dataRect) :
    _plugin(plugin), _urlTypeName(urlTypeName),
    _tabularData(std::move(tabularData)), _dataRect(dataRect)
{}

template<typename ParseFn>
static bool parseUsing(const QString& fileType, const ParseFn& parseFn)
{
    if(fileType == u"CorrelationCSV"_s)
        return parseFn(CsvFileParser());

    if(fileType == u"CorrelationTSV"_s)
        return parseFn(TsvFileParser());

    if(fileType == u"CorrelationSSV"_s)
        return parseFn(SsvFileParser());

    if(fileType == u"CorrelationXLSX"_s)
        return parseFn(XlsxTabularDataParser());

    return false;
}

bool CorrelationFileParser::parse(const QUrl& fileUrl, IGraphModel*)
{
    if(_tabularData.empty())
    {
        const bool success = parseUsing(_urlTypeName, [this, &fileUrl](auto&& parser)
        {
            parser.setProgressFn([this](int progress) { setProgress(progress); });

            if(!parser.parse(fileUrl))
            {
                setFailureReason(parser.failureReason());
                return false;
            }

            _tabularData = std::move(parser.tabularData());

            return true;
        });

        if(!success)
            return false;
    }

    if(_tabularData.empty() || cancelled())
        return false;

    _tabularData.setTransposed(_plugin->transpose());

    if(_dataRect.isEmpty())
    {
        setProgress(-1);
        _dataRect = _tabularData.findLargestNumericalDataRect(this);

        if(_dataRect.isEmpty())
            _dataRect = _tabularData.findLargestNonNumericalDataRect(this);
    }

    if(_dataRect.isEmpty() || cancelled())
        return false;

    size_t numContinuousColumns = 0;
    size_t numDiscreteColumns = 0;

    switch(_plugin->dataType())
    {
    default:
    case CorrelationDataType::Continuous:
        numContinuousColumns = static_cast<size_t>(_dataRect.width());
        break;

    case CorrelationDataType::Discrete:
        numDiscreteColumns = static_cast<size_t>(_dataRect.width());
        break;
    }

    _plugin->setDimensions(numContinuousColumns, numDiscreteColumns, static_cast<size_t>(_dataRect.height()));

    setPhase(QObject::tr("Attributes"));
    if(!_plugin->loadUserData(_tabularData, _dataRect, *this))
        return false;

    // We don't need this any more, so free up any memory it's consuming
    _tabularData.reset();

    setProgress(-1);
    _plugin->finishDataRows();

    if(_plugin->requiresNormalisation())
    {
        setPhase(QObject::tr("Normalisation"));
        if(!_plugin->normalise(this))
            return false;
    }

    if(cancelled())
        return false;

    setProgress(-1);

    setPhase(QObject::tr("Correlation"));
    auto edges = _plugin->correlation(*this);

    if(cancelled())
        return false;

    _plugin->createAttributes();

    setPhase(QObject::tr("Building Graph"));
    if(!_plugin->createEdges(edges, *this))
        return false;

    clearPhase();

    return true;
}

QString CorrelationFileParser::log() const
{
    return _plugin->log();
}
