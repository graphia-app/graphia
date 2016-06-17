#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/baseparser.h"

class IGenericPluginInstance;

class PairwiseTxtFileParser : public BaseParser
{
private:
    IGenericPluginInstance* _genericPluginInstance;

public:
    PairwiseTxtFileParser(IGenericPluginInstance* genericPluginInstance);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // PAIRWISETXTFILEPARSER_H
