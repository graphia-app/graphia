#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/baseparser.h"

class BaseGenericPluginInstance;
class UserNodeData;

class PairwiseTxtFileParser : public BaseParser
{
private:
    BaseGenericPluginInstance* _genericPluginInstance;
    UserNodeData* _userNodeData;

public:
    explicit PairwiseTxtFileParser(BaseGenericPluginInstance* genericPluginInstance,
                                   UserNodeData* userNodeData = nullptr);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // PAIRWISETXTFILEPARSER_H
