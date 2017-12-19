#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/userelementdata.h"

class BaseGenericPluginInstance;

class PairwiseTxtFileParser : public IParser
{
private:
    BaseGenericPluginInstance* _genericPluginInstance;
    UserNodeData* _userNodeData;

public:
    explicit PairwiseTxtFileParser(BaseGenericPluginInstance* genericPluginInstance,
                                   UserNodeData* userNodeData = nullptr);

    bool parse(const QUrl& url, IGraphModel& graphModel, const ProgressFn& progressFn) override;
};

#endif // PAIRWISETXTFILEPARSER_H
