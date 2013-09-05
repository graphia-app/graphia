#ifndef GMLFILEPARSER_H
#define GMLFILEPARSER_H

#include "graphfileparser.h"

#include <QObject>
#include <QString>

#include <cstdint>

#ifndef Q_MOC_RUN
#include <boost/variant/recursive_variant.hpp>
#include <boost/fusion/container/vector.hpp>
#endif

class GmlFileParser : public GraphFileParser
{
    Q_OBJECT
private:
    QString filename;
    int64_t fileSize;
    int percentage;

public:
    GmlFileParser(QString filename) : GraphFileParser(), filename(filename), fileSize(0), percentage(0) {}

    bool parse(Graph& graph);

public:
    struct KeyValuePairList;

    typedef boost::variant<int, double, QString, boost::recursive_wrapper<KeyValuePairList> > Value;
    typedef boost::fusion::vector<QString,Value> KeyValuePair;
    struct KeyValuePairList : std::vector<KeyValuePair> {};

private:
    enum class GmlList
    {
        File,
        Graph,
        Node,
        Edge
    };

    bool parseGmlList(Graph& graph, const KeyValuePairList& keyValuePairList, GmlList listType = GmlList::File);
    void onParsePositionIncremented(int64_t position);
};

#endif // GMLFILEPARSER_H
