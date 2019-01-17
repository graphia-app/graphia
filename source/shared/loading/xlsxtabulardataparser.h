#ifndef XLSXTABULARDATAPARSER_H
#define XLSXTABULARDATAPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"

#include <QUrl>

class XlsxTabularDataParser : public IParser
{
private:
    TabularData _tabularData;

public:
    explicit XlsxTabularDataParser(IParser* parent = nullptr);

    bool parse(const QUrl& url, IGraphModel* graphModel = nullptr) override;

    TabularData& tabularData() { return _tabularData; }

    static bool canLoad(const QUrl& url);
};

#endif // XLSXTABULARDATAPARSER_H
