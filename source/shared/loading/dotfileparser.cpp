#include "dotfileparser.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/support/iterators/istream_iterator.hpp>
#include <boost/boost_spirit_qstring_adapter.h>

#include "progress_iterator.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphmodel.h"

#include <QUrl>
#include <QFileInfo>

#include <fstream>

// https://www.graphviz.org/doc/info/lang.html

namespace SpiritDotParser
{
struct DotGraph
{
    QString _content;
};

} // namespace SpiritDotParser

BOOST_FUSION_ADAPT_STRUCT(
    SpiritDotParser::DotGraph,
    _content
)

namespace SpiritDotParser
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::lit;
// Only parse strict doubles (i.e. not integers)
x3::real_parser<double, x3::strict_real_policies<double>> const double_ = {};
using x3::int_;
using x3::lexeme;
using ascii::char_;

const x3::rule<class DG, DotGraph> dotGraph = "dotGraph";
const auto dotGraph_def = *char_;

BOOST_SPIRIT_DEFINE(dotGraph)

bool build(DotFileParser&, const DotGraph&, IGraphModel&,
    UserNodeData&, UserEdgeData&)
{
    return false;
}

} // namespace SpiritDotParser

DotFileParser::DotFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
    _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
}

bool DotFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    QString localFile = url.toLocalFile();
    QFileInfo fileInfo(localFile);

    if(!fileInfo.exists())
        return false;

    auto fileSize = fileInfo.size();

    setProgress(-1);

    std::ifstream stream(localFile.toStdString());
    stream.unsetf(std::ios::skipws);

    boost::spirit::istream_iterator istreamIt(stream);
    using DotIterator = progress_iterator<decltype(istreamIt)>;
    DotIterator it(istreamIt);
    DotIterator end;

    it.onPositionChanged(
    [this, &fileSize](size_t position)
    {
        setProgress(static_cast<int>((position * 100) / fileSize));
    });

    auto cancelledFn = [this] { return cancelled(); };
    it.setCancelledFn(cancelledFn);

    graphModel->mutableGraph().setPhase(QObject::tr("Parsing"));

    SpiritDotParser::DotGraph dot;
    bool success = false;

    try
    {
        success = SpiritDotParser::x3::phrase_parse(it, end,
            SpiritDotParser::dotGraph, SpiritDotParser::ascii::space, dot);
    }
    catch(DotIterator::cancelled_exception&) {}

    if(cancelled() || !success || it != end)
        return false;

    graphModel->mutableGraph().setPhase(QObject::tr("Building Graph"));
    setProgress(-1);

    return SpiritDotParser::build(*this, dot, *graphModel,
        *_userNodeData, *_userEdgeData);
}
