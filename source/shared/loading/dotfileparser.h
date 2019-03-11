#ifndef DOTFILEPARSER_H
#define DOTFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/userelementdata.h"

class DotFileParser : public IParser
{
private:
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;

public:
    explicit DotFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData);

    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    static bool canLoad(const QUrl&) { return true; }
};

#endif // DOTFILEPARSER_H
