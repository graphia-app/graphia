#ifndef CORRELATIONFILEPARSER_H
#define CORRELATIONFILEPARSER_H

#include "shared/loading/baseparser.h"

class CorrelationPluginInstance;

class CorrelationFileParser : public BaseParser
{
private:
    CorrelationPluginInstance* _correlationPluginInstance;

public:
    CorrelationFileParser(CorrelationPluginInstance* correlationPluginInstance);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // CORRELATIONFILEPARSER_H
