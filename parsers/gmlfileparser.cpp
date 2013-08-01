#include "gmlfileparser.h"

#include <vector>
#include <string>
#include <fstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/qi_parse_auto.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/spirit/include/qi_real.hpp>

namespace fusion = boost::fusion;
namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace boost { namespace spirit { namespace traits
{
    // Make Qi recognize QString as a container
    template <> struct is_container<QString> : mpl::true_ {};

    // Expose the container's (QString's) value_type
    template <> struct container_value<QString> : mpl::identity<QChar> {};

    // Define how to insert a new element at the end of the container (QString)
    template <>
    struct push_back_container<QString, QChar>
    {
        static bool call(QString& c, QChar const& val)
        {
            c.append(val);
            return true;
        }
    };

    // The following specializations are required for debugging to work
    template <>
    struct is_empty_container<QString>
    {
        static bool call(QString const& c)
        {
            return c.isEmpty();
        }
    };

    template <typename Out, typename Enable>
    struct print_attribute_debug<Out, QString, Enable>
    {
        static void call(Out& out, QString const& val)
        {
            out << val.toStdString();
        }
    };
}}}

struct KeyValuePairList;

typedef boost::variant<int, double, QString, boost::recursive_wrapper<KeyValuePairList> > Value;
typedef fusion::vector<QString,Value> KeyValuePair;
struct KeyValuePairList : std::vector<KeyValuePair> {};

template <typename Iterator>
struct GmlGrammar : qi::grammar<Iterator, KeyValuePairList(), ascii::space_type>
{
    GmlGrammar() : GmlGrammar::base_type(list)
    {
        using qi::char_;
        using qi::int_;
        qi::real_parser<double, qi::strict_real_policies<double> > strict_double;
        using qi::lexeme;

        string %= lexeme['"' >> (*(char_ - char_("&\"")) | ('&' >> +char_ >> ';')) >> '"'];

        value %= int_ | strict_double | string | '[' >> list >> ']';
        key %= lexeme[char_("a-zA-Z") >> *(char_("a-zA-Z0-9"))];
        list %= *(key >> value);

        //debug(list);
    }

    qi::rule<Iterator, KeyValuePairList(), ascii::space_type> list;
    qi::rule<Iterator, Value(), ascii::space_type> value;
    qi::rule<Iterator, QString(), ascii::space_type> key;
    qi::rule<Iterator, QString(), ascii::space_type> string;
};

enum class GmlList
{
    File,
    Graph,
    Node,
    Edge
};

static bool parseGmlList(Graph& graph, const KeyValuePairList& keyValuePairList, GmlList listType = GmlList::File)
{
    static QMap<int,Graph::NodeId> nodeIdMap;
    Graph::NodeId sourceId = Graph::NullNodeId;

    for(const KeyValuePair keyValuePair : keyValuePairList)
    {
        QString key = fusion::at_c<0>(keyValuePair);
        Value value = fusion::at_c<1>(keyValuePair);
        KeyValuePairList* subKeyValuePairList = boost::get<KeyValuePairList>(&value);
        int* intValue = boost::get<int>(&value);

        switch(listType)
        {
        case GmlList::File:
            nodeIdMap.clear();
            if(!key.compare("graph") && subKeyValuePairList != 0)
                return parseGmlList(graph, *subKeyValuePairList, GmlList::Graph);
            break;

        case GmlList::Graph:
            if(subKeyValuePairList != 0)
            {
                bool result = false;

                if(!key.compare("node"))
                    result = parseGmlList(graph, *subKeyValuePairList, GmlList::Node);
                else if(!key.compare("edge"))
                    result = parseGmlList(graph, *subKeyValuePairList, GmlList::Edge);

                if(!result)
                    return false;
            }
            break;

        case GmlList::Node:
            if(intValue != 0)
            {
                if(!key.compare("id"))
                    nodeIdMap.insert(*intValue, graph.addNode());
                else
                    return false;
            }
            break;

        case GmlList::Edge:
            if(intValue != 0)
            {
                if(!key.compare("source") && nodeIdMap.contains(*intValue))
                    sourceId = nodeIdMap[*intValue];
                else if(!key.compare("target") && sourceId != Graph::NullNodeId && nodeIdMap.contains(*intValue))
                    graph.addEdge(sourceId, nodeIdMap[*intValue]);
                else
                    return false;
            }
            break;
        }
    }

    return true;
}

bool GmlFileParser::parse(Graph& graph)
{
    std::ifstream stream(filename.toStdString());
    stream.unsetf(std::ios::skipws);

    spirit::istream_iterator begin(stream);
    spirit::istream_iterator end;

    GmlGrammar<spirit::istream_iterator> grammar;
    KeyValuePairList keyValuePairList;

    if(qi::phrase_parse(begin, end, grammar, boost::spirit::ascii::space, keyValuePairList))
        return parseGmlList(graph, keyValuePairList);

    return false;
}
