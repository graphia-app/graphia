#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/userelementdata.h"

class GmlFileParser: public IParser
{
private:
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;

public:
    explicit GmlFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData);

    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    static bool isType(const QUrl&) { return true; }
};

#endif // GMLFILEPARSER_H
