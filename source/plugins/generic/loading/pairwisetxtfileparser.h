#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/cancellableparser.h"

class IGraphModel;
class GenericPlugin;

class PairwiseTxtFileParser : public IParser, public CancellableParser
{
private:
    IGraphModel* _graphModel;
    GenericPlugin* _genericPlugin;

public:
    PairwiseTxtFileParser(GenericPlugin* genericPlugin);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // PAIRWISETXTFILEPARSER_H
