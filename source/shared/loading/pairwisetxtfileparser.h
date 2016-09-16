#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/baseparser.h"

class BaseGenericPluginInstance;

class PairwiseTxtFileParser : public BaseParser
{
private:
    BaseGenericPluginInstance* _genericPluginInstance;

public:
    explicit PairwiseTxtFileParser(BaseGenericPluginInstance* genericPluginInstance);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // PAIRWISETXTFILEPARSER_H
