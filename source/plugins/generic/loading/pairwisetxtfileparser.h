#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/cancellableparser.h"

class GenericPluginInstance;

class PairwiseTxtFileParser : public IParser, public CancellableParser
{
private:
    GenericPluginInstance* _genericPluginInstance;

public:
    PairwiseTxtFileParser(GenericPluginInstance* genericPluginInstance);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // PAIRWISETXTFILEPARSER_H
