#ifndef GRAPHFILEPARSER_H
#define GRAPHFILEPARSER_H

#include "../graph/graph.h"

class GraphFileParser
{
public:
    virtual bool parse(Graph& graph) = 0;
};

#endif // GRAPHFILEPARSER_H
