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

#ifndef XLSXTABULARDATAPARSER_H
#define XLSXTABULARDATAPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"

#include <QUrl>

class XlsxTabularDataParser : public IParser
{
private:
    size_t _rowLimit = 0;
    TabularData _tabularData;

public:
    explicit XlsxTabularDataParser(IParser* parent = nullptr);

    bool parse(const QUrl& url, IGraphModel* graphModel = nullptr) override;

    size_t rowLimit() const { return _rowLimit; }
    void setRowLimit(size_t rowLimit) { _rowLimit = rowLimit; }

    TabularData& tabularData() { return _tabularData; }

    static bool canLoad(const QUrl& url);
};

#endif // XLSXTABULARDATAPARSER_H
