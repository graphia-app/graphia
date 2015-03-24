#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "graphfileparser.h"

class PairwiseTxtFileParser : public GraphFileParser
{
    Q_OBJECT
private:
    QString _filename;

public:
    PairwiseTxtFileParser(const QString& filename) :
        GraphFileParser(),
        _filename(filename)
    {}

    bool parse(Graph& graph);
};

#endif // PAIRWISETXTFILEPARSER_H
