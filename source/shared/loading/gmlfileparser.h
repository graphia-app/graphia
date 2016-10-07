#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "shared/loading/baseparser.h"

class NodeAttributes;

class GmlFileParser: public BaseParser
{
private:
    NodeAttributes* _nodeAttributes;

public:
    explicit GmlFileParser(NodeAttributes* nodeAttributes = nullptr);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // GMLFILEPARSER_H
