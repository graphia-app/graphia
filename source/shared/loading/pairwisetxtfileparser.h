#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/userelementdata.h"

class PairwiseTxtFileParser : public IParser
{
private:
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;

public:
    explicit PairwiseTxtFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData);

    bool parse(const QUrl& url, IGraphModel* graphModel) override;

    static bool canLoad(const QUrl&) { return true; }
};

#endif // PAIRWISETXTFILEPARSER_H
