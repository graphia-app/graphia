#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/cancellableparser.h"

class GmlFileParser: public IParser, public CancellableParser
{
public:
    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // GMLFILEPARSER_H
