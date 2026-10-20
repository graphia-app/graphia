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

#include "app/application.h"
#include "app/attributes/attributeedits.h"
#include "app/attributes/enrichmenttablemodel.h"
#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"
#include "app/graph/mutablegraph.h"
#include "app/ui/document.h"
#include "app/ui/qml/Graphia/availableattributesmodel.h"
#include "app/ui/qml/Graphia/editattributetablemodel.h"
#include "app/ui/qml/Graphia/findtypes.h"
#include "app/ui/qml/Graphia/graphdisplay.h"
#include "app/ui/qml/Graphia/importattributeskeydetection.h"
#include "app/ui/selectionmanager.h"

#include "shared/loading/tabulardata.h"

#include <QFile>
#include <QMetaObject>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
// a -> b -> c -> d, with a -> c and e on its own
const QByteArray Gml = R"(graph
[
    directed 1
    node [ id 0 label "a" score 1 kind "x" ]
    node [ id 1 label "b" score 2 kind "x" ]
    node [ id 2 label "c" score 3 kind "x" ]
    node [ id 3 label "d" score 4 kind "y" ]
    node [ id 4 label "e" score 5 kind "y" ]
    edge [ source 0 target 1 ]
    edge [ source 1 target 2 ]
    edge [ source 2 target 3 ]
    edge [ source 0 target 2 ]
])";

const int Timeout = 60000;
} // namespace

// A Document, as the UI uses it, but with a GraphDisplay that isn't in a window, so
// there's no rendering
class TestDocument_ : public QObject
{
    Q_OBJECT

    QTemporaryDir _dir;
    std::unique_ptr<Application> _application;
    std::unique_ptr<GraphDisplay> _graphDisplay;
    std::unique_ptr<Document> _document;

    QUrl write(const QString& fileName, const QByteArray& content)
    {
        auto filePath = _dir.filePath(fileName);
        QFile file(filePath);
        if(!file.open(QIODevice::WriteOnly) || file.write(content) != content.size())
            return {};

        return QUrl::fromLocalFile(filePath);
    }

    bool open(const QUrl& url, const QString& type, const QString& pluginName)
    {
        // Destroy any previous Document first, as the UI does when its tab is closed
        _document.reset();

        _graphDisplay = std::make_unique<GraphDisplay>();
        _document = std::make_unique<Document>();
        _document->setProperty("application", QVariant::fromValue(_application.get()));
        _document->setProperty("graphDisplay", QVariant::fromValue(_graphDisplay.get()));

        QSignalSpy loadComplete(_document.get(), &Document::loadComplete);
        if(!_document->openUrl(url, type, pluginName, {}))
            return false;

        if(!QTest::qWaitFor([&] { return !loadComplete.isEmpty(); }, Timeout))
            return false;

        if(!loadComplete.at(0).at(1).toBool())
            return false;

        // As the renderer does once it has been created, which is otherwise never, as
        // the display isn't in a window; until then, the Document is always busy. The
        // Document only learns of this once events are processed, and pauses the layout
        // when commands start only if it has, so wait for it, as there is in the UI
        QSignalSpy busyChanged(_document.get(), &Document::busyChanged);
        QMetaObject::invokeMethod(_graphDisplay.get(), "onRendererInitialised");

        return QTest::qWaitFor([&] { return !busyChanged.isEmpty() && !_document->busy(); }, Timeout);
    }

    bool open()
    {
        return open(write(u"graph.gml"_s, Gml), u"GML"_s, u"Generic"_s);
    }

    bool waitForCommands()
    {
        return QTest::qWaitFor([this] { return !_document->busy(); }, Timeout);
    }

    // Does something that executes commands, then waits for them to finish
    template<typename Fn>
    bool execute(const Fn& fn)
    {
        QSignalSpy commandsFinished(_document.get(), &Document::commandsFinished);
        fn();

        return QTest::qWaitFor([&] { return !commandsFinished.isEmpty(); }, Timeout) && waitForCommands();
    }

    GraphModel* graphModel() { return dynamic_cast<GraphModel*>(_document->graphModel()); }
    const Graph& graph() { return graphModel()->graph(); }

    NodeId nodeId(const QString& name)
    {
        for(auto nodeId : graphModel()->mutableGraph().nodeIds())
        {
            if(graphModel()->nodeName(nodeId) == name)
                return nodeId;
        }

        return {};
    }

    // The names of the selected nodes, sorted
    QStringList selected()
    {
        QStringList names;
        for(auto nodeId : _document->selectionManager()->selectedNodes())
            names.append(graphModel()->nodeName(nodeId));

        names.sort();
        return names;
    }

    // The names of the nodes in the transformed graph, sorted
    QStringList nodeNames()
    {
        QStringList names;
        for(auto nodeId : graph().nodeIds())
            names.append(graphModel()->nodeName(nodeId));

        names.sort();
        return names;
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        QVERIFY(_dir.isValid());

        Application::setAppDir(QStringLiteral(GRAPHIA_BUILD_DIR));
        _application = std::make_unique<Application>(nullptr, false);
    }

    void cleanup()
    {
        _document.reset();
        _graphDisplay.reset();
    }

    void cleanupTestCase()
    {
        _application.reset();
    }

    void load()
    {
        QVERIFY(open());

        QCOMPARE(graph().numNodes(), 5u);
        QCOMPARE(graph().numEdges(), 4u);
        QCOMPARE(graph().numComponents(), 2u);
        QVERIFY(!_document->canUndo());
    }

    void applyTransformUndoRedo()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({u"RemoveNodes where $score > 3"_s}); }));
        QCOMPARE(graph().numNodes(), 3u);
        QCOMPARE(_document->transforms().size(), 1);
        QVERIFY(_document->canUndo());

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(graph().numNodes(), 5u);
        QVERIFY(_document->transforms().isEmpty());
        QVERIFY(_document->canRedo());

        QVERIFY(execute([this] { _document->redo(); }));
        QCOMPARE(graph().numNodes(), 3u);
        QCOMPARE(_document->transforms().size(), 1);
    }

    void selection()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->selectAll(); }));
        QCOMPARE(_document->property("numNodesSelected").toInt(), 5);

        QVERIFY(execute([this] { _document->selectNone(); }));
        QCOMPARE(_document->property("numNodesSelected").toInt(), 0);

        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"b"_s)); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        QVERIFY(execute([this] { _document->invertSelection(); }));
        QCOMPARE(selected(), (QStringList{u"d"_s, u"e"_s}));

        QVERIFY(execute([this] { _document->selectSources(nodeId(u"c"_s), false); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        QVERIFY(execute([this] { _document->selectTargets(nodeId(u"a"_s), false); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        QVERIFY(execute([this] { _document->selectTargets(nodeId(u"c"_s), true); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s}));

        // Selection isn't undoable
        QVERIFY(!_document->canUndo());
    }

    void find()
    {
        QVERIFY(open());

        QVERIFY(execute([this]
        {
            _document->find(u"[ab]"_s, static_cast<int>(FindOptions::MatchUsingRegex),
                {u"NodeName"_s}, static_cast<int>(FindSelectStyle::All));
        }));

        QCOMPARE(_document->property("numNodesFound").toInt(), 2);
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s}));

        QVERIFY(execute([this] { _document->resetFind(); }));
        QCOMPARE(_document->property("numNodesFound").toInt(), 0);
    }

    void deleteNodes()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->deleteNode(nodeId(u"c"_s)); }));
        QCOMPARE(nodeNames(), (QStringList{u"a"_s, u"b"_s, u"d"_s, u"e"_s}));
        QCOMPARE(graph().numEdges(), 1u);
        QVERIFY(_document->property("saveRequired").toBool());

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(graph().numNodes(), 5u);
        QCOMPARE(graph().numEdges(), 4u);

        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"a"_s), false); }));
        QVERIFY(execute([this] { _document->deleteSelectedNodes(); }));
        QCOMPARE(nodeNames(), (QStringList{u"d"_s, u"e"_s}));
        QCOMPARE(_document->property("numNodesSelected").toInt(), 0);
    }

    void bookmarks()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"d"_s), false); }));
        _document->addBookmark(u"cd"_s);
        QCOMPARE(_document->bookmarks(), QStringList{u"cd"_s});

        QVERIFY(execute([this] { _document->selectNone(); }));
        QVERIFY(execute([this] { _document->gotoBookmark(u"cd"_s); }));
        QCOMPARE(selected(), (QStringList{u"c"_s, u"d"_s}));

        _document->renameBookmark(u"cd"_s, u"Renamed"_s);
        QCOMPARE(_document->bookmarks(), QStringList{u"Renamed"_s});

        _document->removeBookmarks({u"Renamed"_s});
        QVERIFY(_document->bookmarks().isEmpty());
    }

    void attributes()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->cloneAttribute(u"score"_s, u"Copy"_s); }));
        QVERIFY(_document->attributeExists(u"Copy"_s));
        QCOMPARE(_document->allAttributeValues(u"Copy"_s).size(), 5);

        QVERIFY(execute([this] { _document->removeAttributes({u"score"_s}); }));
        QVERIFY(!_document->attributeExists(u"score"_s));

        QVERIFY(execute([this] { _document->undo(); }));
        QVERIFY(_document->attributeExists(u"score"_s));

        QVERIFY(execute([this] { _document->undo(); }));
        QVERIFY(!_document->attributeExists(u"Copy"_s));
    }

    // Editing an attribute that a transform refers to applies the transforms again
    void editAttributeUsedByTransform()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({u"RemoveNodes where $score > 3"_s}); }));
        QCOMPARE(nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        AttributeEdits edits;
        edits.setNodeValue(nodeId(u"a"_s), u"10"_s);
        QVERIFY(execute([&] { _document->editAttribute(u"score"_s, edits, ValueType::Unknown, {}); }));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s}));

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        // The transforms are unchanged throughout
        QCOMPARE(_document->transforms().size(), 1);
    }

    void visualisations()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({}, {u"score Colour"_s}); }));
        QCOMPARE(_document->visualisations().size(), 1);
        QVERIFY(_document->visualisations().at(0).startsWith(u"score Colour"_s));
        QVERIFY(_document->hasVisualisationInfo());

        QVERIFY(execute([this] { _document->undo(); }));
        QVERIFY(_document->visualisations().isEmpty());
    }

    // As the transform panel edits them; the edits are only applied when the panel applies them
    void transformsEdited()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({u"RemoveNodes where $score > 3"_s}); }));
        QVERIFY(execute([this] { _document->update({uR"(RemoveNodes where $NodeName == "a")"_s}); }));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s}));

        auto applyFromUI = [this]
        {
            _document->apply(_document->property("transforms").toStringList(),
                _document->property("visualisations").toStringList());
        };

        _document->setGraphTransform(0, u"RemoveNodes where $score > 4"_s);
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s}));
        QVERIFY(execute(applyFromUI));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s, u"d"_s}));

        _document->removeGraphTransform(0);
        QVERIFY(execute(applyFromUI));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s, u"d"_s, u"e"_s}));
        QCOMPARE(_document->transforms().size(), 1);

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s, u"d"_s}));

        const auto transforms = _document->transforms();
        QVERIFY(execute([this] { _document->moveGraphTransform(0, 1); }));
        QCOMPARE(_document->transforms(), (QStringList{transforms.at(1), transforms.at(0)}));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s, u"d"_s}));
    }

    // What's saved is what's loaded, including the transforms, visualisations and bookmarks
    void saveAndLoad()
    {
        QVERIFY(open());

        QVERIFY(execute([this]
        {
            _document->update({u"RemoveNodes where $score > 4"_s}, {u"score Size"_s});
        }));

        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"a"_s), false); }));
        _document->addBookmark(u"Around a"_s);

        const auto transforms = _document->transforms();
        const auto visualisations = _document->visualisations();

        auto url = QUrl::fromLocalFile(_dir.filePath(u"saved.%1"_s.arg(Application::nativeExtension())));
        QSignalSpy saveComplete(_document.get(), &Document::saveComplete);
        QVERIFY(execute([&] { _document->saveFile(url, Application::name(), {}, {}); }));
        QVERIFY(QTest::qWaitFor([&] { return !saveComplete.isEmpty(); }, Timeout));
        QVERIFY(saveComplete.at(0).at(0).toBool());
        QVERIFY(!_document->property("saveRequired").toBool());

        QVERIFY(open(url, QString::fromLatin1(Application::NativeFileType), {}));

        QCOMPARE(nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s}));
        QCOMPARE(_document->transforms(), transforms);
        QCOMPARE(_document->visualisations(), visualisations);
        QCOMPARE(_document->bookmarks(), QStringList{u"Around a"_s});
        QVERIFY(execute([this] { _document->gotoBookmark(u"Around a"_s); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));
    }

    void loadFailure()
    {
        QVERIFY(!open(write(u"broken.gml"_s, "graph [ node [ id 0 "), u"GML"_s, u"Generic"_s));
        QVERIFY(!_document->property("failureReason").toString().isEmpty());

        QVERIFY(!open(write(u"graph.gml"_s, Gml), u"GML"_s, u"Nothing"_s));
    }

    // The table in the dialog for editing an attribute's values
    void editAttributeTable()
    {
        QVERIFY(open());

        EditAttributeTableModel model;
        model.setProperty("document", QVariant::fromValue(_document.get()));
        model.setProperty("attributeName", u"kind"_s);

        auto value = [&model](int row, int column)
            { return model.data(model.index(row, column), Qt::DisplayRole); };

        // Sorted by name, by default
        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(value(0, 0).toString(), u"a"_s);
        QCOMPARE(value(4, 0).toString(), u"e"_s);
        QCOMPARE(value(3, 1).toString(), u"y"_s);

        model.setProperty("ascendingSortOrder", false);
        QCOMPARE(value(0, 0).toString(), u"e"_s);
        model.setProperty("ascendingSortOrder", true);

        model.editValue(1, u"z"_s);
        QVERIFY(model.property("hasEdits").toBool());
        QVERIFY(model.rowIsEdited(1));
        QVERIFY(!model.rowIsEdited(0));
        QCOMPARE(value(1, 1).toString(), u"z"_s);

        model.resetRowValue(1);
        QVERIFY(!model.property("hasEdits").toBool());
        QCOMPARE(value(1, 1).toString(), u"x"_s);

        // Editing a value back to what it was is no edit
        model.editValue(1, u"z"_s);
        model.editValue(1, u"x"_s);
        QCOMPARE(value(1, 1).toString(), u"x"_s);
        QVERIFY(!model.rowIsEdited(1));
        QVERIFY(!model.property("hasEdits").toBool());

        // Numbers too, though what's typed is a string
        model.setProperty("attributeName", u"score"_s);
        model.editValue(1, u"9"_s);
        model.editValue(1, u"2"_s);
        QVERIFY(!model.rowIsEdited(1));
        model.setProperty("attributeName", u"kind"_s);

        // Only the selected nodes, if any
        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"d"_s), false); }));
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(value(0, 0).toString(), u"c"_s);
        QVERIFY(execute([this] { _document->selectNone(); }));
        QCOMPARE(model.rowCount(), 5);

        // Each distinct value, with how many nodes have it
        model.setProperty("combineSharedValues", true);
        QCOMPARE(model.rowCount(), 2);

        const int xRow = value(0, 1).toString() == u"x"_s ? 0 : 1;
        QCOMPARE(value(xRow, 0).toInt(), 3);
        QCOMPARE(value(1 - xRow, 0).toInt(), 2);

        model.editValue(xRow, u"x"_s);
        QVERIFY(!model.rowIsEdited(xRow));

        model.editValue(xRow, u"w"_s);
        QVERIFY(model.rowIsEdited(xRow));

        // Applying the edits, as the dialog does
        const auto edits = model.property("edits").value<AttributeEdits>();
        QCOMPARE(edits.nodeValues().size(), 3u);
        QVERIFY(execute([&] { _document->editAttribute(u"kind"_s, edits, ValueType::Unknown, {}); }));

        const auto values = _document->allAttributeValues(u"kind"_s);
        QCOMPARE(values.count(u"w"_s), 3);
        QCOMPARE(values.count(u"y"_s), 2);
    }

    // Finding which column of a table to match with which attribute, then importing it
    void importAttributes()
    {
        QVERIFY(open());

        auto data = std::make_shared<TabularData>();
        const QList<QStringList> rows
        {
            {u"Id"_s,   u"Group"_s,   u"Value"_s},
            {u"a"_s,    u"p"_s,       u"1.5"_s},
            {u"b"_s,    u"q"_s,       u"2.5"_s},
            {u"c"_s,    u"p"_s,       u"3.5"_s},
            {u"z"_s,    u"q"_s,       u"4.5"_s},
        };

        data->reserve(3, static_cast<size_t>(rows.size()));
        for(qsizetype row = 0; row < rows.size(); row++)
        {
            for(qsizetype column = 0; column < 3; column++)
                data->setValueAt(static_cast<size_t>(column), static_cast<size_t>(row), rows.at(row).at(column));
        }

        ImportAttributesKeyDetection keyDetection;
        keyDetection.setProperty("document", QVariant::fromValue(_document.get()));
        keyDetection.setProperty("tabularData", QVariant::fromValue(data));

        QSignalSpy resultChanged(&keyDetection, &ImportAttributesKeyDetection::resultChanged);
        keyDetection.start();
        QVERIFY(QTest::qWaitFor([&] { return !resultChanged.isEmpty() && !keyDetection.busy(); }, Timeout));

        // Three of the five nodes are in the table
        const auto result = keyDetection.property("result").toMap();
        QCOMPARE(result.value(u"attributeName"_s).toString(), u"NodeName"_s);
        QCOMPARE(result.value(u"column"_s).toInt(), 0);
        QCOMPARE(result.value(u"percent"_s).toInt(), 60);

        QVERIFY(execute([&] { _document->importAttributesFromTable(u"NodeName"_s, data, 0, {2}, false); }));
        QVERIFY(_document->attributeExists(u"Value"_s));
        QCOMPARE(graphModel()->attributeValueByName(u"Value"_s).numericValueOf(nodeId(u"c"_s)), 3.5);
        QVERIFY(graphModel()->attributeValueByName(u"Value"_s).valueMissingOf(nodeId(u"d"_s)));

        QVERIFY(execute([this] { _document->undo(); }));
        QVERIFY(!_document->attributeExists(u"Value"_s));

        keyDetection.reset();
        QVERIFY(keyDetection.property("result").toMap().isEmpty());
    }

    // Enrichment results are kept with the Document, and saved with it
    void enrichment()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->performEnrichment(u"kind"_s, u"kind"_s); }));

        auto tableModels = _document->property("enrichmentTableModels").toList();
        QCOMPARE(tableModels.size(), 1);

        auto* tableModel = tableModels.at(0).value<EnrichmentTableModel*>();
        QVERIFY(tableModel != nullptr);
        const auto rowCount = tableModel->rowCount();
        QVERIFY(rowCount > 0);
        QVERIFY(_document->property("saveRequired").toBool());

        auto url = QUrl::fromLocalFile(_dir.filePath(u"enrichment.%1"_s.arg(Application::nativeExtension())));
        QVERIFY(execute([&] { _document->saveFile(url, Application::name(), {}, {}); }));
        QVERIFY(open(url, QString::fromLatin1(Application::NativeFileType), {}));

        // The results are added once loading is complete
        QVERIFY(QTest::qWaitFor([this]
            { return !_document->property("enrichmentTableModels").toList().isEmpty(); }, Timeout));

        tableModels = _document->property("enrichmentTableModels").toList();
        QCOMPARE(tableModels.size(), 1);
        QCOMPARE(tableModels.at(0).value<EnrichmentTableModel*>()->rowCount(), rowCount);

        _document->removeEnrichmentResults(0);
        QVERIFY(_document->property("enrichmentTableModels").toList().isEmpty());
    }

    // What the transform and visualisation panels ask of the Document
    void queries()
    {
        QVERIFY(open());

        auto numerical = _document->availableAttributeNames(static_cast<int>(ElementType::Node),
            static_cast<int>(ValueType::Numerical));
        QVERIFY(numerical.contains(u"score"_s));
        QVERIFY(numerical.contains(u"NodeDegree"_s));
        QVERIFY(!numerical.contains(u"kind"_s));

        // Edges can be filtered by their nodes' attributes
        auto edge = _document->availableAttributeNames(static_cast<int>(ElementType::Edge),
            static_cast<int>(ValueType::All));
        QVERIFY(edge.contains(u"source.kind"_s));
        QVERIFY(edge.contains(u"target.score"_s));

        auto score = _document->attribute(u"score"_s);
        QVERIFY(score.value(u"isValid"_s).toBool());
        QVERIFY(score.value(u"userDefined"_s).toBool());
        QCOMPARE(score.value(u"minimumValue"_s).toDouble(), 1.0);
        QCOMPARE(score.value(u"maximumValue"_s).toDouble(), 5.0);
        QVERIFY(score.value(u"ops"_s).toStringList().contains(u">"_s));

        auto sourceKind = _document->attribute(u"source.kind"_s);
        QCOMPARE(sourceKind.value(u"name"_s).toString(), u"source.kind"_s);
        QCOMPARE(sourceKind.value(u"sharedValues"_s).toStringList(), (QStringList{u"x"_s, u"y"_s}));

        QVERIFY(!_document->attribute(u"Not an attribute"_s).value(u"isValid"_s).toBool());

        auto removeNodes = _document->transform(u"RemoveNodes"_s);
        QVERIFY(removeNodes.value(u"requiresCondition"_s).toBool());
        QVERIFY(!removeNodes.value(u"description"_s).toString().isEmpty());

        auto pageRank = _document->transform(u"PageRank"_s);
        QVERIFY(!pageRank.value(u"requiresCondition"_s).toBool());
        QVERIFY(!pageRank.value(u"defaultVisualisations"_s).toMap().isEmpty());

        QVERIFY(_document->transform(u"Not a transform"_s).isEmpty());

        const auto transform = uR"(RemoveNodes where $score > 3 and $kind == "x")"_s;
        QVERIFY(_document->graphTransformIsValid(transform));
        QVERIFY(!_document->graphTransformIsValid(u"RemoveNodes where"_s));
        auto referenced = _document->attributeNamesReferencedBy(transform);
        referenced.sort();
        QCOMPARE(referenced, (QStringList{u"kind"_s, u"score"_s}));
        QVERIFY(!_document->displayTextForGraphTransform(transform).isEmpty());

        QVERIFY(_document->availableVisualisationChannelNames(static_cast<int>(ElementType::Node),
            static_cast<int>(ValueType::Int)).contains(u"Colour"_s));
        QVERIFY(_document->visualisationIsValid(u"score Size"_s));
        QVERIFY(!_document->visualisationIsValid(u"score Nothing"_s));

        std::unique_ptr<AvailableAttributesModel> model(_document->availableAttributesModel(
            static_cast<int>(ElementType::Node), static_cast<int>(ValueType::Numerical)));
        QVERIFY(model != nullptr);
        QVERIFY(model->rowCount() > 0);
    }

    // Tables, such as those of node attributes or enrichment results, exported to files
    void exportTable_data()
    {
        QTest::addColumn<QString>("extension");
        QTest::addColumn<QString>("expected");

        QTest::newRow("CSV") << u"csv"_s <<
            u"Name,\"Value, in units\"\r\n"
            "plain,1.5\r\n"
            "\"a,b\",42\r\n"
            "\"say \"\"hi\"\"\",\r\n"_s;

        QTest::newRow("TSV") << u"tsv"_s <<
            u"Name\tValue, in units\r\n"
            "plain\t1.5\r\n"
            "a,b\t42\r\n"
            "say \"hi\"\t\r\n"_s;
    }

    void exportTable()
    {
        QFETCH(QString, extension);
        QFETCH(const QString, expected);

        QVERIFY(open());

        QStandardItemModel model(0, 2);
        auto appendRow = [&model](const QString& name, const QVariant& value)
        {
            auto* nameItem = new QStandardItem(name);
            auto* valueItem = new QStandardItem;
            valueItem->setData(value, Qt::DisplayRole);
            model.appendRow({nameItem, valueItem});
        };

        appendRow(u"plain"_s, 1.5);
        appendRow(u"a,b"_s, 42);
        appendRow(u"say \"hi\""_s, {});

        auto fileName = _dir.filePath(u"table.%1"_s.arg(extension));
        QVERIFY(execute([&]
        {
            _document->writeTableModelToFile(&model, QUrl::fromLocalFile(fileName),
                extension, {u"Name"_s, u"Value, in units"_s});
        }));

        QFile file(fileName);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(QString::fromUtf8(file.readAll()), expected);
    }
};

QTEST_MAIN(TestDocument_)
#include "tst_document.moc"
