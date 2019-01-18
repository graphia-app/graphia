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
