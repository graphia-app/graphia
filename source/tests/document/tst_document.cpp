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

#include "tests/common/documenttest.h"
#include "tests/common/testenvironment.h"
#include "tests/common/xlsxwriter.h"

#include "app/attributes/attributeedits.h"
#include "app/attributes/enrichmenttablemodel.h"
#include "app/layout/nodepositions.h"
#include "app/ui/qml/Graphia/availableattributesmodel.h"
#include "app/ui/qml/Graphia/alert.h"
#include "app/ui/qml/Graphia/availabletransformsmodel.h"
#include "app/ui/qml/Graphia/editattributetablemodel.h"
#include "app/ui/qml/Graphia/findtypes.h"
#include "app/ui/qml/Graphia/importattributeskeydetection.h"
#include "app/ui/qml/Graphia/graphrenderertypes.h"
#include "app/ui/qml/Graphia/limitconstants.h"
#include "app/ui/qml/Graphia/Utils/preferences.h"

#include "shared/loading/tabulardata.h"
#include "shared/utils/container.h"
#include "shared/utils/json.h"

#include <QAbstractItemModel>
#include <QClipboard>
#include <QGuiApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QUrl>
#include <QVector3D>

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <vector>

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
// Components of three, two and one nodes: a -> b -> c, d -> e and f
const QByteArray ThreeComponentsGml = R"(graph
[
    directed 1
    node [ id 0 label "a" ]
    node [ id 1 label "b" ]
    node [ id 2 label "c" ]
    node [ id 3 label "d" ]
    node [ id 4 label "e" ]
    node [ id 5 label "f" ]
    edge [ source 0 target 1 ]
    edge [ source 1 target 2 ]
    edge [ source 3 target 4 ]
])";

// a -> b -> c, in other formats
const QByteArray GraphML =
    "<?xml version=\"1.0\"?>\n"
    "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\">\n"
    "  <graph edgedefault=\"directed\">\n"
    "    <node id=\"a\"/><node id=\"b\"/><node id=\"c\"/>\n"
    "    <edge source=\"a\" target=\"b\"/><edge source=\"b\" target=\"c\"/>\n"
    "  </graph>\n"
    "</graphml>\n";

const QByteArray JsonGraph =
    "{\"graph\": {\"nodes\": [{\"id\": \"a\"}, {\"id\": \"b\"}, {\"id\": \"c\"}], "
    "\"edges\": [{\"source\": \"a\", \"target\": \"b\"}, {\"source\": \"b\", \"target\": \"c\"}]}}";
} // namespace

// Document, as the UI uses it
class TestDocument_ : public DocumentTest
{
    Q_OBJECT

    using DocumentTest::open;

    bool open()
    {
        return open(write(u"graph.gml"_s, Gml), u"GML"_s, u"Generic"_s);
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        QVERIFY(createApplication());
    }

    void cleanup()
    {
        closeDocument();
    }

    void cleanupTestCase()
    {
        destroyApplication();
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

    // Nodes merged by a transform are shown as one, so are deleted together, as when selected
    void deleteMergedNode()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({uR"(ContractEdges where $source.NodeName == "a")"_s}); }));
        QCOMPARE(graph().mergedNodeIdsForNodeId(nodeId(u"a"_s)).size(), 3u);

        QVERIFY(execute([this] { _document->deleteNode(nodeId(u"a"_s)); }));
        QCOMPARE(nodeNames(), (QStringList{u"d"_s, u"e"_s}));

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s}));
        QCOMPARE(graphModel()->mutableGraph().numEdges(), 4u);
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

    // As the context menu's "Select Shared Values", and selecting by an attribute's value
    void selectByValue()
    {
        QVERIFY(open());

        // The nodes that share d's kind
        QVERIFY(execute([this] { _document->selectBySharedAttributeValue(u"kind"_s, nodeId(u"d"_s), false); }));
        QCOMPARE(selected(), (QStringList{u"d"_s, u"e"_s}));

        // ...and, adding the selection's, a's score
        QVERIFY(execute([this] { _document->selectBySharedAttributeValue(u"score"_s, nodeId(u"a"_s), true); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"d"_s, u"e"_s}));

        // The selection's
        QVERIFY(execute([this] { _document->selectBySharedAttributeValue(u"kind"_s); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s}));

        QVERIFY(execute([this] { _document->selectByAttributeValue(u"kind"_s, u"^y$"_s); }));
        QCOMPARE(selected(), (QStringList{u"d"_s, u"e"_s}));

        QVERIFY(execute([this] { _document->selectByAttributeValue(u"score"_s, u"^[12]$"_s); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s}));

        // Nothing matches, or there's no such attribute, so nothing is selected
        _document->selectByAttributeValue(u"kind"_s, u"z"_s);
        QVERIFY(QTest::qWaitFor([this] { return selected().isEmpty(); }));

        QVERIFY(execute([this] { _document->selectAll(); }));
        _document->selectByAttributeValue(u"Not an attribute"_s, u"x"_s);
        QVERIFY(QTest::qWaitFor([this] { return selected().isEmpty(); }));
    }

    // Selecting by the values an attribute shows, rather than those it stores
    void selectByDisplayValue()
    {
        QVERIFY(open(write(u"graph.gml"_s, ThreeComponentsGml), u"GML"_s, u"Generic"_s));

        const auto componentOf = [this](const QString& name)
        {
            return u"^%1$"_s.arg(graphModel()->attributeValueByName(u"NodeComponentIdentifier"_s)
                .stringValueOf(nodeId(name)));
        };

        QVERIFY(execute([&] { _document->selectByAttributeValue(u"NodeComponentIdentifier"_s, componentOf(u"d"_s)); }));
        QCOMPARE(selected(), (QStringList{u"d"_s, u"e"_s}));

        // An invalid regex matches nothing
        _document->selectByAttributeValue(u"NodeComponentIdentifier"_s, u"[Component"_s);
        QVERIFY(QTest::qWaitFor([this] { return selected().isEmpty(); }));
    }

    // Stepping between components, as the toolbar's buttons do; there's no renderer here
    // to move to the component, but the display records which it's asked to focus on
    void componentNavigation()
    {
        QVERIFY(open(write(u"graph.gml"_s, ThreeComponentsGml), u"GML"_s, u"Generic"_s));

        const auto abc = graph().componentIdOfNode(nodeId(u"a"_s));
        const auto de = graph().componentIdOfNode(nodeId(u"d"_s));
        const auto f = graph().componentIdOfNode(nodeId(u"f"_s));

        // Largest first
        QCOMPARE(graph().componentIds(), (std::vector<ComponentId>{abc, de, f}));

        // From the overview, the first or last
        _document->gotoNextComponent();
        QCOMPARE(_graphDisplay->desiredFocusComponentId(), abc);
        _document->gotoPrevComponent();
        QCOMPARE(_graphDisplay->desiredFocusComponentId(), f);

        // From a component, the one after or before it, wrapping around
        _graphDisplay->setFocusedComponentId(abc);
        _document->gotoNextComponent();
        QCOMPARE(_graphDisplay->desiredFocusComponentId(), de);
        _document->gotoPrevComponent();
        QCOMPARE(_graphDisplay->desiredFocusComponentId(), f);

        _graphDisplay->setFocusedComponentId(f);
        _document->gotoNextComponent();
        QCOMPARE(_graphDisplay->desiredFocusComponentId(), abc);
        _document->gotoPrevComponent();
        QCOMPARE(_graphDisplay->desiredFocusComponentId(), de);
    }

    // Selecting all of what's shown: everything in the overview, or only the focused component
    void selectAllVisible()
    {
        QVERIFY(open(write(u"graph.gml"_s, ThreeComponentsGml), u"GML"_s, u"Generic"_s));

        QVERIFY(execute([this] { _document->selectAllVisible(); }));
        QCOMPARE(selected().size(), 6);

        // Focused on d -> e, from where the overview can be returned to
        _graphDisplay->setFocusedComponentId(graph().componentIdOfNode(nodeId(u"d"_s)));
        _graphDisplay->setCanEnterOverviewMode(true);

        QVERIFY(execute([this] { _document->selectNone(); }));
        QVERIFY(execute([this] { _document->selectAllVisible(); }));
        QCOMPARE(selected(), (QStringList{u"d"_s, u"e"_s}));
        QCOMPARE(_document->property("numInvisibleNodesSelected").toInt(), 0);

        // The nodes selected in the components that aren't shown
        QVERIFY(execute([this] { _document->selectAll(); }));
        QCOMPARE(_document->property("numInvisibleNodesSelected").toInt(), 4);
    }

    // What the UI shows of the selection
    void selectionInfo()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->selectSources(nodeId(u"c"_s), false); }));
        QCOMPARE(_document->property("numHeadNodesSelected").toInt(), 3);

        const auto selectedNodeIds = _document->property("selectedNodeIds").toList();

        QStringList names;
        names.reserve(selectedNodeIds.size());
        for(const auto& id : selectedNodeIds)
            names.append(_document->nodeName(id.value<QmlNodeId>()));
        names.sort();
        QCOMPARE(names, (QStringList{u"a"_s, u"b"_s, u"c"_s}));
        QCOMPARE(_document->property("selectedHeadNodeIds").toList().size(), 3);

        QVERIFY(_document->nodeIsSelected(nodeId(u"a"_s)));
        QVERIFY(!_document->nodeIsSelected(nodeId(u"d"_s)));

        QVERIFY(_document->property("directed").toBool());
        QVERIFY(_document->property("editable").toBool());

        QCOMPARE(_document->graphSizeSummary(),
            u"Mutable Graph Nodes: 5 Edges: 4\nTransformed Graph Nodes: 5 Edges: 4 Components: 2"_s);
    }

    // Stepping through the nodes found, as the find panel's buttons do
    void findNavigation()
    {
        QVERIFY(open());

        QVERIFY(execute([this]
        {
            _document->find(u"[abd]"_s, static_cast<int>(FindOptions::MatchUsingRegex),
                {u"NodeName"_s}, static_cast<int>(FindSelectStyle::None));
        }));
        QCOMPARE(_document->property("numNodesFound").toInt(), 3);

        // With nothing selected, the first found node is selected
        QCOMPARE(selected(), QStringList{u"a"_s});
        QCOMPARE(_document->property("foundIndex").toInt(), 0);

        // The nodes found are in order, by component, wrapping around
        QVERIFY(execute([this] { _document->selectNextFound(); }));
        QCOMPARE(selected(), QStringList{u"b"_s});
        QVERIFY(execute([this] { _document->selectNextFound(); }));
        QCOMPARE(selected(), QStringList{u"d"_s});
        QCOMPARE(_document->property("foundIndex").toInt(), 2);
        QVERIFY(execute([this] { _document->selectNextFound(); }));
        QCOMPARE(selected(), QStringList{u"a"_s});

        QVERIFY(execute([this] { _document->selectPrevFound(); }));
        QCOMPARE(selected(), QStringList{u"d"_s});

        QVERIFY(execute([this] { _document->selectFirstFound(); }));
        QCOMPARE(selected(), QStringList{u"a"_s});

        QVERIFY(execute([this] { _document->selectAllFound(); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"d"_s}));
        QCOMPARE(_document->property("foundIndex").toInt(), -1);
    }

    // Finding when something is already selected, then finding nothing
    void findWithSelection()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->selectNodes({nodeId(u"b"_s)}); }));

        // The selection is kept, and is where the stepping starts from
        QVERIFY(execute([this]
        {
            _document->find(u"[abd]"_s, static_cast<int>(FindOptions::MatchUsingRegex),
                {u"NodeName"_s}, static_cast<int>(FindSelectStyle::None));
        }));
        QCOMPARE(selected(), QStringList{u"b"_s});
        QCOMPARE(_document->property("foundIndex").toInt(), 1);
        QVERIFY(execute([this] { _document->selectNextFound(); }));
        QCOMPARE(selected(), QStringList{u"d"_s});

        // Finding nothing clears the selection of what was found
        _document->find(u"z"_s, static_cast<int>(FindOptions::MatchUsingRegex),
            {u"NodeName"_s}, static_cast<int>(FindSelectStyle::None));
        QVERIFY(QTest::qWaitFor([this] { return _document->property("numNodesFound").toInt() == 0; }, Timeout));
        QVERIFY(waitForCommands());
        QVERIFY(selected().isEmpty());
        QCOMPARE(_document->property("foundIndex").toInt(), -1);
    }

    void gotoAllBookmarks()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->selectSources(nodeId(u"b"_s), false); }));
        _document->addBookmark(u"ab"_s);
        QVERIFY(execute([this] { _document->selectBySharedAttributeValue(u"kind"_s, nodeId(u"e"_s), false); }));
        _document->addBookmark(u"de"_s);

        QVERIFY(execute([this] { _document->selectNone(); }));
        QVERIFY(execute([this] { _document->gotoAllBookmarks(); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"d"_s, u"e"_s}));

        // Bookmarked nodes that have been removed are skipped
        QVERIFY(execute([this] { _document->update({uR"(RemoveNodes where $NodeName == "a")"_s}); }));
        QVERIFY(execute([this] { _document->selectNone(); }));
        QVERIFY(execute([this] { _document->gotoAllBookmarks(); }));
        QCOMPARE(selected(), (QStringList{u"b"_s, u"d"_s, u"e"_s}));
    }

    // The details the transform and visualisation panels show
    void transformAndVisualisationDetails()
    {
        QVERIFY(open());

        auto k = _document->transformParameter(u"KNN"_s, u"k"_s);
        QCOMPARE(k.value(u"name"_s).toString(), u"k"_s);
        QCOMPARE(k.value(u"displayName"_s).toString(), u"k"_s);
        QCOMPARE(k.value(u"valueType"_s).toInt(), static_cast<int>(ValueType::Int));
        QCOMPARE(k.value(u"hasMinimumValue"_s).toBool(), true);
        QCOMPARE(k.value(u"minimumValue"_s).toInt(), 1);
        QCOMPARE(k.value(u"hasMaximumValue"_s).toBool(), false);
        QCOMPARE(k.value(u"initialValue"_s).toInt(), 5);

        auto rankOrder = _document->transformParameter(u"KNN"_s, u"RankOrder"_s);
        QCOMPARE(rankOrder.value(u"initialIndex"_s).toInt(), 0);
        QCOMPARE(rankOrder.value(u"displayName"_s).toString(), u"Rank Order"_s);
        QVERIFY(_document->transformParameter(u"KNN"_s, u"Not a parameter"_s).isEmpty());
        QVERIFY(_document->transformParameter(u"Not a transform"_s, u"k"_s).isEmpty());

        const auto knn = _document->transform(u"KNN"_s);
        QCOMPARE(knn.value(u"displayName"_s).toString(), u"k-NN"_s);
        QCOMPARE(knn.value(u"attributeParameterNames"_s).toStringList(), QStringList{u"Ranked Attribute"_s});
        const auto rankedAttribute = knn.value(u"attributeParameters"_s).toList().at(0).toMap();
        QCOMPARE(rankedAttribute.value(u"elementType"_s).toInt(), static_cast<int>(ElementType::Edge));
        QCOMPARE(rankedAttribute.value(u"valueType"_s).toInt(), static_cast<int>(ValueType::Numerical));

        const auto parsed = _document->parseGraphTransform(u"RemoveNodes where $score > 3"_s);
        QCOMPARE(parsed.value(u"action"_s).toString(), u"RemoveNodes"_s);
        QCOMPARE(parsed.value(u"displayName"_s).toString(), u"Remove Nodes"_s);
        QVERIFY(_document->parseGraphTransform(u"not a transform"_s).isEmpty());

        QVERIFY(execute([this] { _document->update({u"RemoveNodes where $score > 3"_s}); }));
        QCOMPARE(_document->transformInfoAtIndex(0).value(u"alertType"_s).toInt(), static_cast<int>(AlertType::None));

        // Of the nodes that remain
        QVERIFY(execute([this]
        {
            _document->update({u"RemoveNodes where $score > 3"_s},
                {u"score Size"_s, u"kind Colour"_s});
        }));
        auto info = _document->visualisationInfoAtIndex(0);
        QCOMPARE(info.value(u"minimumNumericValue"_s).toDouble(), 1.0);
        QCOMPARE(info.value(u"maximumNumericValue"_s).toDouble(), 3.0);
        QCOMPARE(info.value(u"hasNumericRange"_s).toBool(), true);
        QCOMPARE(info.value(u"alertType"_s).toInt(), static_cast<int>(AlertType::None));

        auto values = _document->visualisationNumericValuesAtIndex(0);
        std::ranges::sort(values);
        QCOMPARE(values, (QList<double>{1.0, 2.0, 3.0}));

        auto strings = _document->visualisationStringValuesAtIndex(1);
        strings.removeDuplicates();
        QCOMPARE(strings, QStringList{u"x"_s});

        const auto parsedVisualisation = _document->parseVisualisation(u"kind Colour"_s);
        QCOMPARE(parsedVisualisation.value(u"attribute"_s).toString(), u"kind"_s);
        QCOMPARE(parsedVisualisation.value(u"channel"_s).toString(), u"Colour"_s);

        QVERIFY(_document->visualisationsAreValid({u"kind Colour"_s}));
        QVERIFY(!_document->visualisationsAreValid({u"kind Colour"_s, uR"("kind" "Not a channel")"_s}));
        QVERIFY(!_document->visualisationDefaultParameters(static_cast<int>(ValueType::Float), u"Colour"_s).isEmpty());

        // Attributes like another, for the visualisation panel's menu of alternatives
        const std::unique_ptr<AvailableAttributesModel> similar(_document->attributesSimilarTo(u"score"_s, 0));
        QVERIFY(similar != nullptr);
        QVERIFY(_document->attributesSimilarTo(u"Not an attribute"_s, 0) == nullptr);
    }

    // The layout's settings, as the layout settings panel shows them
    void layoutSettings()
    {
        QVERIFY(open());

        const auto names = _document->property("layoutSettingNames").toStringList();
        QVERIFY(!names.isEmpty());

        const auto setting = _document->layoutSetting(names.at(0));
        QCOMPARE(setting.value(u"name"_s).toString(), names.at(0));
        QVERIFY(setting.value(u"minimumValue"_s).toFloat() <= setting.value(u"value"_s).toFloat());
        QVERIFY(setting.value(u"value"_s).toFloat() <= setting.value(u"maximumValue"_s).toFloat());
        QVERIFY(_document->layoutSetting(u"Not a setting"_s).isEmpty());
    }

    // What the Application tells the UI about opening files, and links
    void application()
    {
        auto* application = _application.get();

        const auto gml = write(u"graph.gml"_s, Gml);
        QCOMPARE(application->urlTypesOf(gml), QStringList{u"GML"_s});
        QCOMPARE(application->urlTypesFor(u"gml"_s), QStringList{u"GML"_s});
        QVERIFY(application->urlTypesFor(u"csv"_s).contains(u"CorrelationCSV"_s));
        QVERIFY(application->pluginNames(u"GML"_s).contains(u"Generic"_s));
        QCOMPARE(application->pluginNames(u"CorrelationCSV"_s), QStringList{u"Correlation"_s});
        QVERIFY(application->pluginNames(u"Not a type"_s).isEmpty());

        QVERIFY(application->canOpen(u"GML"_s));
        QVERIFY(application->canOpen(QString::fromLatin1(Application::NativeFileType)));
        QVERIFY(!application->canOpen(u"Not a type"_s));
        QVERIFY(application->canOpenAnyOf({u"Not a type"_s, u"GML"_s}));
        QVERIFY(!application->canOpenAnyOf({u"Not a type"_s}));

        // A file type is identified by its description and extensions, as the open file dialog lists them
        const auto description = application->descriptionForUrlType(u"GML"_s);
        QVERIFY(!description.isEmpty());
        QCOMPARE(application->urlTypeFor(description, {u"gml"_s}), u"GML"_s);
        QVERIFY(application->urlTypeFor(description, {u"xml"_s}).isEmpty());

        // Why a file couldn't be identified, here as its extension doesn't match its content
        const auto tabs = write(u"tabs.csv"_s, "Name\ta\tb\nr1\t1\t2\nr2\t3\t4\n");
        QVERIFY(!application->urlTypesOf(tabs).contains(u"CorrelationCSV"_s));
        const auto reasons = application->failureReasons(tabs);
        QVERIFY(std::ranges::any_of(reasons, [](const auto& reason) { return reason.contains(u"resembles a 'TSV' file"_s); }));
        QVERIFY(application->failureReasons(write(u"unknown.xyz"_s, "?")).isEmpty());

        auto saverNames = [application]
        {
            const auto saverFileTypes = application->saverFileTypes();

            QStringList names;
            names.reserve(saverFileTypes.size());
            for(const auto& saver : saverFileTypes)
                names.append(saver.toMap().value(u"extension"_s).toString());
            return names;
        }();
        QVERIFY(saverNames.contains(u"gml"_s));
        QVERIFY(saverNames.contains(u"graphml"_s));

        const std::unique_ptr<QAbstractItemModel> plugins(application->pluginDetailsModel());
        QCOMPARE(plugins->rowCount(), 3);
        QVERIFY(!plugins->data(plugins->index(0, 0), Qt::DisplayRole).toString().isEmpty());
        const std::unique_ptr<QAbstractItemModel> urlTypes(application->urlTypeDetailsModel());
        QVERIFY(urlTypes->rowCount() > 0);

        // Links to Graphia itself
        const QUrl link(u"%1://v2/open/a%20b/"_s.arg(Application::nativeExtension()));
        QVERIFY(application->isNativeLink(link));
        QCOMPARE(application->linkVersionFor(link), 2);
        QCOMPARE(application->linkArgumentsFor(link), (QStringList{u"open"_s, u"a b"_s}));
        QCOMPARE(application->linkVersionFor(QUrl(u"%1://x/open"_s.arg(Application::nativeExtension()))), -1);
        QVERIFY(!application->isNativeLink(QUrl(u"https://graphia.app"_s)));
        QCOMPARE(application->linkVersionFor(QUrl(u"https://graphia.app"_s)), -1);

        // The files that come with Graphia, here the tests in the build directory
        QVERIFY(application->isResourceFile(QCoreApplication::applicationFilePath()));
        QVERIFY(!application->isResourceFile(gml.toLocalFile()));
        QVERIFY(!application->isResourceFileUrl(gml));

        // Beside the build directory, and in one whose name starts with the build directory's
        for(const auto& name : {u"tst_document-XXXXXX"_s, QFileInfo(QStringLiteral(BUILD_DIR)).fileName() + u"-XXXXXX"_s})
        {
            const QTemporaryDir besideBuildDir(QFileInfo(QStringLiteral(BUILD_DIR)).dir().filePath(name));
            QVERIFY(besideBuildDir.isValid());

            QFile file(besideBuildDir.filePath(u"Tutorial.graphia"_s));
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.close();

            QVERIFY2(!application->isResourceFile(file.fileName()), qPrintable(file.fileName()));
        }
    }

    // The transforms offered when adding one, by category, with the user's favourites first
    void availableTransforms()
    {
        QVERIFY(open());

        const std::unique_ptr<AvailableTransformsModel> model(_document->availableTransforms());
        QVERIFY(model != nullptr);
        QVERIFY(model->roleNames().values().contains("category"));

        auto valueFor = [&model](const QString& transformName, const QByteArray& roleName)
        {
            for(int row = 0; row < model->rowCount({}); row++)
            {
                auto index = model->index(row);
                if(model->get(index).toString() == transformName)
                    return model->data(index, model->roleNames().key(roleName));
            }

            return QVariant{};
        };

        QCOMPARE(model->rowCount({}), static_cast<int>(graphModel()->availableTransformNames().size()));
        QCOMPARE(valueFor(u"PageRank"_s, "category").toString(), u"Metrics"_s);
        QCOMPARE(valueFor(u"PageRank"_s, "isFavourite").toBool(), false);
        QVERIFY(!model->data(model->index(model->rowCount({})), Qt::DisplayRole).isValid());

        QSignalSpy modelReset(model.get(), &AvailableTransformsModel::modelReset);
        u::setPref(u"misc/favouriteTransforms"_s, uR"(["PageRank", 1])"_s);
        QVERIFY(QTest::qWaitFor([&] { return !modelReset.isEmpty(); }));
        QCOMPARE(valueFor(u"PageRank"_s, "category").toString(), u"Favourites"_s);
        QCOMPARE(valueFor(u"PageRank"_s, "isFavourite").toBool(), true);
        QCOMPARE(valueFor(u"Betweenness"_s, "category").toString(), u"Metrics"_s);

        // Not JSON
        u::setPref(u"misc/favouriteTransforms"_s, u"PageRank"_s);
        QTest::qWait(0);
        QCOMPARE(valueFor(u"PageRank"_s, "isFavourite").toBool(), true);

        u::setPref(u"misc/favouriteTransforms"_s, u"[]"_s);
        QVERIFY(QTest::qWaitFor([&] { return modelReset.count() == 2; }));
        QCOMPARE(valueFor(u"PageRank"_s, "isFavourite").toBool(), false);
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

    // As when dragging one in the visualisation panel; there can be more visualisations than transforms
    void moveVisualisation()
    {
        QVERIFY(open());

        const QStringList visualisations
        {
            u"score Colour"_s,
            u"kind Text"_s,
            u"score Size"_s,
        };

        // No transforms
        QVERIFY(execute([&] { _document->update({}, visualisations); }));
        QVERIFY(_document->transforms().isEmpty());

        auto channels = [this]
        {
            const auto documentVisualisations = _document->visualisations();

            QStringList result;
            result.reserve(documentVisualisations.size());
            for(const auto& visualisation : documentVisualisations)
                result.append(_document->parseVisualisation(visualisation).value(u"channel"_s).toString());

            return result;
        };

        QCOMPARE(channels(), (QStringList{u"Colour"_s, u"Text"_s, u"Size"_s}));

        _document->moveVisualisation(0, 2);
        QVERIFY(QTest::qWaitFor([&] { return channels() == QStringList{u"Text"_s, u"Size"_s, u"Colour"_s}; }));
        QVERIFY(waitForCommands());

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(channels(), (QStringList{u"Colour"_s, u"Text"_s, u"Size"_s}));
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
        model.resetRowValue(xRow);
        QVERIFY(!model.rowIsEdited(xRow));
        QVERIFY(!model.property("hasEdits").toBool());
        QCOMPARE(value(xRow, 1).toString(), u"x"_s);

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

    // The tree of attributes the UI's attribute lists show, including an edge's nodes' attributes
    void displayTextForGraphTransform_data()
    {
        QTest::addColumn<QString>("transform");
        QTest::addColumn<QString>("expected");

        QTest::newRow("attributes")
            << u"AverageAttribute using $kind $score"_s
            << u"Average Attribute using kind score"_s;
        QTest::newRow("declared attribute and parameter")
            << u"KNN using $NodeDegree with k = 3"_s
            << u"k-NN using Node Degree with k = 3"_s;
        QTest::newRow("choices")
            << u"KNN using $score with k = 3 RankOrder = Ascending"_s
            << u"k-NN using score with k = 3 Rank Order = Ascending"_s;
        QTest::newRow("choice with a different display value")
            << u"SpanningForest with TraversalOrder = DepthFirst"_s
            << u"Spanning Forest with Traversal Order = Depth First"_s;
        QTest::newRow("float parameter")
            << u"MCLCluster with Granularity = 2.5"_s
            << u"MCL Cluster with Granularity = 2.5"_s;
        QTest::newRow("string parameter")
            << uR"(CombineAttributes using $kind $score with Name = "Kind and Score" AttributeValue = " ")"_s
            << uR"(Combine Attributes using kind score with Name = Kind and Score Attribute Value =  )"_s;
        QTest::newRow("pinned")
            << u"[pinned] RemoveLeaves with Limit = 2"_s
            << u"📌 Remove Leaves with Limit = 2"_s;
        QTest::newRow("pinned with condition")
            << u"[pinned] RemoveNodes where $NodeDegree > 1"_s
            << u"📌 Remove Nodes where Node Degree > 1"_s;
        QTest::newRow("numerical ops")
            << u"RemoveNodes where $score < 1 or $score >= 5"_s
            << u"Remove Nodes where score < 1 or score ≥ 5"_s;
        QTest::newRow("including string ops")
            << uR"(RemoveNodes where $kind includes "a" and $kind excludes "b")"_s
            << u"Remove Nodes where kind Includes a and kind Excludes b"_s;
        QTest::newRow("starting and ending string ops")
            << uR"(RemoveNodes where $kind starts "a" or $kind ends "b")"_s
            << u"Remove Nodes where kind Starts With a or kind Ends With b"_s;
        QTest::newRow("regex string ops")
            << uR"(RemoveNodes where $kind matches "^a" or $kind matchesCaseInsensitive "b$")"_s
            << u"Remove Nodes where kind Matches Regex ^a or kind Matches Case Insensitive Regex b$"_s;
        QTest::newRow("unknown transform and parameter")
            << u"Unknown using $score with Some = 1"_s
            << u"Unknown using score with Some = 1"_s;
        QTest::newRow("malformed")
            << u"RemoveNodes where"_s
            << QString();
    }

    // As shown when choosing a template, or a plugin's default transforms
    void displayTextForGraphTransform()
    {
        QFETCH(const QString, transform);
        QFETCH(const QString, expected);

        QVERIFY(open());
        QCOMPARE(_document->displayTextForGraphTransform(transform), expected);
    }

    void availableAttributesTree()
    {
        QVERIFY(open());

        const std::unique_ptr<AvailableAttributesModel> model(_document->availableAttributesModel(
            static_cast<int>(ElementType::Edge), static_cast<int>(ValueType::All)));
        QVERIFY(model != nullptr);
        QCOMPARE(model->columnCount({}), 1);
        QCOMPARE(model->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), u"Attribute"_s);

        auto roleFor = [&model](const QByteArray& name) { return model->roleNames().key(name); };

        const auto source = model->find(u"source.score"_s);
        QVERIFY(source.isValid());
        QCOMPARE(source.data().toString(), u"score"_s);
        QCOMPARE(model->get(source), u"source.score"_s);
        QCOMPARE(model->parent(source).data().toString(), u"Source Node"_s);
        QCOMPARE(model->data(model->parent(source), roleFor("hasChildren")).toBool(), true);

        // The nodes' attributes are an edge's
        QCOMPARE(model->data(source, roleFor("elementType")).toString(), u"Edge"_s);

        const auto target = model->find(u"target.kind"_s);
        QVERIFY(target.isValid());
        QCOMPARE(model->get(target), u"target.kind"_s);
        QVERIFY(model->find(uR"(target."Not an attribute")"_s) == QModelIndex());

        const auto multiplicity = model->find(u"EdgeMultiplicity"_s);
        QVERIFY(multiplicity.isValid());
        QCOMPARE(model->get(multiplicity), u"EdgeMultiplicity"_s);
        QCOMPARE(model->data(multiplicity, roleFor("elementType")).toString(), u"Edge"_s);
        QCOMPARE(model->data(multiplicity, roleFor("valueType")).toString(), u"Numerical"_s);
        QCOMPARE(model->data(multiplicity, roleFor("userDefined")).toString(), u"Calculated"_s);
        QCOMPARE(model->data(multiplicity, roleFor("editable")).toBool(), false);
        QCOMPARE(model->data(multiplicity, roleFor("hasChildren")).toBool(), false);
        QCOMPARE(model->data(multiplicity, roleFor("hasParameter")).toBool(), false);
        QVERIFY(model->flags(multiplicity).testFlag(Qt::ItemIsEnabled));

        // Node attributes on their own
        const std::unique_ptr<AvailableAttributesModel> nodes(_document->availableAttributesModel(
            static_cast<int>(ElementType::Node), static_cast<int>(ValueType::All)));
        const auto kind = nodes->find(u"kind"_s);
        QVERIFY(kind.isValid());
        QVERIFY(!nodes->parent(kind).isValid());
        QCOMPARE(nodes->data(kind, roleFor("valueType")).toString(), u"Textual"_s);
        QCOMPARE(nodes->data(kind, roleFor("userDefined")).toString(), u"User Defined"_s);
        QCOMPARE(nodes->data(kind, roleFor("editable")).toBool(), true);
        QCOMPARE(nodes->data(kind, roleFor("hasSharedValues")).toBool(), true);
        QCOMPARE(nodes->data(kind, roleFor("searchable")).toBool(), true);
        QCOMPARE(nodes->data(kind, roleFor("elementType")).toString(), u"Node"_s);
    }

    // A matrix, as the dialog for opening one configures the generic plugin
    void matrix()
    {
        // a-b at 0.3 one way and 0.9 the other, b-c at 0.1, and a-c at 0.6
        const QByteArray csv =
            ",a,b,c\n"
            "a,0,0.9,0.6\n"
            "b,0.3,0,0.1\n"
            "c,0,0,0\n";

        QVERIFY(open(write(u"matrix.csv"_s, csv), u"MatrixCSV"_s, u"Generic"_s,
        {
            {u"filterEdges"_s, u"true"_s},
            {u"minimumThreshold"_s, 0.2},
            {u"initialThreshold"_s, 0.5},
            {u"skipDuplicates"_s, u"true"_s},
        }));

        QCOMPARE(_document->transforms(), QStringList{u"RemoveEdges where $AbsoluteEdgeWeight < 0.5"_s});

        // b-c is below the minimum, and the duplicate a-b keeps the larger weight
        QCOMPARE(graphModel()->mutableGraph().numEdges(), 2u);
        QCOMPARE(graph().numEdges(), 2u);

        std::vector<double> weights;
        weights.reserve(graph().edgeIds().size());
        for(auto edgeId : graph().edgeIds())
            weights.push_back(graphModel()->attributeValueByName(u"EdgeWeight"_s).numericValueOf(edgeId));
        std::ranges::sort(weights);
        QCOMPARE(weights.size(), 2u);
        QVERIFY(std::abs(weights.at(0) - 0.6) < 1e-6);
        QVERIFY(std::abs(weights.at(1) - 0.9) < 1e-6);

        // Without the options, every non-zero value is an edge
        QVERIFY(open(write(u"matrix.csv"_s, csv), u"MatrixCSV"_s, u"Generic"_s));
        QVERIFY(_document->transforms().isEmpty());
        QCOMPARE(graph().numEdges(), 4u);
    }

    // The other file types the generic plugin loads, through Document
    void genericFileTypes_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<QString>("type");
        QTest::addColumn<QByteArray>("content");

        QTest::newRow("GraphML") << u"graph.graphml"_s << u"GraphML"_s << GraphML;
        QTest::newRow("DOT") << u"graph.dot"_s << u"DOT"_s << QByteArray("digraph { a -> b; b -> c; }");
        QTest::newRow("JSON Graph") << u"graph.json"_s << u"JSONGraph"_s << JsonGraph;
        QTest::newRow("Pairwise TSV") << u"graph.tsv"_s << u"PairwiseTSV"_s << QByteArray("a\tb\nb\tc\n");
        QTest::newRow("Pairwise SSV") << u"graph.ssv"_s << u"PairwiseSSV"_s << QByteArray("a;b\nb;c\n");
        QTest::newRow("Pairwise TXT") << u"graph.txt"_s << u"PairwiseTXT"_s << QByteArray("a b\nb c\n");
        QTest::newRow("Pairwise XLSX") << u"graph.xlsx"_s << u"PairwiseXLSX"_s <<
            XlsxWriter::xlsx({{u"a"_s, u"b"_s}, {u"b"_s, u"c"_s}});
        QTest::newRow("Matrix CSV") << u"matrix.csv"_s << u"MatrixCSV"_s <<
            QByteArray(",a,b,c\na,0,1,0\nb,0,0,1\nc,0,0,0\n");
    }

    void genericFileTypes()
    {
        QFETCH(const QString, fileName);
        QFETCH(const QString, type);
        QFETCH(const QByteArray, content);

        const auto url = write(fileName, content);
        QVERIFY(_application->urlTypesOf(url).contains(type));
        QVERIFY(open(url, type, u"Generic"_s));

        QCOMPARE(graph().numNodes(), 3u);
        QCOMPARE(graph().numEdges(), 2u);
    }

    // An edge's source and target nodes' attributes, as transforms refer to them
    void edgeNodeAttributes()
    {
        QVERIFY(open(write(u"weights.gml"_s, R"(graph
[
    directed 1
    node [ id 0 label "a" weight 0.5 rank 1 kind "x" ]
    node [ id 1 label "b" weight 1.5 rank 2 kind "y" ]
    node [ id 2 label "c" weight 2.5 rank 3 kind "x" ]
    edge [ source 0 target 1 ]
    edge [ source 1 target 2 ]
    edge [ source 2 target 0 ]
])"), u"GML"_s, u"Generic"_s));

        auto edges = [this]
        {
            QStringList names;
            for(auto edgeId : graph().edgeIds())
            {
                const auto& edge = graph().edgeById(edgeId);
                names.append(graphModel()->nodeName(edge.sourceId()) + graphModel()->nodeName(edge.targetId()));
            }

            names.sort();
            return names;
        };

        QCOMPARE(graphModel()->attributeValueByName(u"source.weight"_s).valueType(), ValueType::Float);
        QVERIFY(execute([this] { _document->update({u"RemoveEdges where $source.weight > 1"_s}); }));
        QCOMPARE(edges(), QStringList{u"ab"_s});
        QVERIFY(execute([this] { _document->undo(); }));

        QCOMPARE(graphModel()->attributeValueByName(u"target.rank"_s).valueType(), ValueType::Int);
        QVERIFY(execute([this] { _document->update({u"RemoveEdges where $target.rank == 1"_s}); }));
        QCOMPARE(edges(), (QStringList{u"ab"_s, u"bc"_s}));
        QVERIFY(execute([this] { _document->undo(); }));

        QVERIFY(execute([this] { _document->update({uR"(RemoveEdges where $target.kind == "x")"_s}); }));
        QCOMPARE(edges(), QStringList{u"ab"_s});
        QCOMPARE(_document->transforms().size(), 1);

        // Only nodes have source and target nodes
        QVERIFY(!graphModel()->attributeValueByName(u"source.EdgeMultiplicity"_s).isValid());
    }

    // An edge attribute, whose rows are the edges
    void editEdgeAttributeTable()
    {
        QVERIFY(open(write(u"edges.gml"_s,
            "graph [ directed 1\n"
            "    node [ id 0 label \"a\" ] node [ id 1 label \"b\" ] node [ id 2 label \"c\" ]\n"
            "    edge [ source 0 target 1 kind \"p\" ]\n"
            "    edge [ source 1 target 2 kind \"q\" ]\n"
            "    edge [ source 2 target 0 kind \"p\" ]\n"
            "]\n"), u"GML"_s, u"Generic"_s));

        EditAttributeTableModel model;
        model.setProperty("document", QVariant::fromValue(_document.get()));
        model.setProperty("attributeName", u"kind"_s);

        auto value = [&model](int row, int column)
            { return model.data(model.index(row, column), Qt::DisplayRole); };

        QCOMPARE(model.rowCount(), 3);
        QStringList kinds;
        kinds.reserve(model.rowCount());
        for(int row = 0; row < model.rowCount(); row++)
            kinds.append(value(row, 1).toString());
        kinds.sort();
        QCOMPARE(kinds, (QStringList{u"p"_s, u"p"_s, u"q"_s}));

        int row = 0;
        while(value(row, 1).toString() != u"q"_s)
            row++;

        model.editValue(row, u"r"_s);
        QVERIFY(model.rowIsEdited(row));
        QCOMPARE(value(row, 1).toString(), u"r"_s);
        model.resetRowValue(row);
        QVERIFY(!model.property("hasEdits").toBool());

        model.editValue(row, u"r"_s);
        model.resetAllEdits();
        QVERIFY(!model.property("hasEdits").toBool());
        QCOMPARE(value(row, 1).toString(), u"q"_s);

        // Shared values: p is on two edges
        model.setProperty("combineSharedValues", true);
        QCOMPARE(model.rowCount(), 2);
        const int pRow = value(0, 1).toString() == u"p"_s ? 0 : 1;
        QCOMPARE(value(pRow, 0).toInt(), 2);

        model.editValue(pRow, u"s"_s);
        QVERIFY(model.rowIsEdited(pRow));
        model.resetRowValue(pRow);
        QVERIFY(!model.rowIsEdited(pRow));
        QVERIFY(!model.property("hasEdits").toBool());
        QCOMPARE(value(pRow, 1).toString(), u"p"_s);

        model.editValue(pRow, u"s"_s);
        QVERIFY(model.rowIsEdited(pRow));
        QCOMPARE(value(pRow, 1).toString(), u"s"_s);

        const auto edits = model.property("edits").value<AttributeEdits>();
        QCOMPARE(edits.edgeValues().size(), 2u);
        QVERIFY(execute([&] { _document->editAttribute(u"kind"_s, edits, ValueType::Unknown, {}); }));

        const auto values = _document->allAttributeValues(u"kind"_s);
        QCOMPARE(values.count(u"s"_s), 2);
        QCOMPARE(values.count(u"q"_s), 1);
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
            "\"say \"\"hi\"\"\",\r\n"
            "\"two\nlines\",x\r\n"_s;

        QTest::newRow("TSV") << u"tsv"_s <<
            u"Name\tValue, in units\r\n"
            "plain\t1.5\r\n"
            "a,b\t42\r\n"
            "say \"hi\"\t\r\n"
            "two lines\tx\r\n"_s;
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

        // Line breaks would split the row
        appendRow(u"two\nlines"_s, u"x"_s);

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

    std::vector<QVector3D> positions()
    {
        std::vector<QVector3D> v;
        v.reserve(graph().nodeIds().size());
        for(auto nodeId : graph().nodeIds())
            v.push_back(graphModel()->nodePositions().get(nodeId));

        return v;
    }

    // Node positions saved, then loaded again
    void nodePositions()
    {
        QVERIFY(open());

        // Let the layout put the nodes in different places
        QVERIFY(QTest::qWaitFor([this]
        {
            const auto& positions = graphModel()->nodePositions();
            return positions.get(nodeId(u"a"_s)) != positions.get(nodeId(u"c"_s));
        }, Timeout));

        _document->setUserLayoutPaused(true);
        QVERIFY(waitForCommands());

        const auto saved = QUrl::fromLocalFile(_dir.filePath(u"positions.json"_s));
        QVERIFY(execute([&] { _document->saveNodePositionsToFile(saved); }));

        QFile file(saved.toLocalFile());
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto content = parseJsonFrom(file.readAll());
        QVERIFY(content.is_array());
        QCOMPARE(content.size(), 5u);

        // Each node's position as the layout has it, before it's scaled for display
        std::map<QString, QVector3D> savedPositions;
        for(const auto& node : content)
        {
            const auto& position = node["position"];
            savedPositions.emplace(QString::fromStdString(node["name"].get<std::string>()),
                QVector3D(position.at(0).get<float>(), position.at(1).get<float>(), position.at(2).get<float>()));
        }

        QVERIFY(savedPositions.at(u"a"_s) != savedPositions.at(u"c"_s));

        auto displayed = [this](const QString& name) { return graphModel()->nodePositions().get(nodeId(name)); };
        const auto scale = graphModel()->nodePositions().scale();

        // Move a, then load the saved positions, which puts it back
        const auto moved = write(u"moved.json"_s,
            QByteArray::fromStdString(json::array({{{"id", static_cast<int>(nodeId(u"a"_s))},
            {"position", {100.0, 200.0, 300.0}}}}).dump()));
        QVERIFY(execute([&] { _document->loadNodePositionsFromFile(moved); }));
        QCOMPARE(displayed(u"a"_s), QVector3D(100.0f, 200.0f, 300.0f) * scale);

        QVERIFY(execute([&] { _document->loadNodePositionsFromFile(saved); }));

        for(const auto& [name, position] : savedPositions)
            QCOMPARE(displayed(name), position * scale);
    }

    void malformedNodePositions_data()
    {
        QTest::addColumn<QByteArray>("content");

        QTest::newRow("Not JSON") << QByteArray("[");
        QTest::newRow("Not an array") << QByteArray(R"({"id": 0})");
        QTest::newRow("Unknown node") << QByteArray(R"([{"id": 1000, "position": [1, 2, 3]}])");
        QTest::newRow("No position") << QByteArray(R"([{"id": 0}])");
        QTest::newRow("Short position") << QByteArray(R"([{"id": 0, "position": [1, 2]}])");
        QTest::newRow("String id") << QByteArray(R"([{"id": "0", "position": [1, 2, 3]}])");
        QTest::newRow("String coordinate") << QByteArray(R"([{"id": 0, "position": [1, "2", 3]}])");
        QTest::newRow("Null coordinate") << QByteArray(R"([{"id": 0, "position": [1, null, 3]}])");
        QTest::newRow("Empty") << QByteArray();
        QTest::newRow("Not objects") << QByteArray(R"([[0, [1, 2, 3]]])");
    }

    // A file that can't be loaded changes nothing
    void malformedNodePositions()
    {
        QFETCH(QByteArray, content);

        QVERIFY(open());

        _document->setUserLayoutPaused(true);
        QVERIFY(waitForCommands());

        const auto before = positions();
        QVERIFY(execute([&] { _document->loadNodePositionsFromFile(write(u"positions.json"_s, content)); }));
        QCOMPARE(positions(), before);

        // ...as does one that doesn't exist
        QVERIFY(execute([&] { _document->loadNodePositionsFromFile(QUrl::fromLocalFile(_dir.filePath(u"missing.json"_s))); }));
        QCOMPARE(positions(), before);
    }

    // Changing only a transform's flags, such as locking it, is undoable, without
    // applying the transforms again
    void transformFlags()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({u"RemoveNodes where $score > 3"_s}); }));
        QCOMPARE(graph().numNodes(), 3u);

        QVERIFY(execute([this]
        {
            _document->apply({u"[locked] RemoveNodes where $score > 3"_s},
                _document->visualisations());
        }));
        QCOMPARE(_document->transforms(), QStringList{u"[locked] RemoveNodes where $score > 3"_s});
        QCOMPARE(graph().numNodes(), 3u);
        QVERIFY(_document->property("nextUndoAction").toString().endsWith(u"Apply Transform Flags"_s));

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(_document->transforms(), QStringList{u"RemoveNodes where $score > 3"_s});
        QCOMPARE(graph().numNodes(), 3u);

        // Applying the same transforms again does nothing
        const auto undoAction = _document->property("nextUndoAction").toString();
        _document->apply(_document->transforms(), _document->visualisations());
        QVERIFY(waitForCommands());
        QCOMPARE(_document->property("nextUndoAction").toString(), undoAction);
    }

    // A transform that refers to an attribute that doesn't exist is shown with an error
    void transformAlert()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({uR"(RemoveNodes where $"Not an attribute" > 3)"_s}); }));
        QCOMPARE(graph().numNodes(), 5u);

        const auto info = _document->transformInfoAtIndex(0);
        QCOMPARE(info.value(u"alertType"_s).toInt(), static_cast<int>(AlertType::Error));
        QVERIFY2(info.value(u"alertText"_s).toString().contains(u"Not an attribute"_s),
            qPrintable(info.value(u"alertText"_s).toString()));
    }

    // The sizes of nodes, edges and text, which are saved with the Document
    void sizes()
    {
        QVERIFY(open());
        QVERIFY(!_document->property("saveRequired").toBool());

        const QSignalSpy nodeSizeChanged(_document.get(), &Document::nodeSizeChanged);
        _document->setProperty("nodeSize", 0.75f);
        _document->setProperty("edgeSize", 0.25f);
        _document->setProperty("textSize", 0.25f);
        QCOMPARE(nodeSizeChanged.count(), 1);
        QCOMPARE(_document->property("nodeSize").toFloat(), 0.75f);
        QCOMPARE(_document->property("edgeSize").toFloat(), 0.25f);
        QCOMPARE(_document->property("textSize").toFloat(), 0.25f);
        QVERIFY(_document->property("saveRequired").toBool());

        // Setting the same size again changes nothing
        _document->setProperty("nodeSize", 0.75f);
        QCOMPARE(nodeSizeChanged.count(), 1);

        auto url = QUrl::fromLocalFile(_dir.filePath(u"sizes.%1"_s.arg(Application::nativeExtension())));
        QVERIFY(execute([&] { _document->saveFile(url, Application::name(), {}, {}); }));
        QVERIFY(open(url, QString::fromLatin1(Application::NativeFileType), {}));

        QCOMPARE(_document->property("nodeSize").toFloat(), 0.75f);
        QCOMPARE(_document->property("edgeSize").toFloat(), 0.25f);
        QCOMPARE(_document->property("textSize").toFloat(), 0.25f);

        _document->resetNodeSize();
        _document->resetEdgeSize();
        _document->resetTextSize();
        QCOMPARE(_document->property("nodeSize").toFloat(), u::pref(u"visuals/defaultNormalNodeSize"_s).toFloat());
        QCOMPARE(_document->property("edgeSize").toFloat(), u::pref(u"visuals/defaultNormalEdgeSize"_s).toFloat());
        QCOMPARE(_document->property("textSize").toFloat(), LimitConstants::defaultTextSize());
    }

    // As the visualisation panel removes them; the removal is only applied when the panel applies it
    void removeVisualisation()
    {
        QVERIFY(open());

        QVERIFY(execute([this]
        {
            _document->update({}, {u"score Size"_s, u"kind Colour"_s});
        }));

        _document->removeVisualisation(0);
        QCOMPARE(_document->property("visualisations").toStringList(), QStringList{u"kind Colour"_s});
        QCOMPARE(_document->visualisations().size(), 2);

        QVERIFY(execute([this]
        {
            _document->apply(_document->transforms(), _document->property("visualisations").toStringList());
        }));
        QCOMPARE(_document->visualisations(), QStringList{u"kind Colour"_s});
    }

    // A command that runs until it's cancelled, as from the cancel button
    void cancelCommand()
    {
        QVERIFY(open());
        QVERIFY(!_document->property("commandIsCancellable").toBool());

        QSignalSpy commandsFinished(_document.get(), &Document::commandsFinished);
        const QSignalSpy cancellableChanged(_document.get(), &Document::commandIsCancellableChanged);
        _document->startTestCommand();

        QVERIFY(QTest::qWaitFor([this] { return _document->property("commandIsCancellable").toBool(); }, Timeout));
        QVERIFY(_document->busy());
        QCOMPARE(_document->property("commandVerb").toString(), u"Test Command"_s);
        QVERIFY(!_document->property("commandIsCancelling").toBool());

        _document->cancelCommand();
        QVERIFY(_document->property("commandIsCancelling").toBool());

        QVERIFY(QTest::qWaitFor([&] { return !commandsFinished.isEmpty(); }, Timeout));
        QVERIFY(waitForCommands());
        QVERIFY(!_document->property("commandIsCancelling").toBool());
        QVERIFY(!_document->canUndo());
    }

    // What the visualisation panel shows of visualisations that can't be applied
    void visualisationAlerts()
    {
        QVERIFY(open());

        QVERIFY(execute([this]
        {
            _document->update({}, {u"kind Size"_s, u"nope Colour"_s, u"score Nope"_s,
                u"NodePageRank Colour"_s, u"score Colour"_s});
        }));

        auto alert = [this](int index)
        {
            const auto info = _document->visualisationInfoAtIndex(index);
            return u"%1: %2"_s.arg(info.value(u"alertType"_s).toInt() == static_cast<int>(AlertType::Error) ?
                u"Error"_s : u"Not an error"_s, info.value(u"alertText"_s).toString());
        };

        QCOMPARE(alert(0), u"Error: Visualisation doesn't support attribute type"_s);
        QCOMPARE(alert(1), u"Error: Attribute 'nope' doesn't exist"_s);
        QCOMPARE(alert(2), u"Error: Visualisation channel 'Nope' doesn't exist"_s);
        QCOMPARE(alert(3), u"Error: Attribute 'NodePageRank' doesn't exist"_s);
        QCOMPARE(_document->visualisationInfoAtIndex(4).value(u"alertType"_s).toInt(), static_cast<int>(AlertType::None));
    }

    // A plugin's default visualisations are given the default parameters of their attributes'
    // types; NodePageRank is a transform's attribute, but there's no such transform, so it's given
    // those of the type its transform says it is, as score, a float, is
    void defaultVisualisationsOfTransformAttributes()
    {
        QVERIFY(open(write(u"graph.gml"_s, Gml), u"GML"_s, u"Generic"_s,
            {{u"additionalVisualisations"_s, QStringList{u"NodePageRank Colour"_s, u"score Colour"_s}}}));

        const auto visualisations = _document->property("visualisations").toStringList();
        QCOMPARE(visualisations.size(), 2);

        const auto parametersOf = [](const QString& visualisation) { return visualisation.mid(visualisation.indexOf(u" with "_s)); };
        QVERIFY(visualisations.at(1).contains(u" with "_s));
        QCOMPARE(parametersOf(visualisations.at(0)), parametersOf(visualisations.at(1)));
    }

    // Transforms that don't give every parameter, as a template saved by an earlier version
    // may not, are given the defaults of those that are missing
    void transformsWithoutParameters()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({u"MCLCluster"_s}); }));
        QVERIFY(graphModel()->attributeExists(u"MCLCluster"_s));

        const auto transforms = _document->property("transforms").toStringList();
        QCOMPARE(transforms.size(), 1);
        QVERIFY(transforms.at(0).contains(u"Granularity"_s));

        QVERIFY(execute([this] { _document->apply({u"PageRank"_s}, {}); }));
        QVERIFY(graphModel()->attributeExists(u"NodePageRank"_s));
        QVERIFY(!graphModel()->attributeExists(u"MCLCluster"_s));
    }

    // The attributes transforms add or change, from a transform on, which those before it can't use
    void attributesAddedByLaterTransforms()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({u"MCLCluster"_s, u"PageRank"_s}); }));

        auto addedFrom = [this](int index)
        {
            auto names = _document->addedOrChangedAttributeNamesAtTransformIndexOrLater(index);
            names.sort();
            return names;
        };

        QCOMPARE(addedFrom(0), (QStringList{u"MCLCluster"_s, u"MCLClusterSize"_s, u"NodePageRank"_s}));
        QCOMPARE(addedFrom(1), QStringList{u"NodePageRank"_s});
        QVERIFY(addedFrom(2).isEmpty());
    }

    // Edge text visualisations warn that edge text is hidden, and stop when it's shown
    void edgeTextPreference()
    {
        u::setPref(u"visuals/showEdgeText"_s, static_cast<int>(TextState::Off));
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({}, {u"EdgeMultiplicity Text"_s}); }));
        QCOMPARE(_document->visualisationInfoAtIndex(0).value(u"alertType"_s).toInt(),
            static_cast<int>(AlertType::Warning));
        QVERIFY(_document->property("hasValidEdgeTextVisualisation").toBool());

        QSignalSpy visualisationsChanged(_document.get(), &Document::visualisationsChanged);
        u::setPref(u"visuals/showEdgeText"_s, static_cast<int>(TextState::All));
        QVERIFY(QTest::qWaitFor([&] { return !visualisationsChanged.isEmpty(); }, Timeout));
        QVERIFY(waitForCommands());

        QCOMPARE(_document->visualisationInfoAtIndex(0).value(u"alertType"_s).toInt(),
            static_cast<int>(AlertType::None));

        visualisationsChanged.clear();
        u::setPref(u"visuals/showEdgeText"_s, static_cast<int>(TextState::Selected));
        QVERIFY(QTest::qWaitFor([&] { return !visualisationsChanged.isEmpty(); }, Timeout));
        QVERIFY(waitForCommands());
    }

    // A column of a table, as copied from its header's menu
    void copyColumnToClipboard()
    {
        QVERIFY(open());

        QStandardItemModel model(3, 2);
        for(int row = 0; row < 3; row++)
        {
            model.setItem(row, 0, new QStandardItem(u"r%1"_s.arg(row)));
            model.setItem(row, 1, new QStandardItem(QString::number(row * 10)));
        }

        _document->copyTableModelColumnToClipboard(&model, 1, {2, 0});
        QCOMPARE(QGuiApplication::clipboard()->text(), u"20\n0\n"_s);

        // A column that doesn't exist copies nothing
        QTest::ignoreMessage(QtDebugMsg, "Document::copyTableModelColumnToClipboard: requested column exceeds column count");
        _document->copyTableModelColumnToClipboard(&model, 2, {0});
        QCOMPARE(QGuiApplication::clipboard()->text(), u"20\n0\n"_s);
    }
};

QTEST_MAIN(TestDocument_)
#include "tst_document.moc"
