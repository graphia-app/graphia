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
#include "tests/common/testplugin.h"

#include "app/layout/nodepositions.h"
#include "app/loading/nativeloader.h"
#include "app/loading/nativesaver.h"

#include "shared/utils/json.h"

#include <QTest>
#include <QByteArray>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>
#include <QVector3D>

#include <memory>
#include <string>

using namespace Qt::Literals::StringLiterals;

namespace
{
// Loads content in the native format, in the same way as Document
std::unique_ptr<TestGraphModel> load(const QByteArray& content, TestPluginInstance& pluginInstance,
    QString* failureReason = nullptr)
{
    return TestGraphModel::parse(content, [&pluginInstance](auto*, auto*)
    {
        auto loader = std::make_unique<Loader>();
        loader->setPluginInstance(&pluginInstance);
        return loader;
    }, failureReason);
}

QByteArray nativeFile(const json& header, const json& body)
{
    return QByteArray::fromStdString(json::array({header, body}).dump());
}

json header(int version, int pluginDataVersion = 1)
{
    return {{"version", version}, {"pluginName", "Test"}, {"pluginDataVersion", pluginDataVersion}};
}

// The graph a -> b -> c, with ids 0, 1 and 2
json graph()
{
    json nodes = json::array();
    for(const auto* id : {"0", "1", "2"})
        nodes.push_back({{"id", id}});

    json edges = json::array();
    edges.push_back({{"id", "0"}, {"source", "0"}, {"target", "1"}});
    edges.push_back({{"id", "1"}, {"source", "1"}, {"target", "2"}});

    return {{"directed", true}, {"nodes", nodes}, {"edges", edges}};
}

const QVector3D PositionOfB(1.5F, -2.25F, 3.0F);

// The body of a file of the given version, as the version of NativeSaver at the time would have
// written it; the differences between versions are those that NativeLoader accounts for
json body(int version)
{
    json body;

    body["graph"] = graph();

    const std::vector<std::string> names{"a", "b", "c"};
    const std::vector<json> positions{{0.0, 0.0, 0.0},
        {PositionOfB.x(), PositionOfB.y(), PositionOfB.z()}, {1.0, 1.0, 1.0}};

    json jsonNames = json::array();
    json jsonPositions = json::array();

    for(size_t i = 0; i < names.size(); i++)
    {
        if(version >= 4)
        {
            // Values are paired with the ids of their nodes
            jsonNames.push_back({{"id", i}, {"value", names.at(i)}});
            jsonPositions.push_back({{"id", i}, {"value", positions.at(i)}});
        }
        else
        {
            // Values are in the order of the ids of their nodes
            jsonNames.push_back(names.at(i));
            jsonPositions.push_back(positions.at(i));
        }
    }

    body["nodeNames"] = jsonNames;

    json userNodeData;
    userNodeData["vectors"] = json::array({{{"Type", {{"values", {"x", "y", "z"}}}}}});
    userNodeData[version <= 3 ? "indexes" : "ids"] = {0, 1, 2};

    json userEdgeData;
    userEdgeData["vectors"] = json::array({{{"Weight", {{"values", {"0.5", "2"}}}}}});
    userEdgeData[version <= 3 ? "indexes" : "ids"] = {0, 1};

    json pluginData = {{"pluginKey", "pluginValue"}};

    if(version >= 6)
    {
        body["userNodeData"] = userNodeData;
        body["userEdgeData"] = userEdgeData;
    }
    else
    {
        // Older versions store the user data in the plugin data
        pluginData["userNodeData"] = userNodeData;
        pluginData["userEdgeData"] = userEdgeData;
    }

    body["pluginData"] = pluginData;

    if(version >= 2)
    {
        body["ui"] = {{"uiKey", "uiValue"}};
        body["pluginUiData"] = {{"pluginUiKey", "pluginUiValue"}};
    }
    else
    {
        // Version 1 only has plugin UI data, which is called "ui"
        body["ui"] = {{"pluginUiKey", "pluginUiValue"}};
    }

    body["layout"] =
    {
        {"algorithm", "ForceDirected"},
        {"settings", {{"ShortRangeRepulseTerm", 2.5}}},
        {"positions", jsonPositions},
        {"paused", true}
    };

    if(version >= 9)
    {
        body["transforms"] = {"RemoveLeaves where $NodeDegree < 1"};
        body["visualisations"] = {"NodeDegree Size"};
    }
    else
    {
        // Older versions name transforms and attributes as they were displayed
        body["transforms"] = {R"("Remove Leaves" where $"Node Degree" < 1)"};
        body["visualisations"] = {R"("Node Degree" "Size")"};
    }
    body["bookmarks"] = {{"Ends", {0, 2}}};
    body["log"] = "Log text";
    json enrichmentTable;
    enrichmentTable["data"] = json::array({json::array({"x", 0.25})});
    enrichmentTable["selectionA"] = "A";
    enrichmentTable["selectionB"] = "B";
    body["enrichmentTables"] = json::array({enrichmentTable});

    body["nodeSize"] = 0.5;
    body["edgeSize"] = 0.25;
    body["textSize"] = 0.75;

    return body;
}

QByteArray readFile(const QString& fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
        return {};

    return file.readAll();
}

// QCOMPARE can't compare json values on every platform, so they're compared as strings
QString jsonString(const json& value)
{
    return QString::fromStdString(value.dump());
}
} // namespace

class TestNativeLoader : public QObject
{
    Q_OBJECT

private:
    TestPlugin _plugin;

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void versions_data()
    {
        QTest::addColumn<int>("version");

        for(int version = 1; version <= NativeSaver::Version; version++)
            QTest::addRow("%d", version) << version;
    }

    void versions()
    {
        QFETCH(int, version);

        TestPluginInstance pluginInstance(&_plugin);
        QString failureReason;
        auto model = load(nativeFile(header(version), body(version)), pluginInstance, &failureReason);
        QVERIFY2(model != nullptr, qPrintable(failureReason));

        const auto* loader = model->parser<Loader>();
        QVERIFY(loader != nullptr);

        QCOMPARE(model->nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));
        QCOMPARE(model->edgeNames(), (QStringList{u"a-b"_s, u"b-c"_s}));

        QCOMPARE(model->nodeValue(u"Type"_s, u"b"_s), u"y"_s);
        QCOMPARE(model->edgeNumericValue(u"Weight"_s, u"b"_s, u"c"_s), 2.0);

        QVERIFY(loader->nodePositions() != nullptr);
        QCOMPARE(loader->nodePositions()->get(model->nodeId(u"b"_s)), PositionOfB);
        QCOMPARE(loader->layoutName(), u"ForceDirected"_s);
        QCOMPARE(loader->layoutSettings().size(), 1);
        QCOMPARE(loader->layoutSettings().at(0)._name, u"ShortRangeRepulseTerm"_s);
        QCOMPARE(loader->layoutSettings().at(0)._value, 2.5F);
        QVERIFY(loader->layoutPaused());

        if(version >= 9)
        {
            QCOMPARE(loader->transforms(), QStringList{u"RemoveLeaves where $NodeDegree < 1"_s});
            QCOMPARE(loader->visualisations(), QStringList{u"NodeDegree Size"_s});
        }
        else
        {
            // Older names are upgraded where they are, keeping their quotes
            QCOMPARE(loader->transforms(), QStringList{uR"("RemoveLeaves" where $"NodeDegree" < 1)"_s});
            QCOMPARE(loader->visualisations(), QStringList{uR"("NodeDegree" "Size")"_s});
        }

        QCOMPARE(loader->bookmarks().size(), 1);
        QCOMPARE(loader->bookmarks().at(u"Ends"_s),
            (NodeIdSet{model->nodeId(u"a"_s), model->nodeId(u"c"_s)}));

        QCOMPARE(loader->log(), u"Log text"_s);

        QCOMPARE(loader->enrichmentTableData().size(), 1);
        const auto& enrichmentTable = loader->enrichmentTableData().at(0);
        QCOMPARE(enrichmentTable._table.size(), 1);
        QCOMPARE(enrichmentTable._table.at(0), (EnrichmentTableModel::Row{u"x"_s, 0.25}));
        QCOMPARE(enrichmentTable._selectionA, u"A"_s);
        QCOMPARE(enrichmentTable._selectionB, u"B"_s);

        QCOMPARE(model->graphModel().nodeSize(), 0.5F);
        QCOMPARE(model->graphModel().edgeSize(), 0.25F);
        QCOMPARE(model->graphModel().textSize(), 0.75F);

        if(version >= 2)
            QCOMPARE(jsonString(json::parse(loader->uiData())), jsonString(json({{"uiKey", "uiValue"}})));
        else
            QVERIFY(loader->uiData().isEmpty());

        QCOMPARE(jsonString(json::parse(loader->pluginUiData())),
            jsonString(json({{"pluginUiKey", "pluginUiValue"}})));
        QCOMPARE(loader->pluginUiDataVersion(), 1);

        QCOMPARE(jsonString(json::parse(pluginInstance.loadedData())["pluginKey"]),
            jsonString("pluginValue"));
        QCOMPARE(pluginInstance.loadedDataVersion(), 1);
    }

    // A file saved by a release of the application, with gzip compression
    void tutorial()
    {
        auto content = readFile(QFINDTESTDATA("../../app/examples/Tutorial.graphia"));
        QVERIFY(!content.isEmpty());

        TestPluginInstance pluginInstance(&_plugin);
        QString failureReason;
        auto model = load(content, pluginInstance, &failureReason);
        QVERIFY2(model != nullptr, qPrintable(failureReason));

        const auto* loader = model->parser<Loader>();

        QCOMPARE(model->graph().numNodes(), 394);
        QCOMPARE(model->graph().numEdges(), 602);
        QVERIFY(model->nodeNames().contains(u"Oxford_Circus"_s));
        QVERIFY(model->edgeNames().contains(u"Oxford_Circus-Bond_Street"_s));
        QCOMPARE(model->nodeValue(u"Node Name"_s, u"Marble_Arch"_s), u"Marble_Arch"_s);

        QCOMPARE(loader->layoutName(), u"ForceDirected"_s);
        QCOMPARE(loader->nodePositions()->get(model->nodeId(u"Oxford_Circus"_s)),
            QVector3D(50.156368255615234F, -43.49605941772461F, 39.6646614074707F));
        QVERIFY(loader->transforms().isEmpty());
        QCOMPARE(jsonString(json::parse(loader->pluginUiData())),
            jsonString(json({{"sortColumn", 0}, {"sortOrder", 0}})));
    }

    void canOpen()
    {
        const QTemporaryDir dir;

        auto write = [&dir](const QString& name, const QByteArray& content)
        {
            auto fileName = dir.filePath(name);
            QFile file(fileName);
            if(file.open(QIODevice::WriteOnly))
                file.write(content);

            return QUrl::fromLocalFile(fileName);
        };

        auto tutorialUrl = QUrl::fromLocalFile(QFINDTESTDATA("../../app/examples/Tutorial.graphia"));
        QVERIFY(Loader::canOpen(tutorialUrl));
        QCOMPARE(Loader::pluginNameFor(tutorialUrl), u"Generic"_s);

        auto nativeUrl = write(u"native"_s, nativeFile(header(NativeSaver::Version), body(NativeSaver::Version)));
        QVERIFY(Loader::canOpen(nativeUrl));
        QCOMPARE(Loader::pluginNameFor(nativeUrl), u"Test"_s);

        auto pairwiseUrl = write(u"pairwise"_s, "a b\nb c\n");
        QVERIFY(!Loader::canOpen(pairwiseUrl));
        QVERIFY(Loader::pluginNameFor(pairwiseUrl).isEmpty());

        auto noVersionUrl = write(u"noVersion"_s, nativeFile({{"pluginName", "Test"}, {"pluginDataVersion", 1}},
            body(NativeSaver::Version)));
        QVERIFY(!Loader::canOpen(noVersionUrl));
    }

    void invalid_data()
    {
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<QString>("failureReason");

        const auto version = NativeSaver::Version;

        QTest::newRow("Empty") << QByteArray() << QString();
        QTest::newRow("Not JSON") << QByteArray("[{\"version\": 8, \"pluginName\": \"Test\", "
            "\"pluginDataVersion\": 1}, {") << QString();
        QTest::newRow("No body") << QByteArray::fromStdString(json::array({header(version)}).dump()) << QString();
        QTest::newRow("Body not an object") << nativeFile(header(version), json::array()) << QString();

        auto noGraph = body(version);
        noGraph.erase("graph");
        QTest::newRow("No graph") << nativeFile(header(version), noGraph) << QString();

        auto noPluginData = body(version);
        noPluginData.erase("pluginData");
        QTest::newRow("No plugin data") << nativeFile(header(version), noPluginData) << QString();

        auto uiNotObject = body(version);
        uiNotObject["ui"] = 1;
        QTest::newRow("UI not an object") << nativeFile(header(version), uiNotObject) << QString();

        auto edgeToNowhere = body(version);
        edgeToNowhere["graph"]["edges"].push_back({{"id", "2"}, {"source", "2"}, {"target", "3"}});
        QTest::newRow("Edge to nowhere") << nativeFile(header(version), edgeToNowhere) << QString();

        QTest::newRow("Newer version") << nativeFile(header(version + 1), body(version)) <<
            u"newer version of"_s;
        QTest::newRow("Newer plugin data version") << nativeFile(header(version, 2), body(version)) <<
            u"newer version of the plugin"_s;

        auto tutorial = readFile(QFINDTESTDATA("../../app/examples/Tutorial.graphia"));
        QTest::newRow("Truncated compressed") << tutorial.left(tutorial.size() / 2) << QString();
    }

    void invalid()
    {
        QFETCH(const QByteArray, content);
        QFETCH(const QString, failureReason);

        TestPluginInstance pluginInstance(&_plugin);
        QString actualFailureReason;
        QVERIFY(load(content, pluginInstance, &actualFailureReason) == nullptr);
        QVERIFY(!actualFailureReason.isEmpty());

        if(!failureReason.isEmpty())
            QVERIFY2(actualFailureReason.contains(failureReason), qPrintable(actualFailureReason));
    }
};

QTEST_MAIN(TestNativeLoader)
#include "tst_nativeloader.moc"
