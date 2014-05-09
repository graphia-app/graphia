#include "gmlfileparser.h"

#include <QFileInfo>

#include <vector>
#include <string>
#include <fstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/qi_parse_auto.hpp>
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

template <typename Iterator>
struct GmlGrammar : qi::grammar<Iterator, GmlFileParser::KeyValuePairList(), ascii::space_type>
{
    GmlGrammar() : GmlGrammar::base_type(list)
    {
        using qi::char_;
        using qi::int_;
        qi::real_parser<double, qi::strict_real_policies<double> > strict_double;
        using qi::lexeme;

        instring %= lexeme["" >> (char_ - char_("&\"")) | '&' >> *(char_ - char_(";")) >> ';'];
        string %= lexeme['"' >> *instring >> '"'];

        value %= strict_double | int_ | string | '[' >> list >> ']';
        key %= lexeme[char_("a-zA-Z") >> *(char_("a-zA-Z0-9"))];
        list %= *(key >> value);

        //debug(list);
    }

    qi::rule<Iterator, GmlFileParser::KeyValuePairList(), ascii::space_type> list;
    qi::rule<Iterator, GmlFileParser::Value(), ascii::space_type> value;
    qi::rule<Iterator, QString(), ascii::space_type> key;
    qi::rule<Iterator, QString(), ascii::space_type> string;
    qi::rule<Iterator, QString()> instring;
};

bool GmlFileParser::parseGmlList(Graph& graph, const GmlFileParser::KeyValuePairList& keyValuePairList, GmlList listType)
{
    static QMap<int,NodeId> nodeIdMap;
    NodeId sourceId;

    for(unsigned int i = 0; i < keyValuePairList.size(); i++)
    {
        if(cancelled())
            return false;

        const KeyValuePair& keyValuePair = keyValuePairList[i];
        QString key = fusion::at_c<0>(keyValuePair);
        Value value = fusion::at_c<1>(keyValuePair);
        KeyValuePairList* subKeyValuePairList = boost::get<KeyValuePairList>(&value);
        int* intValue = boost::get<int>(&value);

        switch(listType)
        {
        case GmlList::File:
            nodeIdMap.clear();
            if(!key.compare("graph") && subKeyValuePairList != nullptr)
            {
                bool result = parseGmlList(graph, *subKeyValuePairList, GmlList::Graph);
                emit complete(result);
                return result;
            }
            break;

        case GmlList::Graph:
        {
            if(subKeyValuePairList != nullptr)
            {
                bool result = false;

                if(!key.compare("node"))
                    result = parseGmlList(graph, *subKeyValuePairList, GmlList::Node);
                else if(!key.compare("edge"))
                    result = parseGmlList(graph, *subKeyValuePairList, GmlList::Edge);

                if(!result)
                    return false;
            }

            int newPercentage = ((i * 50) / (int)keyValuePairList.size()) + 50;
            if(newPercentage > percentage)
            {
                percentage = newPercentage;
                emit progress(percentage);
            }

            break;
        }

        case GmlList::Node:
            if(intValue != nullptr)
            {
                if(!key.compare("id"))
                    nodeIdMap.insert(*intValue, graph.addNode());
                else
                {
                    // Unhandled node data
                }
            }
            break;

        case GmlList::Edge:
            if(intValue != nullptr)
            {
                if(!key.compare("source") && nodeIdMap.contains(*intValue))
                    sourceId = nodeIdMap[*intValue];
                else if(!key.compare("target") && !sourceId.isNull() && nodeIdMap.contains(*intValue))
                    graph.addEdge(sourceId, nodeIdMap[*intValue]);
                else
                {
                    // Unhandled edge data
                }
            }
            break;
        }
    }

    return true;
}

void GmlFileParser::onParsePositionIncremented(int64_t position)
{
    int64_t newPercentage = (position * 50) / fileSize;

    if(newPercentage > percentage)
    {
        percentage = newPercentage;
        emit progress(percentage);
    }
}

bool GmlFileParser::parse(Graph& graph)
{
    QFileInfo info(filename);
    std::ifstream stream(filename.toStdString());
    stream.unsetf(std::ios::skipws);

    fileSize = info.size();

    spirit::istream_iterator begin(stream);
    progress_iterator endp;
    progress_iterator beginp(begin, this, &endp);

    GmlGrammar<progress_iterator> grammar;
    KeyValuePairList keyValuePairList;

    if(qi::phrase_parse(beginp, endp, grammar, boost::spirit::ascii::space, keyValuePairList))
        return parseGmlList(graph, keyValuePairList);

    return false;
}
