#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "graphfileparser.h"

#include <QString>

class GmlFileParser : public GraphFileParser
{
private:
    QString filename;

public:
    GmlFileParser(QString filename) : filename(filename) {}

    bool parse(Graph& graph);
};

#endif // GMLFILEPARSER_H
