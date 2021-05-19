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

#include "xlsxtabulardataparser.h"

#include <xlsxio/include/xlsxio_read.h>

XlsxTabularDataParser::XlsxTabularDataParser(IParser* parent)
{
    if(parent != nullptr)
        setProgressFn([parent](int percent) { parent->setProgress(percent); });
}

static int cellCallback(size_t row, size_t column, const XLSXIOCHAR* value, void* cbData)
{
    if(value == nullptr)
        return 0;

    auto* xlsxTabularDataParser = reinterpret_cast<XlsxTabularDataParser*>(cbData);

    auto rowLimit = xlsxTabularDataParser->rowLimit();

    if(rowLimit > 0 && (row - 1) > rowLimit)
        return 1;

    if(xlsxTabularDataParser->cancelled())
        return 1;

    // XLSXIO is some kind of deviant library that indexes from 1
    xlsxTabularDataParser->tabularData().setValueAt(column - 1, row - 1, value);

    return 0;
}

bool XlsxTabularDataParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    if(graphModel != nullptr)
        graphModel->mutableGraph().setPhase(QObject::tr("Parsing"));

    xlsxioreader xlsxioread = nullptr;

    auto filename = url.toLocalFile().toUtf8();
    if((xlsxioread = xlsxioread_open(filename.constData())) == nullptr)
        return false;

    xlsxioread_process(xlsxioread, nullptr, XLSXIOREAD_SKIP_NONE,
        cellCallback, nullptr, this);
    xlsxioread_close(xlsxioread);

    // Free up any over-allocation
    _tabularData.shrinkToFit();

    return !cancelled();
}

bool XlsxTabularDataParser::canLoad(const QUrl& url)
{
    xlsxioreader xlsxioread = nullptr;

    auto filename = url.toLocalFile().toUtf8();
    if((xlsxioread = xlsxioread_open(filename.constData())) == nullptr)
        return false;

    xlsxioread_close(xlsxioread);
    return true;
}
