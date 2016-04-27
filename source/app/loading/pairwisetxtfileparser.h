#ifndef PAIRWISETXTFILEPARSER_H
#define PAIRWISETXTFILEPARSER_H

#include "graphfileparser.h"

#include <memory>

class WeightedEdgeGraphModel;

class PairwiseTxtFileParser : public GraphFileParser
{
    Q_OBJECT
private:
    QString _filename;
    std::shared_ptr<WeightedEdgeGraphModel> _graphModel;

public:
    PairwiseTxtFileParser(const QString& filename, std::shared_ptr<WeightedEdgeGraphModel> graphModel) :
        GraphFileParser(),
        _filename(filename),
        _graphModel(graphModel)
    {}

    bool parse(MutableGraph& graph);
};

#endif // PAIRWISETXTFILEPARSER_H
