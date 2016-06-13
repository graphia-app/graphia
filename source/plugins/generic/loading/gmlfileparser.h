#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "shared/loading/baseparser.h"

class GmlFileParser: public BaseParser
{
public:
    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // GMLFILEPARSER_H
