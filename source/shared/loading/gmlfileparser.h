#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "shared/loading/baseparser.h"

class UserNodeData;

class GmlFileParser: public BaseParser
{
private:
    UserNodeData* _userNodeData;

public:
    explicit GmlFileParser(UserNodeData* userNodeData = nullptr);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // GMLFILEPARSER_H
