#ifndef JSONGRAPHPARSER_H
#define JSONGRAPHPARSER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/userelementdata.h"
#include "json_helper.h"

class JsonGraphParser : public IParser
{
private:
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;
public:
    JsonGraphParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {}

    bool parse(const QUrl &url, IGraphModel *graphModel) override;
    static bool canLoad(const QUrl &) { return true; }
    static bool parseGraphObject(const json& jsonGraphObject, IGraphModel *graphModel,
                                 Progressable& progressable, bool useElementIdsLiterally = false,
                                 UserNodeData* userNodeData = nullptr,
                                 UserEdgeData* userEdgeData = nullptr);
};

#endif // JSONGRAPHPARSER_H
