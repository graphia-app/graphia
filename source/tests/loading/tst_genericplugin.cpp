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

#include "tests/common/testdocument.h"
#include "tests/common/testenvironment.h"
#include "tests/common/testgraphmodel.h"
#include "tests/common/xlsxwriter.h"

#include "shared/loading/adjacencymatrixfileparser.h"
#include "shared/loading/biopaxfileparser.h"
#include "shared/loading/cxparser.h"
#include "shared/loading/dotfileparser.h"
#include "shared/loading/gmlfileparser.h"
#include "shared/loading/graphmlparser.h"
#include "shared/loading/iparserthread.h"
#include "shared/loading/jsongraphparser.h"
#include "shared/loading/pairwisefileparser.h"
#include "shared/plugins/basegenericplugin.h"
#include "app/transform/graphtransformconfigparser.h"

#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>

#include <functional>
#include <memory>
#include <utility>
#include <vector>

using namespace Qt::Literals::StringLiterals;

// As GenericPlugin, which is built as a plugin, so can't be linked
class TestGenericPluginInstance : public BaseGenericPluginInstance
{
    Q_OBJECT
};

class TestGenericPlugin : public BaseGenericPlugin, public PluginInstanceProvider<TestGenericPluginInstance>
{
    Q_OBJECT

public:
    QString name() const override { return u"Generic"_s; }
    QString displayName() const override { return u"Generic"_s; }
    QString description() const override { return {}; }
    QString imageSource() const override { return {}; }
    int dataVersion() const override { return 3; }
    QString qmlModule() const override { return {}; }
};

// As ParserThread, from which the plugin instance learns that a file has loaded
class TestParserThread : public QObject, public IParserThread
{
    Q_OBJECT

signals:
    void success(IParser*);
};

class TestGenericPlugin_ : public QObject
{
    Q_OBJECT

    QTemporaryDir _dir;
    TestGenericPlugin _plugin;

    QUrl write(const QString& fileName, const QByteArray& content)
    {
        auto filePath = _dir.filePath(fileName);
        QFile file(filePath);
        if(!file.open(QIODevice::WriteOnly))
            return {};

        file.write(content);
        return QUrl::fromLocalFile(filePath);
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        QVERIFY(_dir.isValid());
    }

    void identifyUrl_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<QStringList>("expected");

        QTest::newRow("GML") << u"graph.gml"_s << QByteArray("graph [ node [ id 0 ] ]") << QStringList{u"GML"_s};
        QTest::newRow("GML upper case") << u"GRAPH.GML"_s << QByteArray("graph [ node [ id 0 ] ]") << QStringList{u"GML"_s};
        QTest::newRow("GraphML") << u"graph.graphml"_s << QByteArray("<graphml><graph></graph></graphml>") << QStringList{u"GraphML"_s};
        QTest::newRow("DOT") << u"graph.dot"_s << QByteArray("graph { a -- b }") << QStringList{u"DOT"_s};
        QTest::newRow("Pairwise CSV") << u"pairwise.csv"_s << QByteArray("a,b\nb,c\nc,d\n") << QStringList{u"PairwiseCSV"_s};
        QTest::newRow("Weighted pairwise CSV") << u"weighted.csv"_s << QByteArray("a,b,1\nb,c,2\nc,d,3\n") << QStringList{u"PairwiseCSV"_s};
        // An adjacency matrix could also be an edge list
        QTest::newRow("Matrix CSV") << u"matrix.csv"_s << QByteArray(",a,b,c\na,0,1,0\nb,1,0,1\nc,0,1,0\n") << (QStringList{u"MatrixCSV"_s, u"PairwiseCSV"_s});
        QTest::newRow("Matrix TSV") << u"matrix.tsv"_s << QByteArray("\ta\tb\na\t0\t1\nb\t1\t0\n") << (QStringList{u"MatrixTSV"_s, u"PairwiseTSV"_s});
        QTest::newRow("Pairwise TXT") << u"pairwise.txt"_s << QByteArray("a b\nb c\n") << QStringList{u"PairwiseTXT"_s};
        QTest::newRow("Pairwise SSV") << u"pairwise.ssv"_s << QByteArray("a;b\nb;c\n") << QStringList{u"PairwiseSSV"_s};
        QTest::newRow("JSON Graph") << u"graph.json"_s << QByteArray(R"({"graph": {"nodes": {}}})") << QStringList{u"JSONGraph"_s};
        // Only the extension is checked, and that it's JSON
        QTest::newRow("JSON not a graph") << u"other.json"_s << QByteArray(R"({"a": 1})") << QStringList{u"JSONGraph"_s};
        QTest::newRow("CX") << u"graph.cx"_s << QByteArray(R"([{"numberVerification": [{"longNumber": 281474976710655}]}])") << QStringList{u"CX"_s};
        QTest::newRow("CX2") << u"graph.cx2"_s << QByteArray(R"([{"CXVersion": "2.0"}])") << QStringList{u"CX"_s};
        QTest::newRow("BioPAX") << u"pathway.owl"_s << QByteArray("<rdf:RDF></rdf:RDF>") << QStringList{u"BiopaxOWL"_s};
        QTest::newRow("Pairwise XLSX") << u"pairwise.xlsx"_s <<
            XlsxWriter::xlsx({{u"a"_s, u"b"_s}, {u"b"_s, u"c"_s}}) << QStringList{u"PairwiseXLSX"_s};
        QTest::newRow("Matrix XLSX") << u"matrix.xlsx"_s <<
            XlsxWriter::xlsx({{u""_s, u"a"_s, u"b"_s}, {u"a"_s, u"0"_s, u"1"_s}, {u"b"_s, u"1"_s, u"0"_s}}) <<
            QStringList{u"MatrixXLSX"_s, u"PairwiseXLSX"_s};
        QTest::newRow("Not XLSX") << u"csv.xlsx"_s << QByteArray("a,b\n") << QStringList{};
        QTest::newRow("Unknown extension") << u"graph.xyz"_s << QByteArray("a,b\n") << QStringList{};
        QTest::newRow("No extension") << u"graph"_s << QByteArray("a,b\n") << QStringList{};
        QTest::newRow("Empty CSV") << u"empty.csv"_s << QByteArray() << QStringList{};
        QTest::newRow("Binary CSV") << u"binary.csv"_s << QByteArray("\x00\x01\x02\xff", 4) << QStringList{};
    }

    void identifyUrl()
    {
        QFETCH(const QString, fileName);
        QFETCH(const QByteArray, content);
        QFETCH(const QStringList, expected);

        auto url = write(fileName, content);
        QVERIFY(url.isValid());

        auto actual = _plugin.identifyUrl(url);
        QCOMPARE(actual, expected);

        if(!expected.isEmpty())
            QVERIFY(_plugin.failureReason(url).contains(expected.join(u", "_s)));
    }

    void notLocal()
    {
        QCOMPARE(_plugin.identifyUrl(QUrl(u"https://example.com/graph.gml"_s)), QStringList{});
        QCOMPARE(_plugin.failureReason(QUrl(u"https://example.com/graph.gml"_s)), QString());
        QCOMPARE(_plugin.failureReason(QUrl::fromLocalFile(u"graph.xyz"_s)), QString());
    }

    void urlTypes()
    {
        QCOMPARE(_plugin.parametersQmlType(u"MatrixCSV"_s), u"MatrixParameters"_s);
        QCOMPARE(_plugin.parametersQmlType(u"PairwiseXLSX"_s), u"PairwiseParameters"_s);
        QCOMPARE(_plugin.parametersQmlType(u"GML"_s), QString());
        QVERIFY(_plugin.editable());

        QCOMPARE(_plugin.loadableUrlTypeNames().size(), 17);
        QVERIFY(_plugin.loadableUrlTypeNames().contains(u"MatrixMatLab"_s));
        QCOMPARE(_plugin.extensionsForUrlTypeName(u"CX"_s), (QStringList{u"cx"_s, u"cx2"_s}));
    }

    // The parser each type of file is loaded with
    void parserForUrlTypeName()
    {
        TestGraphModel model({u"a-b"_s});
        TestDocument document(model);
        const TestParserThread parserThread;
        auto instance = _plugin.createInstance();
        instance->initialise(&_plugin, &document, &parserThread);

        using IsParserFn = std::function<bool(const IParser*)>;
        auto is = []<typename Parser>(const Parser*) -> IsParserFn
            { return [](const IParser* parser) { return dynamic_cast<const Parser*>(parser) != nullptr; }; };

        const std::vector<std::pair<QString, IsParserFn>> expected
        {
            {u"GML"_s,          is(static_cast<GmlFileParser*>(nullptr))},
            {u"GraphML"_s,      is(static_cast<GraphMLParser*>(nullptr))},
            {u"DOT"_s,          is(static_cast<DotFileParser*>(nullptr))},
            {u"PairwiseCSV"_s,  is(static_cast<PairwiseCSVFileParser*>(nullptr))},
            {u"PairwiseSSV"_s,  is(static_cast<PairwiseSSVFileParser*>(nullptr))},
            {u"PairwiseTSV"_s,  is(static_cast<PairwiseTSVFileParser*>(nullptr))},
            {u"PairwiseTXT"_s,  is(static_cast<PairwiseTXTFileParser*>(nullptr))},
            {u"PairwiseXLSX"_s, is(static_cast<PairwiseXLSXFileParser*>(nullptr))},
            {u"MatrixCSV"_s,    is(static_cast<AdjacencyMatrixCSVFileParser*>(nullptr))},
            {u"MatrixSSV"_s,    is(static_cast<AdjacencyMatrixSSVFileParser*>(nullptr))},
            {u"MatrixTSV"_s,    is(static_cast<AdjacencyMatrixTSVFileParser*>(nullptr))},
            {u"MatrixTXT"_s,    is(static_cast<AdjacencyMatrixTXTFileParser*>(nullptr))},
            {u"MatrixXLSX"_s,   is(static_cast<AdjacencyMatrixXLSXFileParser*>(nullptr))},
            {u"MatrixMatLab"_s, is(static_cast<AdjacencyMatrixMatLabFileParser*>(nullptr))},
            {u"BiopaxOWL"_s,    is(static_cast<BiopaxFileParser*>(nullptr))},
            {u"JSONGraph"_s,    is(static_cast<JsonGraphParser*>(nullptr))},
            {u"CX"_s,           is(static_cast<CxParser*>(nullptr))},
        };

        for(const auto& [urlTypeName, isExpectedParser] : expected)
        {
            auto parser = instance->parserForUrlTypeName(urlTypeName);
            QVERIFY2(isExpectedParser(parser.get()), qPrintable(urlTypeName));
        }

        // Every type the plugin can load has a parser
        QCOMPARE(static_cast<qsizetype>(expected.size()), _plugin.loadableUrlTypeNames().size());

        for(const auto* urlTypeName : {"Unknown", "PairwiseXYZ", "MatrixXYZ"})
            QVERIFY(instance->parserForUrlTypeName(QString::fromLatin1(urlTypeName)) == nullptr);
    }

    // The table's rows highlighted, and the names of the nodes selected, as the plugin's UI shows them
    void highlightedRowsAndSelectedNames()
    {
        auto model = TestGraphModel::parse(R"(graph [ node [ id 0 label "a" ] node [ id 1 label "b" ] node [ id 2 label "c" ] ])",
            [](auto* userNodeData, auto* userEdgeData) { return std::make_unique<GmlFileParser>(userNodeData, userEdgeData); });
        QVERIFY(model != nullptr);

        TestDocument document(*model);
        const TestParserThread parserThread;
        auto instance = _plugin.createInstance();
        instance->initialise(&_plugin, &document, &parserThread);
        auto* genericInstance = dynamic_cast<BaseGenericPluginInstance*>(instance.get());
        QVERIFY(genericInstance != nullptr);

        const auto& userNodeData = model->graphModel().userNodeData();
        auto rowFor = [&](const QString& name) { return static_cast<int>(userNodeData.indexFor(model->nodeId(name))); };

        document.selectNodes({model->nodeId(u"a"_s), model->nodeId(u"c"_s)});
        QCOMPARE(genericInstance->property("selectedNodeNames").toString(), u"a, c"_s);
        document.selectNodes({});
        QCOMPARE(genericInstance->property("selectedNodeNames").toString(), QString());

        const QSignalSpy highlightedRowsChanged(genericInstance, SIGNAL(highlightedRowsChanged()));
        genericInstance->setHighlightedRows({rowFor(u"b"_s), rowFor(u"c"_s)});
        QCOMPARE(document._highlightedNodeIds, (NodeIdSet{model->nodeId(u"b"_s), model->nodeId(u"c"_s)}));
        QCOMPARE(highlightedRowsChanged.count(), 1);

        genericInstance->setHighlightedRows({});
        QVERIFY(document._highlightedNodeIds.empty());
        QCOMPARE(highlightedRowsChanged.count(), 2);

        // Nothing was highlighted, and still isn't
        genericInstance->setHighlightedRows({});
        QCOMPARE(highlightedRowsChanged.count(), 2);
    }

    void defaultTransformsAndVisualisations()
    {
        auto instance = _plugin.createInstance();
        QCOMPARE(instance->defaultTransforms(), QStringList{});
        QCOMPARE(instance->defaultVisualisations(), QStringList{});

        // The threshold is only applied when edges are filtered
        instance->applyParameter(u"initialThreshold"_s, 0.5);
        QCOMPARE(instance->defaultTransforms(), QStringList{});

        instance->applyParameter(u"filterEdges"_s, u"true"_s);
        instance->applyParameter(u"additionalTransforms"_s, QStringList{u"MCLCluster"_s});
        instance->applyParameter(u"additionalVisualisations"_s, QStringList{u"MCLCluster Colour"_s});

        const QStringList expectedTransforms
        {
            u"RemoveEdges where $AbsoluteEdgeWeight < 0.5"_s,
            u"MCLCluster"_s
        };

        QCOMPARE(instance->defaultTransforms(), expectedTransforms);
        QCOMPARE(instance->defaultVisualisations(), QStringList{u"MCLCluster Colour"_s});

        const auto defaultTransforms = instance->defaultTransforms();
        for(const auto& transform : defaultTransforms)
            QVERIFY(GraphTransformConfigParser().parse(transform));

        instance->applyParameter(u"filterEdges"_s, u"false"_s);
        QCOMPARE(instance->defaultTransforms(), QStringList{u"MCLCluster"_s});

        QTest::ignoreMessage(QtDebugMsg, QRegularExpression(u"unknown parameter"_s));
        instance->applyParameter(u"unknown"_s, 1);
    }

    void edgeFilterThresholdPrecision_data()
    {
        QTest::addColumn<double>("threshold");
        QTest::addColumn<QString>("expected");

        QTest::newRow("Large") << 1234567.0 << u"1234567"_s;
        QTest::newRow("Many digits") << 0.123456789 << u"0.123456789"_s;
        QTest::newRow("Small") << 0.0000123456 << u"1.23456e-05"_s;
        QTest::newRow("Negative") << -2.5 << u"-2.5"_s;
    }

    void edgeFilterThresholdPrecision()
    {
        QFETCH(const double, threshold);
        QFETCH(const QString, expected);

        // The threshold must be exactly as given, so that the edges whose
        // weights are the same as it, but no more, are kept
        auto instance = _plugin.createInstance();
        instance->applyParameter(u"filterEdges"_s, u"true"_s);
        instance->applyParameter(u"initialThreshold"_s, threshold);

        const auto transform = instance->defaultTransforms().at(0);
        QCOMPARE(transform, u"RemoveEdges where $AbsoluteEdgeWeight < %1"_s.arg(expected));
        QVERIFY(GraphTransformConfigParser().parse(transform));
    }
};

QTEST_MAIN(TestGenericPlugin_)
#include "tst_genericplugin.moc"
