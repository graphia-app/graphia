#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "shared/loading/baseparser.h"

class BaseGenericPluginInstance;

class GmlFileParser: public BaseParser
{
private:
    BaseGenericPluginInstance* _genericPluginInstance;

public:
    explicit GmlFileParser(BaseGenericPluginInstance* genericPluginInstance);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // GMLFILEPARSER_H
