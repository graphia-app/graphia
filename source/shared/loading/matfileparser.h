#ifndef MATFILEPARSER_H
#define MATFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/userelementdata.h"

class MatFileParser : public IParser
{
private:
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;

public:
    MatFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {}

    bool parse(const QUrl &url, IGraphModel *graphModel) override;
    static bool canLoad(const QUrl&) { return true; }
};

#endif // MATFILEPARSER_H
