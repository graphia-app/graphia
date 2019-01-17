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

    xlsxioreader xlsxioread;

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
    xlsxioreader xlsxioread;

    auto filename = url.toLocalFile().toUtf8();
    if((xlsxioread = xlsxioread_open(filename.constData())) == nullptr)
        return false;

    xlsxioread_close(xlsxioread);
    return true;
}
