#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "graphfileparser.h"


class GmlFileParser: public GraphFileParser
{
    Q_OBJECT
public:
    explicit GmlFileParser(const QString& filename) :
        GraphFileParser(),
        _filename(filename)
    {}

    bool parse(MutableGraph& graph);

private:
    QString _filename;
    int _percentage = 0;

    template<class It> bool parse(MutableGraph &graph, It begin, It end);
};

#endif // GMLFILEPARSER_H
