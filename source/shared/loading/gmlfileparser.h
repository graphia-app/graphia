#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "shared/loading/iparser.h"

class UserNodeData;

class GmlFileParser: public IParser
{
private:
    UserNodeData* _userNodeData;

public:
    explicit GmlFileParser(UserNodeData* userNodeData = nullptr);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // GMLFILEPARSER_H
