#include "gmlfileparser.h"

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/spirit/home/x3.hpp"
#include "boost/fusion/include/adapt_struct.hpp"
#include "boost/spirit/home/support/iterators/istream_iterator.hpp"
#include "thirdparty/boost/boost_spirit_qstring_adapter.h"

#include "progress_iterator.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphmodel.h"

#include <QUrl>
#include <QFileInfo>
#include <QTextDocumentFragment>

#include <fstream>

// http://www.fim.uni-passau.de/fileadmin/files/lehrstuhl/brandenburg/projekte/gml/gml-technical-report.pdf

namespace SpiritGmlParser
{
struct KeyValue;
using List = std::vector<boost::recursive_wrapper<KeyValue>>;

using Value = boost::variant<double, int, QString, List>;
struct KeyValue
{
    QString _key;
    Value _value;
};
} // namespace SpiritGmlParser

BOOST_FUSION_ADAPT_STRUCT(
    SpiritGmlParser::KeyValue,
    (QString, _key),
    (SpiritGmlParser::Value, _value)
)

namespace SpiritGmlParser
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::lit;
// Only parse strict doubles (i.e. not integers)
x3::real_parser<double, x3::strict_real_policies<double>> const double_ = {};
using x3::int_;
using x3::lexeme;
using ascii::char_;

const x3::rule<class L, List> list = "list";

const x3::rule<class NoQuotesString, QString> noQuotesString = "noQuotesString";
const auto noQuotesString_def = lexeme[x3::lit('"') >> *(~char_('"')) >> x3::lit('"')];

const x3::rule<class V, Value> value = "value";
const auto value_def = double_ | int_ | noQuotesString | (x3::lit('[') >> list >> x3::lit(']'));

const x3::rule<class K, QString> key = "key";
const auto key_def = lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];

const x3::rule<class KV, KeyValue> keyValue = "keyValue";
const auto keyValue_def = key >> value;

const auto list_def = *keyValue;

BOOST_SPIRIT_DEFINE(list, noQuotesString, key, value, keyValue)

struct Attribute
{
    Attribute(QString name, QString value) :
        _name(std::move(name)), _value(std::move(value))
    {}

    QString _name;
    QString _value;
};

using AttributeVector = std::vector<Attribute>;

AttributeVector processAttribute(const KeyValue& attribute)
{
    struct Visitor
    {
        QString _name;
        explicit Visitor(QString name) : _name(std::move(name)) {}

        AttributeVector operator()(double v) const          { return {{_name, QString::number(v)}}; }
        AttributeVector operator()(int v) const             { return {{_name, QString::number(v)}}; }
        AttributeVector operator()(const QString& v) const
        {
            return {{_name, QTextDocumentFragment::fromHtml(v).toPlainText()}};
        }
        AttributeVector operator()(const List& v) const
        {
            AttributeVector result;

            for(const auto& attribute : v)
            {
                auto childAttributes = processAttribute(attribute.get());

                for(const auto& childAttribute : childAttributes)
                {
                    QString subName = _name + "." + childAttribute._name;
                    result.emplace_back(subName, childAttribute._value);
                }
            }

            return result;
        }
    };

    return boost::apply_visitor(Visitor(attribute._key), attribute._value);
}

bool build(const List& gml, IGraphModel& graphModel,
    UserNodeData& userNodeData, UserEdgeData& userEdgeData,
    const ProgressFn& progressFn, const Cancellable& cancellable)
{
    auto findIntValue = [](const List& list, const QString& key) -> const int*
    {
        auto keyValue = std::find_if(list.begin(), list.end(), [&](auto& item)
        {
           return item.get()._key == key;
        });

        if(keyValue != list.end())
            return boost::get<int>(&keyValue->get()._value);

        return nullptr;
    };

    std::map<int, NodeId> gmlIdToNodeId;

    auto processNode = [&](const List& node)
    {
        auto id = findIntValue(node, QStringLiteral("id"));
        if(id == nullptr)
            return false;

        auto nodeId = graphModel.mutableGraph().addNode();
        gmlIdToNodeId[*id] = nodeId;

        auto nodeName = QString::number(*id);

        for(const auto& attributeWrapper : node)
        {
            const auto& attribute = attributeWrapper.get();
            if(attribute._key == QStringLiteral("id"))
                continue;

            if(attribute._key == QStringLiteral("label"))
            {
                // If there is a label attribute, use it as the node name
                const auto* label = boost::get<QString>(&attribute._value);
                if(label != nullptr)
                    nodeName = *label;
            }
            else
            {
                auto attributes = processAttribute(attribute);

                for(const auto& attribute : attributes)
                {
                    QString attributeName = QObject::tr("Node ") + attribute._name;
                    userNodeData.setValueBy(nodeId, attributeName, attribute._value);
                }
            }
        }

        userNodeData.setValueBy(nodeId, QObject::tr("Node Name"), nodeName);
        graphModel.setNodeName(nodeId, nodeName);

        return true;
    };

    auto processEdge = [&](const List& edge)
    {
        auto sourceId = findIntValue(edge, QStringLiteral("source"));
        auto targetId = findIntValue(edge, QStringLiteral("target"));

        if(sourceId == nullptr || targetId == nullptr)
            return false;

        if(!u::contains(gmlIdToNodeId, *sourceId) || !u::contains(gmlIdToNodeId, *targetId))
            return false;

        auto sourceNodeId = gmlIdToNodeId[*sourceId];
        auto targetNodeId = gmlIdToNodeId[*targetId];
        auto edgeId = graphModel.mutableGraph().addEdge(sourceNodeId, targetNodeId);

        for(const auto& attributeWrapper : edge)
        {
            const auto& attribute = attributeWrapper.get();
            if(attribute._key == QStringLiteral("source") || attribute._key == QStringLiteral("target"))
                continue;

            auto attributes = processAttribute(attribute);

            for(const auto& attribute : attributes)
            {
                QString attributeName = QObject::tr("Edge ") + attribute._name;
                userEdgeData.setValueBy(edgeId, attributeName, attribute._value);
            }
        }

        return true;
    };

    for(const auto& keyValue : gml)
    {
        const auto& key = keyValue.get()._key;

        if(key == QStringLiteral("graph"))
        {
            const auto* graph = boost::get<List>(&keyValue.get()._value);

            if(graph == nullptr)
                return false;

            uint64_t i = 0;
            for(const auto& element : *graph)
            {
                progressFn(static_cast<int>((i++ * 100) / graph->size()));

                const auto& type = element.get()._key;
                const auto* value = boost::get<List>(&element.get()._value);

                if(value == nullptr)
                    continue;

                bool success = true;

                if(type == QStringLiteral("node"))
                    success = processNode(*value);
                else if(type == QStringLiteral("edge"))
                    success = processEdge(*value);

                if(!success || cancellable.cancelled())
                    return false;
            }
        }
    }

    return true;
}

} // namespace SpiritGmlParser

GmlFileParser::GmlFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
    _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
}

bool GmlFileParser::parse(const QUrl& url, IGraphModel& graphModel, const ProgressFn& progressFn)
{
    QString localFile = url.toLocalFile();
    QFileInfo fileInfo(localFile);

    if(!fileInfo.exists())
        return false;

    auto fileSize = fileInfo.size();

    progressFn(-1);

    std::ifstream stream(localFile.toStdString());
    stream.unsetf(std::ios::skipws);

    boost::spirit::istream_iterator istreamIt(stream);
    using GmlIterator = progress_iterator<decltype(istreamIt)>;
    GmlIterator it(istreamIt);
    GmlIterator end;

    it.onPositionChanged(
    [&fileSize, &progressFn](size_t position)
    {
        progressFn(static_cast<int>((position * 100) / fileSize));
    });

    auto cancelledFn = [this] { return cancelled(); };
    it.setCancelledFn(cancelledFn);

    graphModel.mutableGraph().setPhase(QObject::tr("Parsing"));

    SpiritGmlParser::List gml;
    bool success = false;

    try
    {
        success = SpiritGmlParser::x3::phrase_parse(it, end,
            SpiritGmlParser::list, SpiritGmlParser::ascii::space, gml);
    }
    catch(GmlIterator::cancelled_exception&) {}

    if(cancelled() || !success || it != end)
        return false;

    graphModel.mutableGraph().setPhase(QObject::tr("Building Graph"));
    progressFn(-1);

    return SpiritGmlParser::build(gml, graphModel,
        *_userNodeData, *_userEdgeData,
        progressFn, *this);
}
