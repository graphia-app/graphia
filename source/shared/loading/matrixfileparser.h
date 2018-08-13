#ifndef MATRIXFILEPARSER_H
#define MATRIXFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"
#include "shared/plugins/userelementdata.h"

class MatrixFileParser : public IParser
{
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;
public:
    MatrixFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData);

public:
    bool parse(const QUrl &url, IGraphModel &graphModel, const ProgressFn &progressFn);
};

#endif // MATRIXFILEPARSER_H
