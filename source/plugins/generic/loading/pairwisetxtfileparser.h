#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/baseparser.h"

class GenericPluginInstance;

class PairwiseTxtFileParser : public BaseParser
{
private:
    GenericPluginInstance* _genericPluginInstance;

public:
    PairwiseTxtFileParser(GenericPluginInstance* genericPluginInstance);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // PAIRWISETXTFILEPARSER_H
