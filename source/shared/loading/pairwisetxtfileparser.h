#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/iparser.h"

class BaseGenericPluginInstance;
class UserNodeData;

class PairwiseTxtFileParser : public IParser
{
private:
    BaseGenericPluginInstance* _genericPluginInstance;
    UserNodeData* _userNodeData;

public:
    explicit PairwiseTxtFileParser(BaseGenericPluginInstance* genericPluginInstance,
                                   UserNodeData* userNodeData = nullptr);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progressFn) override;
};

#endif // PAIRWISETXTFILEPARSER_H
