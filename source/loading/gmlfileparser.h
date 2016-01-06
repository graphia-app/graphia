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
    QString _filename;
    int64_t _fileSize = 0;
    int _percentage = 0;

public:
    explicit GmlFileParser(const QString& filename) :
        GraphFileParser(),
        _filename(filename)
    {}

    bool parse(MutableGraph& graph);

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

    bool parseGmlList(MutableGraph& graph, const KeyValuePairList& keyValuePairList, GmlList listType = GmlList::File);
    void onParsePositionIncremented(int64_t position);
};

#endif // GMLFILEPARSER_H
