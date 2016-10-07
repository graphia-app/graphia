#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/baseparser.h"

class BaseGenericPluginInstance;
class NodeAttributes;

class PairwiseTxtFileParser : public BaseParser
{
private:
    BaseGenericPluginInstance* _genericPluginInstance;
    NodeAttributes* _nodeAttributes;

public:
    explicit PairwiseTxtFileParser(BaseGenericPluginInstance* genericPluginInstance,
                                   NodeAttributes* nodeAttributes = nullptr);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // PAIRWISETXTFILEPARSER_H
