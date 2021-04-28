#ifndef DOTFILEPARSER_H
#define DOTFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/iuserelementdata.h"

class DotFileParser : public IParser
{
private:
    IUserNodeData* _userNodeData;
    IUserEdgeData* _userEdgeData;

public:
    explicit DotFileParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData);

    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    static bool canLoad(const QUrl&) { return true; }
};

#endif // DOTFILEPARSER_H
