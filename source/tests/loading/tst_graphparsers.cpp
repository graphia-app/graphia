/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tests/common/testenvironment.h"
#include "tests/common/testgraphmodel.h"
#include "tests/common/xlsxwriter.h"

#include "shared/loading/adjacencymatrixfileparser.h"
#include "shared/loading/biopaxfileparser.h"
#include "shared/loading/cxparser.h"
#include "shared/loading/dotfileparser.h"
#include "shared/loading/gmlfileparser.h"
#include "shared/loading/graphmlparser.h"
#include "shared/loading/jsongraphparser.h"
#include "shared/loading/matlabfileparser.h"
#include "shared/loading/pairwisefileparser.h"

#include <QTest>
#include <QByteArray>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>

#include <array>
#include <functional>
#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
using CreateMatVarFn = std::function<matvar_t*()>;

// The content of a MATLAB file holding the variables created by the given functions
QByteArray matFile(const std::vector<CreateMatVarFn>& createVariables)
{
    const QTemporaryDir dir;
    auto fileName = dir.filePath(u"data.mat"_s);

    auto* matFile = Mat_CreateVer(qPrintable(fileName), nullptr, MAT_FT_MAT5);
    if(matFile == nullptr)
        return {};

    for(const auto& createVariable : createVariables)
    {
        auto* variable = createVariable();
        Mat_VarWrite(matFile, variable, MAT_COMPRESSION_NONE);
        Mat_VarFree(variable);
    }

    Mat_Close(matFile);

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
        return {};

    return file.readAll();
}

// A dense matrix, whose values are given in column major order, as MATLAB stores them
template<typename T>
CreateMatVarFn denseMatrix(const char* name, size_t rows, size_t columns, std::vector<T> values,
    matio_classes classType = MAT_C_DOUBLE, matio_types dataType = MAT_T_DOUBLE)
{
    return [=]() mutable
    {
        std::array<size_t, 2> dims{rows, columns};
        return Mat_VarCreate(name, classType, dataType, 2, dims.data(), values.data(), 0);
    };
}

// A sparse matrix, given as (row, column, value) triples, in column order
CreateMatVarFn sparseMatrix(const char* name, size_t rows, size_t columns,
    const std::vector<std::tuple<mat_uint32_t, mat_uint32_t, double>>& entries)
{
    return [=]
    {
        std::vector<mat_uint32_t> ir;
        std::vector<mat_uint32_t> jc(columns + 1, 0);
        std::vector<double> data;

        ir.reserve(entries.size());
        data.reserve(entries.size());
        for(const auto& [row, column, value] : entries)
        {
            ir.push_back(row);
            data.push_back(value);
            jc.at(column + 1)++;
        }

        for(size_t column = 0; column < columns; column++)
            jc.at(column + 1) += jc.at(column);

        mat_sparse_t sparse{};
        sparse.nzmax = static_cast<mat_uint32_t>(data.size());
        sparse.ir = ir.data();
        sparse.nir = static_cast<mat_uint32_t>(ir.size());
        sparse.jc = jc.data();
        sparse.njc = static_cast<mat_uint32_t>(jc.size());
        sparse.ndata = static_cast<mat_uint32_t>(data.size());
        sparse.data = data.data();

        std::array<size_t, 2> dims{rows, columns};
        return Mat_VarCreate(name, MAT_C_SPARSE, MAT_T_DOUBLE, 2, dims.data(), &sparse, 0);
    };
}

// A struct with one field, holding the given variable
CreateMatVarFn structOf(const char* name, const char* fieldName, const CreateMatVarFn& createField)
{
    return [=]
    {
        std::array<size_t, 2> dims{1, 1};
        std::array<const char*, 1> fieldNames{fieldName};
        auto* variable = Mat_VarCreateStruct(name, 2, dims.data(), fieldNames.data(), 1);
        Mat_VarSetStructFieldByName(variable, fieldName, 0, createField());
        return variable;
    };
}

// Parses content in the given format, which is the name of a parser,
// optionally followed by a description of how it is configured
std::unique_ptr<TestGraphModel> parse(const QString& format, const QByteArray& content,
    QString* failureReason = nullptr)
{
    auto parseWith = [&](auto createParser)
    {
        return TestGraphModel::parse(content, createParser, failureReason);
    };

    auto parseWithType = [&]<typename Parser>(const Parser*)
    {
        return parseWith([](auto* userNodeData, auto* userEdgeData)
            { return std::make_unique<Parser>(userNodeData, userEdgeData); });
    };

    if(format == u"PairwiseCSV"_s)  return parseWithType(static_cast<PairwiseCSVFileParser*>(nullptr));
    if(format == u"PairwiseTXT"_s)  return parseWithType(static_cast<PairwiseTXTFileParser*>(nullptr));
    if(format == u"GML"_s)          return parseWithType(static_cast<GmlFileParser*>(nullptr));
    if(format == u"GraphML"_s)      return parseWithType(static_cast<GraphMLParser*>(nullptr));
    if(format == u"DOT"_s)          return parseWithType(static_cast<DotFileParser*>(nullptr));
    if(format == u"JSONGraph"_s)    return parseWithType(static_cast<JsonGraphParser*>(nullptr));
    if(format == u"CX"_s)           return parseWithType(static_cast<CxParser*>(nullptr));
    if(format == u"MatrixCSV"_s)    return parseWithType(static_cast<AdjacencyMatrixCSVFileParser*>(nullptr));
    if(format == u"MatrixMatLab"_s) return parseWithType(static_cast<AdjacencyMatrixMatLabFileParser*>(nullptr));
    if(format == u"PairwiseXLSX"_s) return parseWithType(static_cast<PairwiseXLSXFileParser*>(nullptr));
    if(format == u"MatrixXLSX"_s)   return parseWithType(static_cast<AdjacencyMatrixXLSXFileParser*>(nullptr));

    if(format == u"PairwiseCSV attributes"_s)
    {
        return parseWith([](auto* userNodeData, auto* userEdgeData)
        {
            auto parser = std::make_unique<PairwiseCSVFileParser>(userNodeData, userEdgeData);
            parser->setFirstRowIsHeader(true);
            parser->setColumnsConfiguration(
            {
                {0, {._type = PairwiseColumnType::SourceNode, ._name = {}}},
                {1, {._type = PairwiseColumnType::TargetNode, ._name = {}}},
                {2, {._type = PairwiseColumnType::EdgeAttribute, ._name = u"Weight"_s}},
                {3, {._type = PairwiseColumnType::SourceNodeAttribute, ._name = u"Type"_s}},
                {4, {._type = PairwiseColumnType::TargetNodeAttribute, ._name = u"Type"_s}},
            });

            return parser;
        });
    }

    if(format == u"MatrixCSV skip duplicates"_s || format == u"MatrixCSV minimum 0.5"_s)
    {
        return parseWith([&format](auto* userNodeData, auto* userEdgeData)
        {
            auto parser = std::make_unique<AdjacencyMatrixCSVFileParser>(userNodeData, userEdgeData);

            if(format.endsWith(u"skip duplicates"_s))
                parser->setSkipDuplicates(true);
            else
                parser->setMinimumAbsEdgeWeight(0.5);

            return parser;
        });
    }

    if(format == u"BioPAX"_s)
    {
        return parseWith([](auto* userNodeData, auto*)
            { return std::make_unique<BiopaxFileParser>(userNodeData); });
    }

    return nullptr;
}
} // namespace

class TestGraphParsers : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void parse_data()
    {
        QTest::addColumn<QString>("format");
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<QStringList>("nodes");
        QTest::addColumn<QStringList>("edges");

        // Each value is "attribute|node|value" or "attribute|source|target|value"
        QTest::addColumn<QStringList>("values");

        // Pairwise
        QTest::newRow("Pairwise") << u"PairwiseCSV"_s << QByteArray("a,b\nb,c\na,b\nc,c\n") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s} <<
            QStringList{u"a-b"_s, u"a-b"_s, u"b-c"_s, u"c-c"_s} <<
            QStringList{u"NodeName|a|a"_s};
        QTest::newRow("Pairwise attributes") << u"PairwiseCSV attributes"_s <<
            QByteArray("From,To,Weight,From Type,To Type\na,b,1.5,x,y\nb,c,2,z,w\n") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s} <<
            QStringList{u"a-b"_s, u"b-c"_s} <<
            QStringList{u"Weight|a|b|1.5"_s, u"Weight|b|c|2"_s, u"Type|a|x"_s, u"Type|b|z"_s, u"Type|c|w"_s};
        QTest::newRow("Pairwise blank rows") << u"PairwiseCSV"_s << QByteArray("a,b\n\n,\nc,\n,d\n") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s} << QStringList{u"a-b"_s} << QStringList{};
        QTest::newRow("Pairwise one column") << u"PairwiseCSV"_s << QByteArray("a\nb\n") <<
            QStringList{u"a"_s, u"b"_s} << QStringList{} << QStringList{};
        QTest::newRow("Pairwise missing columns") << u"PairwiseCSV attributes"_s << QByteArray("From,To\na,b\nb,c\n") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s} << QStringList{u"a-b"_s, u"b-c"_s} << QStringList{};
        QTest::newRow("Pairwise TXT") << u"PairwiseTXT"_s << QByteArray("a  b\n\"c d\"\te\n") <<
            QStringList{u"a"_s, u"b"_s, u"c d"_s, u"e"_s} <<
            QStringList{u"a-b"_s, u"c d-e"_s} << QStringList{};

        // GML
        QTest::newRow("GML") << u"GML"_s << QByteArray(
            "graph [ directed 1\n"
            " node [ id 1 label \"A\" weight 2.5 ]\n"
            " node [ id 2 label \"B\" graphics [ x 1 y -2.0 fill \"#ff0000\" ] ]\n"
            " node [ id 3 ]\n"
            " edge [ source 1 target 2 label \"e1\" value 3 ]\n"
            " edge [ source 2 target 3 ] ]\n") <<
            QStringList{u"3"_s, u"A"_s, u"B"_s} <<
            QStringList{u"A-B"_s, u"B-3"_s} <<
            QStringList{u"weight|A|2.5"_s, u"graphics_x|B|1"_s, u"graphics_y|B|-2"_s,
                u"graphics_fill|B|#ff0000"_s, u"label|A|B|e1"_s, u"value|A|B|3"_s};
        QTest::newRow("GML edges first") << u"GML"_s <<
            QByteArray(R"(graph [ edge [ source 1 target 2 ] node [ id 1 label "A" ] node [ id 2 label "B" ] ])") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-B"_s} << QStringList{};
        QTest::newRow("GML CRLF loops") << u"GML"_s << QByteArray(
            "graph [\r\n node [ id 1 label \"A\" ]\r\n node [ id 2 label \"B\" ]\r\n"
            " edge [ source 1 target 1 ]\r\n edge [ source 1 target 2 ]\r\n edge [ source 1 target 2 ]\r\n]") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-A"_s, u"A-B"_s, u"A-B"_s} << QStringList{};
        QTest::newRow("GML UTF-8") << u"GML"_s << QByteArray("graph [ node [ id 1 label \"\xc3\xa9\xe2\x82\xac\" ] ]") <<
            QStringList{u"é€"_s} << QStringList{} << QStringList{};
        QTest::newRow("GML numbers") << u"GML"_s << QByteArray("graph [ node [ id 1 label \"A\" x 1e3 y .5 ] ]") <<
            QStringList{u"A"_s} << QStringList{} << QStringList{u"x|A|1000"_s, u"y|A|0.5"_s};
        QTest::newRow("GML clashing names") << u"GML"_s <<
            QByteArray(R"(graph [ node [ id 1 label "A" ComponentSize "n" w 1 ] edge [ source 1 target 1 w 2 ] ])") <<
            QStringList{u"A"_s} << QStringList{u"A-A"_s} <<
            QStringList{u"User ComponentSize|A|n"_s, u"w|A|1"_s, u"Edge w|A|A|2"_s};

        // Not a raw string, which moc misparses here, missing the test class
        QTest::newRow("GML strings") << u"GML"_s << QByteArray(
            "graph [ node [ id 1 label \"A&amp;B\" a \"x < y\" b \"<b>two</b>  spaces\" c \"&quot;&#233;&#xe9;&eacute;&bogus;\" ] ]") << // NOLINT modernize-raw-string-literal
            QStringList{u"A&B"_s} << QStringList{} <<
            QStringList{u"a|A&B|x < y"_s, u"b|A&B|<b>two</b>  spaces"_s, u"c|A&B|\"ééé&bogus;"_s};

        QTest::newRow("GML comments") << u"GML"_s <<
            QByteArray("# Comment\ngraph [\n# Comment \"\n node [ id 1 label \"#A\" ]\n]\n# Comment") <<
            QStringList{u"#A"_s} << QStringList{} << QStringList{};

        // GraphML
        QTest::newRow("GraphML") << u"GraphML"_s << QByteArray(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\">\n"
            "<key id=\"d0\" for=\"node\" attr.name=\"color\" attr.type=\"string\"/>\n"
            "<key id=\"d1\" for=\"edge\" attr.name=\"weight\" attr.type=\"double\"/>\n"
            "<graph id=\"G\" edgedefault=\"undirected\">\n"
            "<node id=\"n0\"><data key=\"d0\">green</data></node>\n"
            "<node id=\"n1\"/>\n"
            "<node id=\"n2\"><data key=\"d0\">blue</data></node>\n"
            "<edge id=\"e0\" source=\"n0\" target=\"n1\"><data key=\"d1\">1.5</data></edge>\n"
            "<edge source=\"n1\" target=\"n2\"/>\n"
            "<edge source=\"n2\" target=\"n2\"/>\n"
            "</graph></graphml>\n") <<
            QStringList{u"n0"_s, u"n1"_s, u"n2"_s} <<
            QStringList{u"n0-n1"_s, u"n1-n2"_s, u"n2-n2"_s} <<
            QStringList{u"color|n0|green"_s, u"color|n2|blue"_s, u"weight|n0|n1|1.5"_s, u"EdgeName|n0|n1|e0"_s};

        QTest::newRow("GraphML split text") << u"GraphML"_s << QByteArray(
            "<graphml><key id=\"d0\" for=\"node\" attr.name=\"text\"/><graph>"
            "<node id=\"a\"><data key=\"d0\">x &amp; y<![CDATA[ <z>]]><!-- c --> w</data></node>"
            "</graph></graphml>") <<
            QStringList{u"a"_s} << QStringList{} << QStringList{u"text|a|x & y <z> w"_s};

        QTest::newRow("GraphML for all") << u"GraphML"_s << QByteArray(
            "<graphml><key id=\"d0\" for=\"all\" attr.name=\"note\"/><graph>"
            "<node id=\"a\"><data key=\"d0\">x</data></node><node id=\"b\"/>"
            "<edge source=\"a\" target=\"b\"><data key=\"d0\">y</data></edge>"
            "</graph></graphml>") <<
            QStringList{u"a"_s, u"b"_s} << QStringList{u"a-b"_s} << QStringList{u"note|a|x"_s, u"Edge note|a|b|y"_s};

        QTest::newRow("GraphML defaults") << u"GraphML"_s << QByteArray(
            "<graphml>"
            "<key id=\"d0\" for=\"node\" attr.name=\"color\"><default>yellow</default></key>"
            "<key id=\"d1\" for=\"edge\" attr.name=\"weight\"><default>1</default></key>"
            "<graph><node id=\"a\"><data key=\"d0\">green</data></node><node id=\"b\"/><node id=\"c\"/>"
            "<edge source=\"a\" target=\"b\"><data key=\"d1\">2</data></edge><edge source=\"b\" target=\"c\"/>"
            "</graph></graphml>") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s} << QStringList{u"a-b"_s, u"b-c"_s} <<
            QStringList{u"color|a|green"_s, u"color|b|yellow"_s, u"color|c|yellow"_s,
                u"weight|a|b|2"_s, u"weight|b|c|1"_s};

        QTest::newRow("GraphML edges first") << u"GraphML"_s << QByteArray(
            "<graphml><key id=\"d0\" for=\"edge\" attr.name=\"weight\"/><graph>"
            "<edge id=\"e\" source=\"a\" target=\"b\"><data key=\"d0\">2</data></edge>"
            "<edge source=\"a\" target=\"z\"/><node id=\"a\"/><node id=\"b\"/>"
            "</graph></graphml>") <<
            QStringList{u"a"_s, u"b"_s} << QStringList{u"a-b"_s} <<
            QStringList{u"weight|a|b|2"_s, u"EdgeName|a|b|e"_s};

        // DOT
        QTest::newRow("DOT") << u"DOT"_s << QByteArray(
            "// c\n"
            "/* c */\n"
            "digraph G {\n"
            "  rankdir=LR;\n"
            "  a [label=\"Alpha\", weight=2];\n"
            "  a -> b -> c [color=red];\n"
            "  d:p1:n -> e:sw;\n"
            "  \"quoted \\\"x\\\"\" -> 42;\n"
            "  -1.5 -> .5;\n"
            "  f;\n"
            "}\n") <<
            QStringList{u"-1.5"_s, u".5"_s, u"42"_s, u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s, u"f"_s, u"quoted \"x\""_s} <<
            QStringList{u"-1.5-.5"_s, u"a-b"_s, u"b-c"_s, u"d-e"_s, u"quoted \"x\"-42"_s} <<
            QStringList{u"label|a|Alpha"_s, u"weight|a|2"_s, u"color|a|b|red"_s, u"color|b|c|red"_s};
        QTest::newRow("DOT undirected") << u"DOT"_s << QByteArray("graph { a -- b; b -- a; c -- c }") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s} << QStringList{u"a-b"_s, u"b-a"_s, u"c-c"_s} << QStringList{};
        QTest::newRow("DOT subgraphs") << u"DOT"_s << QByteArray("digraph { {a b} -> {c d}; e -> subgraph s1 { f g } }") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s, u"f"_s, u"g"_s} <<
            QStringList{u"a-c"_s, u"a-d"_s, u"b-c"_s, u"b-d"_s, u"e-f"_s, u"e-g"_s} << QStringList{};
        QTest::newRow("DOT attribute separators") << u"DOT"_s << QByteArray("digraph { a [x=1, y=2; z=3] b [ ] }") <<
            QStringList{u"a"_s, u"b"_s} << QStringList{} <<
            QStringList{u"x|a|1"_s, u"y|a|2"_s, u"z|a|3"_s};
        QTest::newRow("DOT HTML") << u"DOT"_s << QByteArray("digraph { a [label=<<b>bold</b> &amp; <i>it</i>>] }") <<
            QStringList{u"a"_s} << QStringList{} << QStringList{u"label|a|<b>bold</b> &amp; <i>it</i>"_s};
        QTest::newRow("DOT UTF-8 CRLF") << u"DOT"_s << QByteArray("digraph {\r\n caf\xc3\xa9 -> \"\xe2\x82\xac\" [k=\"v\"];\r\n // c\r\n}") <<
            QStringList{u"café"_s, u"€"_s} << QStringList{u"café-€"_s} << QStringList{u"k|café|€|v"_s};
        QTest::newRow("DOT keyword prefixes") << u"DOT"_s << QByteArray("digraph { graphA -> nodeB; edgeC; subgraphD }") <<
            QStringList{u"edgeC"_s, u"graphA"_s, u"nodeB"_s, u"subgraphD"_s} << QStringList{u"graphA-nodeB"_s} << QStringList{};
        QTest::newRow("DOT no semicolons") << u"DOT"_s << QByteArray("digraph { a -> b c -> d }") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s} << QStringList{u"a-b"_s, u"c-d"_s} << QStringList{};
        QTest::newRow("DOT node defaults") << u"DOT"_s <<
            QByteArray("digraph { a [color=blue]; node [color=red]; b; c [color=green] }") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s} << QStringList{} <<
            QStringList{u"color|a|blue"_s, u"color|b|red"_s, u"color|c|green"_s};
        QTest::newRow("DOT subgraph defaults") << u"DOT"_s <<
            QByteArray("graph { node [k=x] a; subgraph s { node [k=y]; b } c }") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s} << QStringList{} <<
            QStringList{u"k|a|x"_s, u"k|b|y"_s, u"k|c|x"_s};
        QTest::newRow("DOT edge defaults") << u"DOT"_s <<
            QByteArray("digraph { a -> b; edge [style=dashed]; b -> c; c -> d [style=bold] }") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s} << QStringList{u"a-b"_s, u"b-c"_s, u"c-d"_s} <<
            QStringList{u"style|a|b|"_s, u"style|b|c|dashed"_s, u"style|c|d|bold"_s};
        QTest::newRow("DOT keyword case") << u"DOT"_s <<
            QByteArray("STRICT DiGraph { Node [k=x]; a -> b; SubGraph s { c } }") <<
            QStringList{u"a"_s, u"b"_s, u"c"_s} << QStringList{u"a-b"_s} << QStringList{u"k|c|x"_s};
        QTest::newRow("DOT empty") << u"DOT"_s << QByteArray("digraph {}") <<
            QStringList{} << QStringList{} << QStringList{};

        // JSON Graph
        QTest::newRow("JSON Graph") << u"JSONGraph"_s << QByteArray(R"({"graph":{"nodes":[
            {"id":"n1","label":"A","metadata":{"type":"x","count":3,"score":1.5}},
            {"id":"n2","label":"B"}],
            "edges":[{"source":"n1","target":"n2","metadata":{"weight":2.5,"kind":"k"}}]}})") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-B"_s} <<
            QStringList{u"type|A|x"_s, u"count|A|3"_s, u"score|A|1.5"_s, u"weight|A|B|2.5"_s, u"kind|A|B|k"_s};
        QTest::newRow("JSON Graph multiple") << u"JSONGraph"_s << QByteArray(R"({"graphs":[
            {"nodes":[{"id":"1","label":"X"},{"id":"2","label":"Y"}],"edges":[{"source":"2","target":"1"}]},
            {"nodes":[{"id":"q","label":"Q"}],"edges":[]}]})") <<
            QStringList{u"X"_s, u"Y"_s} << QStringList{u"Y-X"_s} << QStringList{};
        QTest::newRow("JSON Graph objects") << u"JSONGraph"_s << QByteArray(R"({"graph":{
            "nodes":{"k1":{"id":"a","label":"A"},"k2":{"id":"b","label":"B"}},
            "edges":{"e":{"source":"a","target":"b"}}}})") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-B"_s} << QStringList{};
        // JSON Graph Format 2 keys nodes by their ids
        QTest::newRow("JSON Graph keyed nodes") << u"JSONGraph"_s << QByteArray(R"({"graph":{
            "nodes":{"a":{"label":"A"},"b":{"label":"B","metadata":{"type":"y"}}},
            "edges":[{"source":"a","target":"b"}]}})") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-B"_s} << QStringList{u"type|B|y"_s};

        // CX
        QTest::newRow("CX1") << u"CX"_s << QByteArray(R"([{"numberVerification":[{"longNumber":281474976710655}]},
            {"nodes":[{"@id":10,"n":"A","r":"up:1"},{"@id":11,"n":"B"}]},
            {"edges":[{"@id":20,"s":10,"t":11,"i":"binds"}]},
            {"nodeAttributes":[{"po":10,"n":"type","v":"protein"},{"po":11,"n":"tags","v":["x","y"]}]},
            {"edgeAttributes":[{"po":20,"n":"score","v":"0.5"}]}])") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-B"_s} <<
            QStringList{u"NodeName|A|A"_s, u"NodeRepresents|A|up:1"_s, u"EdgeInteraction|A|B|binds"_s,
                u"type|A|protein"_s, uR"(tags|B|["x","y"])"_s, u"score|A|B|0.5"_s};
        QTest::newRow("CX2") << u"CX"_s << QByteArray(R"([{"CXVersion":"2.0","hasFragments":false},
            {"attributeDeclarations":[{"nodes":{"name":{"d":"string","a":"n"},"kind":{"d":"string","a":"k"}},
                "edges":{"interaction":{"d":"string","a":"i"}}}]},
            {"nodes":[{"id":1,"v":{"k":"protein","n":"A"}},{"id":2,"v":{"n":"B"}}]},
            {"edges":[{"id":5,"s":1,"t":2,"v":{"i":"binds","w":0.5}}]}])") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-B"_s} <<
            QStringList{u"kind|A|protein"_s, u"interaction|A|B|binds"_s, u"w|A|B|0.5"_s};

        QTest::newRow("CX2 attributes after name") << u"CX"_s <<
            QByteArray(R"([{"CXVersion":"2.0"},{"nodes":[{"id":1,"v":{"n":"A","score":2}}]}])") <<
            QStringList{u"A"_s} << QStringList{} << QStringList{u"score|A|2"_s};

        // BioPAX
        QTest::newRow("BioPAX") << u"BioPAX"_s << QByteArray(
            "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" "
            "xmlns:bp=\"http://www.biopax.org/release/biopax-level3.owl#\">\n"
            "<bp:Protein rdf:ID=\"p1\"><bp:displayName>Kinase</bp:displayName><bp:comment>c</bp:comment></bp:Protein>\n"
            "<bp:SmallMolecule rdf:ID=\"m1\"/>\n"
            "<bp:SmallMolecule rdf:ID=\"m2\"/>\n"
            "<bp:BiochemicalReaction rdf:ID=\"r1\"><bp:left rdf:resource=\"#m1\"/><bp:right rdf:resource=\"#m2\"/>"
            "</bp:BiochemicalReaction>\n"
            "<bp:Catalysis rdf:ID=\"c1\"><bp:controller rdf:resource=\"#p1\"/><bp:controlled rdf:resource=\"#r1\"/>"
            "</bp:Catalysis>\n"
            "</rdf:RDF>\n") <<
            QStringList{u"Kinase"_s, u"c1"_s, u"m1"_s, u"m2"_s, u"r1"_s} <<
            QStringList{u"c1-Kinase"_s, u"c1-r1"_s, u"r1-m1"_s, u"r1-m2"_s} <<
            QStringList{u"ID|Kinase|p1"_s, u"Class|Kinase|Protein"_s, u"Comment|Kinase|c"_s};
        QTest::newRow("BioPAX nested") << u"BioPAX"_s << QByteArray(
            "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" "
            "xmlns:bp=\"http://www.biopax.org/release/biopax-level3.owl#\">\n"
            "<bp:Complex rdf:ID=\"x\"><bp:Protein/></bp:Complex><bp:Protein rdf:ID=\"p\"/>\n"
            "</rdf:RDF>\n") <<
            QStringList{u"p"_s, u"x"_s} << QStringList{} << QStringList{};

        // MATLAB adjacency matrices; as with CSV, an edge is created from each row's node to each column's
        // node; the matrix here is 4 by 4, with 2 at row 1, column 2, and -1 at row 4, column 1
        // (a 3 by 3 matrix of integers can't be distinguished from an edge list)
        std::vector<double> matrix(16, 0.0);
        matrix.at(4) = 2.0;
        matrix.at(3) = -1.0;
        const std::vector<mat_int32_t> intMatrix(matrix.begin(), matrix.end());

        const QStringList matrixNodes{u"Node 1"_s, u"Node 2"_s, u"Node 4"_s};
        const QStringList matrixEdges{u"Node 1-Node 2"_s, u"Node 4-Node 1"_s};
        const QStringList matrixValues{u"EdgeWeight|Node 1|Node 2|2"_s, u"EdgeWeight|Node 4|Node 1|-1"_s};

        QTest::newRow("MATLAB") << u"MatrixMatLab"_s << matFile({denseMatrix("m", 4, 4, matrix)}) <<
            matrixNodes << matrixEdges << matrixValues;
        QTest::newRow("MATLAB int32") << u"MatrixMatLab"_s <<
            matFile({denseMatrix("m", 4, 4, intMatrix, MAT_C_INT32, MAT_T_INT32)}) <<
            matrixNodes << matrixEdges << matrixValues;
        QTest::newRow("MATLAB struct") << u"MatrixMatLab"_s <<
            matFile({structOf("s", "m", denseMatrix("m", 4, 4, matrix))}) <<
            matrixNodes << matrixEdges << matrixValues;
        // The biggest matrix in the file is loaded
        QTest::newRow("MATLAB several variables") << u"MatrixMatLab"_s <<
            matFile({denseMatrix<double>("small", 1, 2, {5.0, 6.0}), denseMatrix("m", 4, 4, matrix),
                denseMatrix<double>("smaller", 1, 1, {7.0})}) <<
            matrixNodes << matrixEdges << matrixValues;
        QTest::newRow("MATLAB precision") << u"MatrixMatLab"_s <<
            matFile({denseMatrix<double>("m", 2, 2, {0, 0, 0.1, 0})}) <<
            QStringList{u"Node 1"_s, u"Node 2"_s} << QStringList{u"Node 1-Node 2"_s} <<
            QStringList{u"EdgeWeight|Node 1|Node 2|0.1"_s};

        // Adjacency matrices; an edge is created from each row's node to each column's node
        QTest::newRow("Matrix") << u"MatrixCSV"_s << QByteArray("0,2\n-3,0\n") <<
            QStringList{u"Node 1"_s, u"Node 2"_s} << QStringList{u"Node 1-Node 2"_s, u"Node 2-Node 1"_s} <<
            QStringList{u"EdgeWeight|Node 1|Node 2|2"_s, u"AbsoluteEdgeWeight|Node 1|Node 2|2"_s,
                u"EdgeWeight|Node 2|Node 1|-3"_s, u"AbsoluteEdgeWeight|Node 2|Node 1|3"_s};
        QTest::newRow("Matrix skip duplicates") << u"MatrixCSV skip duplicates"_s << QByteArray("0,2\n-3,0\n") <<
            QStringList{u"Node 1"_s, u"Node 2"_s} << QStringList{u"Node 1-Node 2"_s} <<
            QStringList{u"EdgeWeight|Node 1|Node 2|-3"_s, u"AbsoluteEdgeWeight|Node 1|Node 2|3"_s};
        QTest::newRow("Matrix headers") << u"MatrixCSV"_s << QByteArray(",A,B,C\nA,0,1,0\nB,1,0,0.5\nC,0,0.5,0\n") <<
            QStringList{u"A"_s, u"B"_s, u"C"_s} << QStringList{u"A-B"_s, u"B-A"_s, u"B-C"_s, u"C-B"_s} <<
            QStringList{u"EdgeWeight|C|B|0.5"_s};
        QTest::newRow("Matrix headers skip duplicates") << u"MatrixCSV skip duplicates"_s <<
            QByteArray(",A,B,C\nA,0,1,0\nB,1,0,0.5\nC,0,0.5,0\n") <<
            QStringList{u"A"_s, u"B"_s, u"C"_s} << QStringList{u"A-B"_s, u"B-C"_s} << QStringList{};
        QTest::newRow("Matrix minimum weight") << u"MatrixCSV minimum 0.5"_s <<
            QByteArray(",A,B,C\nA,0,1,0\nB,1,0,0.5\nC,0,0.5,0\n") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-B"_s, u"B-A"_s} << QStringList{};
        QTest::newRow("Matrix column headers") << u"MatrixCSV"_s << QByteArray("A,B\n0,1\n2,0\n") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-B"_s, u"B-A"_s} <<
            QStringList{u"EdgeWeight|A|B|1"_s, u"EdgeWeight|B|A|2"_s};
        QTest::newRow("Matrix row headers") << u"MatrixCSV"_s << QByteArray("A,0,1\nB,2,0\n") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-B"_s, u"B-A"_s} <<
            QStringList{u"EdgeWeight|A|B|1"_s, u"EdgeWeight|B|A|2"_s};
        QTest::newRow("Matrix empty cells") << u"MatrixCSV"_s << QByteArray(",A,B\nA,5,\nB,1,\n") <<
            QStringList{u"A"_s, u"B"_s} << QStringList{u"A-A"_s, u"B-A"_s} <<
            QStringList{u"EdgeWeight|A|A|5"_s, u"EdgeWeight|B|A|1"_s};

        // XLSX
        QTest::newRow("Pairwise XLSX") << u"PairwiseXLSX"_s <<
            XlsxWriter::xlsx({{u"a"_s, u"b"_s}, {u"b"_s, u"c d"_s}, {u"a & e"_s, u"1"_s}}) <<
            QStringList{u"1"_s, u"a"_s, u"a & e"_s, u"b"_s, u"c d"_s} <<
            QStringList{u"a & e-1"_s, u"a-b"_s, u"b-c d"_s} << QStringList{};
        QTest::newRow("Matrix XLSX") << u"MatrixXLSX"_s <<
            XlsxWriter::xlsx({{u""_s, u"A"_s, u"B"_s, u"C"_s}, {u"A"_s, u"0"_s, u"1"_s, u"0"_s},
                {u"B"_s, u"1"_s, u"0"_s, u"0.5"_s}, {u"C"_s, u"0"_s, u"0.5"_s, u"0"_s}}) <<
            QStringList{u"A"_s, u"B"_s, u"C"_s} << QStringList{u"A-B"_s, u"B-A"_s, u"B-C"_s, u"C-B"_s} <<
            QStringList{u"EdgeWeight|C|B|0.5"_s};
    }

    void parse()
    {
        QFETCH(const QString, format);
        QFETCH(const QByteArray, content);
        QFETCH(const QStringList, nodes);
        QFETCH(const QStringList, edges);
        QFETCH(const QStringList, values);

        QString failureReason;
        auto model = ::parse(format, content, &failureReason);
        QVERIFY2(model != nullptr, qPrintable(failureReason));

        QCOMPARE(model->nodeNames(), nodes);
        QCOMPARE(model->edgeNames(), edges);

        for(const auto& value : values)
        {
            auto fields = value.split(u'|');

            if(fields.size() == 3)
                QCOMPARE(model->nodeValue(fields.at(0), fields.at(1)), fields.at(2));
            else
                QCOMPARE(model->edgeValue(fields.at(0), fields.at(1), fields.at(2)), fields.at(3));
        }
    }

    // A sparse matrix is loaded as an edge list, whose nodes are named in the order they're found
    void matLabSparse()
    {
        QString failureReason;
        auto model = ::parse(u"MatrixMatLab"_s,
            matFile({sparseMatrix("m", 4, 4, {{3, 0, -1.0}, {0, 1, 2.0}})}), &failureReason);
        QVERIFY2(model != nullptr, qPrintable(failureReason));

        QCOMPARE(model->graph().numNodes(), 3);
        QCOMPARE(model->graph().numEdges(), 2);

        QStringList weights;
        weights.reserve(static_cast<qsizetype>(model->graph().edgeIds().size()));
        for(auto edgeId : model->graph().edgeIds())
            weights.append(model->graphModel().attributeValueByName(u"EdgeWeight"_s).stringValueOf(edgeId));

        weights.sort();
        QCOMPARE(weights, (QStringList{u"-1"_s, u"2"_s}));
    }

    void matLabInvalid()
    {
        QString failureReason;
        QVERIFY(::parse(u"MatrixMatLab"_s, QByteArray("not a MATLAB file"), &failureReason) == nullptr);
    }

    void numericPrecision_data()
    {
        QTest::addColumn<QString>("format");
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<QString>("attribute");
        QTest::addColumn<QStringList>("element");
        QTest::addColumn<double>("expected");

        QTest::newRow("GML") << u"GML"_s <<
            QByteArray("graph [ node [ id 1 label \"A\" x 1234567.5 ] ]") <<
            u"x"_s << QStringList{u"A"_s} << 1234567.5;
        QTest::newRow("GML small") << u"GML"_s <<
            QByteArray("graph [ node [ id 1 label \"A\" x 0.123456789 ] ]") <<
            u"x"_s << QStringList{u"A"_s} << 0.123456789;
        QTest::newRow("JSON Graph") << u"JSONGraph"_s <<
            QByteArray(R"({"graph":{"nodes":[{"id":"a","label":"A","metadata":{"x":0.123456789}}],"edges":[]}})") <<
            u"x"_s << QStringList{u"A"_s} << 0.123456789;
        QTest::newRow("JSON Graph large integer") << u"JSONGraph"_s <<
            QByteArray(R"({"graph":{"nodes":[{"id":"a","label":"A","metadata":{"x":3000000000}}],"edges":[]}})") <<
            u"x"_s << QStringList{u"A"_s} << 3000000000.0;
        QTest::newRow("Pairwise large integer") << u"PairwiseCSV attributes"_s <<
            QByteArray("From,To,Weight,From Type,To Type\na,b,3000000000,x,y\nb,c,2,z,w\n") <<
            u"Weight"_s << QStringList{u"a"_s, u"b"_s} << 3000000000.0;
        QTest::newRow("Matrix") << u"MatrixCSV"_s << QByteArray("0,0.123456789\n1234567.5,0\n") <<
            u"EdgeWeight"_s << QStringList{u"Node 1"_s, u"Node 2"_s} << 0.123456789;
    }

    void numericPrecision()
    {
        QFETCH(const QString, format);
        QFETCH(const QByteArray, content);
        QFETCH(const QString, attribute);
        QFETCH(const QStringList, element);
        QFETCH(const double, expected);

        QString failureReason;
        auto model = ::parse(format, content, &failureReason);
        QVERIFY2(model != nullptr, qPrintable(failureReason));

        auto actual = element.size() == 1 ? model->nodeNumericValue(attribute, element.at(0)) :
            model->edgeNumericValue(attribute, element.at(0), element.at(1));

        // Attribute values are stored as floats; these must be exactly
        // equal, which QCOMPARE doesn't require of floating point values
        QVERIFY2(std::equal_to<>()(static_cast<float>(actual), static_cast<float>(expected)),
            qPrintable(u"%1 != %2"_s.arg(actual, 0, 'g', 10).arg(expected, 0, 'g', 10)));
    }

    // Elements that have no value for an attribute have no value, whatever its type
    void missingValues()
    {
        auto model = ::parse(u"GML"_s, QByteArray("graph [ node [ id 0 label \"a\" score 1.5 count 1 kind \"x\" ] "
            "node [ id 1 label \"b\" ] ]"));
        QVERIFY(model != nullptr);

        for(const auto& attributeName : {u"score"_s, u"count"_s, u"kind"_s})
        {
            QVERIFY(model->nodeHasValue(attributeName, u"a"_s));
            QVERIFY2(!model->nodeHasValue(attributeName, u"b"_s), qPrintable(attributeName));
        }

        model->applyTransforms({u"RemoveNodes where $score hasValue"_s});
        QCOMPARE(model->nodeNames(), QStringList{u"b"_s});
    }

    void syntaxErrorLocation_data()
    {
        QTest::addColumn<QString>("format");
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<QString>("expected");

        // The values are missing at the closing ]
        QTest::newRow("GML") << u"GML"_s << QByteArray("graph [\n  node [ id 1 x ]\n]") <<
            u"The file is malformed at line 2, column 17."_s;
        QTest::newRow("DOT") << u"DOT"_s << QByteArray("digraph {\n  a -> ;\n}") <<
            u"The file is malformed at line 2, column 8."_s;
    }

    void syntaxErrorLocation()
    {
        QFETCH(const QString, format);
        QFETCH(const QByteArray, content);
        QFETCH(const QString, expected);

        QString failureReason;
        QVERIFY(::parse(format, content, &failureReason) == nullptr);
        QCOMPARE(failureReason, expected);
    }

    void parseFailure_data()
    {
        QTest::addColumn<QString>("format");
        QTest::addColumn<QByteArray>("content");

        for(const auto* format : {"PairwiseCSV", "GML", "GraphML", "DOT", "JSONGraph", "CX", "MatrixCSV", "BioPAX"})
            QTest::addRow("%s empty", format) << QString::fromLatin1(format) << QByteArray();

        QTest::newRow("GML truncated") << u"GML"_s << QByteArray("graph [ node [ id 1 label \"A\" ");
        QTest::newRow("GML node without id") << u"GML"_s << QByteArray("graph [ node [ label \"A\" ] ]");
        QTest::newRow("GML real id") << u"GML"_s << QByteArray("graph [ node [ id 1.0 ] ]");
        QTest::newRow("GML unknown node") << u"GML"_s << QByteArray("graph [ node [ id 1 ] edge [ source 1 target 9 ] ]");

        // Not a raw string, which moc misparses here, missing the test class
        QTest::newRow("GraphML truncated") << u"GraphML"_s <<
            QByteArray("<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\"><graph><node id=\"a\"/>"); // NOLINT modernize-raw-string-literal
        QTest::newRow("GraphML not GraphML") << u"GraphML"_s << QByteArray("<foo/>");
        QTest::newRow("GraphML duplicate id") << u"GraphML"_s <<
            QByteArray(R"(<graphml><graph><node id="a"/><node id="a"/></graph></graphml>)");
        QTest::newRow("GraphML node without id") << u"GraphML"_s <<
            QByteArray("<graphml><graph><node/></graph></graphml>");
        QTest::newRow("GraphML edge without target") << u"GraphML"_s <<
            QByteArray(R"(<graphml><graph><node id="a"/><edge source="a"/></graph></graphml>)");

        QTest::newRow("DOT truncated") << u"DOT"_s << QByteArray("digraph G { a -> b");
        QTest::newRow("DOT not DOT") << u"DOT"_s << QByteArray("hello");

        QTest::newRow("JSON Graph invalid") << u"JSONGraph"_s << QByteArray("{");
        QTest::newRow("JSON Graph array") << u"JSONGraph"_s << QByteArray("[]");
        QTest::newRow("JSON Graph no graph") << u"JSONGraph"_s << QByteArray(R"({"x":1})");
        QTest::newRow("JSON Graph no nodes") << u"JSONGraph"_s << QByteArray(R"({"graph":{"edges":[]}})");
        QTest::newRow("JSON Graph node without id") << u"JSONGraph"_s <<
            QByteArray(R"({"graph":{"nodes":[{"label":"A"}],"edges":[]}})");
        QTest::newRow("JSON Graph edge without target") << u"JSONGraph"_s <<
            QByteArray(R"({"graph":{"nodes":[{"id":"a"}],"edges":[{"source":"a"}]}})");
        QTest::newRow("JSON Graph unknown node") << u"JSONGraph"_s <<
            QByteArray(R"({"graph":{"nodes":[{"id":"a"}],"edges":[{"source":"a","target":"z"}]}})");

        QTest::newRow("JSON Graph numeric label") << u"JSONGraph"_s <<
            QByteArray(R"({"graph":{"nodes":[{"id":"a","label":1}],"edges":[]}})");
        QTest::newRow("JSON Graph numeric metadata") << u"JSONGraph"_s <<
            QByteArray(R"({"graph":{"nodes":[{"id":"a","metadata":5}],"edges":[]}})");

        QTest::newRow("CX object") << u"CX"_s << QByteArray("{}");
        QTest::newRow("CX not objects") << u"CX"_s << QByteArray("[1]");
        QTest::newRow("CX unknown version") << u"CX"_s << QByteArray(R"([{"foo":1}])");
        QTest::newRow("CX empty version") << u"CX"_s << QByteArray(R"([{"CXVersion":""}])");
        QTest::newRow("CX numeric version") << u"CX"_s << QByteArray(R"([{"CXVersion":2}])");
        QTest::newRow("CX numeric alias") << u"CX"_s <<
            QByteArray(R"([{"CXVersion":"2.0"},{"attributeDeclarations":[{"nodes":{"name":{"a":1}}}]}])");
        QTest::newRow("CX string id") << u"CX"_s << QByteArray(R"([{"numberVerification":[{"longNumber":281474976710655}]},
            {"nodes":[{"@id":"10","n":"A"}]}])");
        QTest::newRow("CX unknown node") << u"CX"_s << QByteArray(R"([{"numberVerification":[{"longNumber":281474976710655}]},
            {"nodes":[{"@id":10,"n":"A"}]},{"edges":[{"@id":20,"s":10,"t":99}]}])");

        QTest::newRow("Matrix single value") << u"MatrixCSV"_s << QByteArray("5\n");
        QTest::newRow("Matrix not square") << u"MatrixCSV"_s << QByteArray(",A,B\nA,0,1\n");
        QTest::newRow("Matrix non-numeric") << u"MatrixCSV"_s << QByteArray(",A,B\nA,0,x\nB,1,0\n");
        QTest::newRow("Matrix mismatched headers") << u"MatrixCSV"_s << QByteArray(",A,B\nC,0,1\nD,1,0\n");
    }

    void parseFailure()
    {
        QFETCH(const QString, format);
        QFETCH(const QByteArray, content);

        QString failureReason;
        QVERIFY(::parse(format, content, &failureReason) == nullptr);
        QVERIFY(!failureReason.isEmpty());
    }
};

QTEST_MAIN(TestGraphParsers)
#include "tst_graphparsers.moc"
