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

#include "app/attributes/attributeedits.h"
#include "app/commands/applytransformscommand.h"
#include "app/commands/cloneattributecommand.h"
#include "app/commands/deletenodescommand.h"
#include "app/commands/editattributecommand.h"
#include "app/commands/importattributescommand.h"
#include "app/commands/applyvisualisationscommand.h"
#include "app/commands/removeattributescommand.h"
#include "app/ui/document.h"
#include "app/ui/selectionmanager.h"
#include "app/ui/visualisations/visualisationconfigparser.h"

#include "shared/loading/gmlfileparser.h"
#include "shared/loading/tabulardata.h"

#include <QMetaObject>
#include <QTest>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// a -> b -> c, with a -> c and c -> c, where b has no score
const QByteArray Gml = R"(graph
[
    node [ id 0 label "a" score 0.123456789 count 1 kind "x" ]
    node [ id 1 label "b" count 2 kind "y" ]
    node [ id 2 label "c" score 2.5 count 3 kind "z" ]
    edge [ source 0 target 1 weight 1.5 link "ab" rank 1 ]
    edge [ source 1 target 2 weight 0.333333333 link "bc" rank 2 ]
    edge [ source 0 target 2 weight 3 link "ac" rank 3 ]
    edge [ source 2 target 2 weight 4 link "cc" rank 4 ]
])";

// Whether two lists of visualisations are the same, ignoring formatting
bool visualisationsEquivalent(const QStringList& a, const QStringList& b)
{
    if(a.size() != b.size())
        return false;

    for(int i = 0; i < a.size(); i++)
    {
        VisualisationConfigParser pa;
        VisualisationConfigParser pb;

        if(!pa.parse(a.at(i)) || !pb.parse(b.at(i)) || !pa.result().equals(pb.result()))
            return false;
    }

    return true;
}

std::unique_ptr<TestGraphModel> createModel()
{
    return TestGraphModel::parse(Gml, [](auto* userNodeData, auto* userEdgeData)
        { return std::make_unique<GmlFileParser>(userNodeData, userEdgeData); });
}

// Executes the command, after which checkExecuted is called, then checks that undoing it
// restores everything to how it was, and that redoing it gives the same result as before
template<typename CheckExecutedFn>
void checkExecuteUndoRedo(TestGraphModel& model, ICommand& command, const CheckExecutedFn& checkExecuted)
{
    const auto before = model.state();

    QVERIFY(command.execute());
    checkExecuted();
    if(QTest::currentTestFailed())
        return;

    const auto after = model.state();
    QVERIFY(after != before);

    command.undo();
    QCOMPARE(model.state(), before);

    QVERIFY(command.execute());
    QCOMPARE(model.state(), after);

    command.undo();
    QCOMPARE(model.state(), before);
}

bool exactlyEqual(double actual, double expected)
{
    return std::equal_to<>()(static_cast<float>(actual), static_cast<float>(expected));
}

TabularData tabularData(const QList<QStringList>& rows)
{
    TabularData data;
    data.reserve(static_cast<size_t>(rows.at(0).size()), static_cast<size_t>(rows.size()));

    for(qsizetype row = 0; row < rows.size(); row++)
    {
        for(qsizetype column = 0; column < rows.at(row).size(); column++)
        {
            data.setValueAt(static_cast<size_t>(column), static_cast<size_t>(row),
                rows.at(row).at(column));
        }
    }

    return data;
}

// A Document with nothing loaded, which is all ApplyTransformsCommand needs; the
// commands are executed on the main thread, so what they ask the Document to do
// on the main thread is done immediately
std::unique_ptr<Document> createDocument()
{
    auto document = std::make_unique<Document>();
    auto* documentPtr = document.get();

    QObject::connect(documentPtr, &Document::taskAddedToExecutor, documentPtr, [documentPtr]
    {
        QMetaObject::invokeMethod(documentPtr, "executeDeferred", Qt::DirectConnection);
    }, Qt::DirectConnection);

    return document;
}

// Cancels itself as soon as the transforms report any progress, i.e. part way through applying them
class CancellingApplyTransformsCommand : public ApplyTransformsCommand
{
public:
    using ApplyTransformsCommand::ApplyTransformsCommand;

    void setProgress(int progress) override
    {
        ApplyTransformsCommand::setProgress(progress);

        if(!cancelled())
            cancel();
    }
};
// Cancels itself once a transform reports how far it has got, i.e. while a transform is running
class CancellingRunningTransformCommand : public ApplyTransformsCommand
{
public:
    using ApplyTransformsCommand::ApplyTransformsCommand;

    void setProgress(int progress) override
    {
        ApplyTransformsCommand::setProgress(progress);

        if(progress >= 0 && !cancelled())
            cancel();
    }
};
} // namespace

class TestCommands : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void deleteNodes_data()
    {
        QTest::addColumn<QStringList>("nodes");
        QTest::addColumn<QStringList>("remainingEdges");

        QTest::newRow("One") << QStringList{u"b"_s} << QStringList{u"a-c"_s, u"c-c"_s};
        QTest::newRow("Adjacent") << QStringList{u"a"_s, u"b"_s} << QStringList{u"c-c"_s};
        QTest::newRow("Self loop") << QStringList{u"c"_s} << QStringList{u"a-b"_s};
        QTest::newRow("All") << QStringList{u"a"_s, u"b"_s, u"c"_s} << QStringList{};
    }

    void deleteNodes()
    {
        QFETCH(const QStringList, nodes);
        QFETCH(QStringList, remainingEdges);

        auto model = createModel();
        QVERIFY(model != nullptr);

        SelectionManager selectionManager(model->graphModel());
        const NodeIdSet selection{model->nodeId(u"a"_s), model->nodeId(u"b"_s)};
        selectionManager.selectNodes(selection);

        NodeIdSet nodeIds;
        for(const auto& node : nodes)
            nodeIds.insert(model->nodeId(node));

        DeleteNodesCommand command(&model->graphModel(), &selectionManager, nodeIds);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->edgeNames(), remainingEdges);

            for(auto nodeId : nodeIds)
                QVERIFY(!selectionManager.nodeIsSelected(nodeId));
        });

        QCOMPARE(selectionManager.selectedNodes(), selection);
    }

    void editAttribute()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        const auto* score = model->graphModel().attributeByName(u"score"_s);
        const auto description = score->description();

        AttributeEdits edits;
        edits.setNodeValue(model->nodeId(u"a"_s), u"7"_s);
        edits.setNodeValue(model->nodeId(u"b"_s), u"1.25"_s);
        edits.setNodeValue(model->nodeId(u"c"_s), u""_s);

        EditAttributeCommand command(&model->graphModel(), u"score"_s, edits, ValueType::Unknown, description);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->nodeNumericValue(u"score"_s, u"a"_s), 7.0);
            QCOMPARE(model->nodeNumericValue(u"score"_s, u"b"_s), 1.25);
            QVERIFY(!model->nodeHasValue(u"score"_s, u"c"_s));
        });
    }

    void editEdgeAttribute()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        const auto description = model->graphModel().attributeByName(u"weight"_s)->description();

        AttributeEdits edits;
        edits.setEdgeValue(model->edgeId(u"b"_s, u"c"_s), u"-1"_s);

        EditAttributeCommand command(&model->graphModel(), u"weight"_s, edits, ValueType::Unknown, description);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->edgeNumericValue(u"weight"_s, u"b"_s, u"c"_s), -1.0);
        });
    }

    void editAttributeType()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        const auto description = model->graphModel().attributeByName(u"score"_s)->description();

        AttributeEdits edits;
        edits.setNodeValue(model->nodeId(u"c"_s), u"high"_s);

        EditAttributeCommand command(&model->graphModel(), u"score"_s, edits, ValueType::String, description);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->graphModel().attributeByName(u"score"_s)->valueType(), ValueType::String);
            QCOMPARE(model->nodeValue(u"score"_s, u"c"_s), u"high"_s);
        });
    }

    void editEdgeAttributeType()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        const auto description = model->graphModel().attributeByName(u"weight"_s)->description();

        AttributeEdits edits;
        edits.setEdgeValue(model->edgeId(u"a"_s, u"c"_s), u"heavy"_s);

        EditAttributeCommand command(&model->graphModel(), u"weight"_s, edits, ValueType::String, description);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->graphModel().attributeByName(u"weight"_s)->valueType(), ValueType::String);
            QCOMPARE(model->edgeValue(u"weight"_s, u"a"_s, u"c"_s), u"heavy"_s);
            QCOMPARE(model->edgeValue(u"weight"_s, u"a"_s, u"b"_s), u"1.5"_s);
        });
    }

    void editEdgeAttributeToFloat()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        const auto* rank = model->graphModel().attributeByName(u"rank"_s);
        const auto description = rank->description();

        // The edit attribute dialog offers the types the user data can be converted to
        QCOMPARE(rank->metaData().value(u"userDataType"_s).toInt(), static_cast<int>(ValueType::Int));

        AttributeEdits edits;
        edits.setEdgeValue(model->edgeId(u"b"_s, u"c"_s), u"2.5"_s);
        edits.setEdgeValue(model->edgeId(u"c"_s, u"c"_s), u""_s);

        EditAttributeCommand command(&model->graphModel(), u"rank"_s, edits, ValueType::Float, description);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->graphModel().attributeByName(u"rank"_s)->valueType(), ValueType::Float);
            QCOMPARE(model->graphModel().attributeByName(u"rank"_s)->metaData().value(u"userDataType"_s).toInt(),
                static_cast<int>(ValueType::Float));
            QCOMPARE(model->edgeNumericValue(u"rank"_s, u"a"_s, u"b"_s), 1.0);
            QCOMPARE(model->edgeNumericValue(u"rank"_s, u"b"_s, u"c"_s), 2.5);
            QVERIFY(!model->edgeHasValue(u"rank"_s, u"c"_s, u"c"_s));
        });
    }

    void editEdgeAttributeDescription()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        EditAttributeCommand command(&model->graphModel(), u"link"_s, {}, ValueType::Unknown, u"Links"_s);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->graphModel().attributeByName(u"link"_s)->description(), u"Links"_s);
        });
    }

    void editAttributeDescription()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        EditAttributeCommand command(&model->graphModel(), u"kind"_s, {}, ValueType::Unknown, u"Kinds"_s);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->graphModel().attributeByName(u"kind"_s)->description(), u"Kinds"_s);
        });
    }

    // Edits that can't be made change nothing
    void editAttributeFailures()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        const auto before = model->state();

        // Strings that aren't numbers
        EditAttributeCommand toInt(&model->graphModel(), u"kind"_s, {}, ValueType::Int, {});
        QVERIFY(!toInt.execute());

        // An attribute that isn't the user's, so has no type to change...
        EditAttributeCommand degree(&model->graphModel(), u"NodeDegree"_s, {}, ValueType::String, {});
        QVERIFY(!degree.execute());

        // ...or one of components, which can't be edited
        EditAttributeCommand componentSize(&model->graphModel(), u"ComponentSize"_s, {}, ValueType::Unknown, {});
        QVERIFY(!componentSize.execute());

        QCOMPARE(model->state(), before);
        QCOMPARE(model->graphModel().attributeByName(u"kind"_s)->valueType(), ValueType::String);
    }

    // What an edit shows of itself in the command stack, as is reported when the app
    // crashes, whether it's been done, undone, or its attribute has since been removed
    void editAttributeDebugDescription()
    {
        QByteArray gml = "graph [\n";
        for(int i = 0; i < 12; i++)
            gml += u"node [ id %1 label \"n%1\" count %1 ]\n"_s.arg(i).toUtf8();
        gml += "edge [ source 0 target 1 weight 1 ]\n]";

        auto model = TestGraphModel::parse(gml, [](auto* userNodeData, auto* userEdgeData)
            { return std::make_unique<GmlFileParser>(userNodeData, userEdgeData); });
        QVERIFY(model != nullptr);

        AttributeEdits edits;
        for(int i = 0; i < 12; i++)
            edits.setNodeValue(model->nodeId(u"n%1"_s.arg(i)), QString::number(i + 100));

        // Only the first ten edits are shown
        EditAttributeCommand command(&model->graphModel(), u"count"_s, edits, ValueType::Float, {});
        QVERIFY(command.execute());
        auto lines = command.debugDescription().split(u'\n');
        QCOMPARE(lines.size(), 12);
        QCOMPARE(lines.at(0), u"Edit Attribute count Float"_s);
        QCOMPARE(lines.at(1), u"  0 -> 100"_s);
        QCOMPARE(lines.at(10), u"  9 -> 109"_s);
        QCOMPARE(lines.at(11), u"  ..."_s);

        // Once undone, what the values were before isn't known
        command.undo();
        lines = command.debugDescription().split(u'\n');
        QCOMPARE(lines.size(), 12);
        QCOMPARE(lines.at(1), u"  -> 100"_s);

        AttributeEdits edgeEdits;
        edgeEdits.setEdgeValue(model->edgeId(u"n0"_s, u"n1"_s), u"2"_s);
        EditAttributeCommand edgeCommand(&model->graphModel(), u"weight"_s, edgeEdits, ValueType::Unknown, {});
        QVERIFY(edgeCommand.execute());
        QCOMPARE(edgeCommand.debugDescription(), u"Edit Attribute weight\n  1 -> 2"_s);

        // The attribute no longer exists
        RemoveAttributesCommand removeCommand(&model->graphModel(), {u"weight"_s});
        QVERIFY(removeCommand.execute());
        QCOMPARE(edgeCommand.debugDescription(), u"Edit Attribute weight\n  1 -> 2"_s);
    }

    void cloneAttribute()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        CloneAttributeCommand command(&model->graphModel(), u"score"_s, u"Copy"_s);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QVERIFY(exactlyEqual(model->nodeNumericValue(u"Copy"_s, u"a"_s), 0.123456789));
            QVERIFY(!model->nodeHasValue(u"Copy"_s, u"b"_s));
            QVERIFY(exactlyEqual(model->nodeNumericValue(u"Copy"_s, u"c"_s), 2.5));
        });

        // As is shown in the command stack, done or undone
        QCOMPARE(command.debugDescription(), u"Clone Attribute\n  Copy"_s);
        QVERIFY(command.execute());
        QCOMPARE(command.debugDescription(), u"Clone Attribute\n  Copy"_s);
    }

    void cloneAttributeOfRemovedNodes()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);
        QCOMPARE(model->applyTransforms({uR"(RemoveNodes where $NodeName == "c")"_s}), QStringList{});

        // Attributes from the user's data are cloned for nodes a transform removes, which come back
        // when it's removed, but computed attributes only have values for the nodes that are left
        CloneAttributeCommand userCommand(&model->graphModel(), u"score"_s, u"Copy"_s);
        CloneAttributeCommand computedCommand(&model->graphModel(), u"NodeDegree"_s, u"DegreeCopy"_s);
        QVERIFY(userCommand.execute());
        QVERIFY(computedCommand.execute());

        QVERIFY(exactlyEqual(model->nodeNumericValue(u"Copy"_s, u"c"_s), 2.5));
        QVERIFY(!model->nodeHasValue(u"Copy"_s, u"b"_s));
        QCOMPARE(model->nodeValue(u"DegreeCopy"_s, u"a"_s), u"1"_s);
        QVERIFY(!model->nodeHasValue(u"DegreeCopy"_s, u"c"_s));
    }

    void removeAttributes()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        // Overriding the description, which must also be restored
        EditAttributeCommand editCommand(&model->graphModel(), u"score"_s, {}, ValueType::Unknown, u"Scores"_s);
        QVERIFY(editCommand.execute());

        RemoveAttributesCommand command(&model->graphModel(), {u"score"_s, u"weight"_s});
        checkExecuteUndoRedo(*model, command, [&]
        {
            QVERIFY(!model->graphModel().attributeExists(u"score"_s));
            QVERIFY(!model->graphModel().attributeExists(u"weight"_s));
        });
    }

    void importAttributes()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        auto data = tabularData(
        {
            {u"Name"_s, u"Extra"_s},
            {u"a"_s, u"10"_s},
            {u"c"_s, u"30"_s},
        });

        ImportAttributesCommand command(&model->graphModel(), u"NodeName"_s, &data, 0, {1}, false);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->nodeNumericValue(u"Extra"_s, u"a"_s), 10.0);
            QVERIFY(!model->nodeHasValue(u"Extra"_s, u"b"_s));
            QCOMPARE(model->nodeNumericValue(u"Extra"_s, u"c"_s), 30.0);
        });
    }

    void importAttributesReplacing()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        auto data = tabularData(
        {
            {u"Name"_s, u"kind"_s},
            {u"a"_s, u"p"_s},
            {u"c"_s, u"q"_s},
        });

        ImportAttributesCommand command(&model->graphModel(), u"NodeName"_s, &data, 0, {1}, true);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->nodeValue(u"kind"_s, u"a"_s), u"p"_s);
            QCOMPARE(model->nodeValue(u"kind"_s, u"c"_s), u"q"_s);
        });
    }

    void importEdgeAttributes()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        auto data = tabularData(
        {
            {u"Link"_s, u"Extra"_s, u"Label"_s},
            {u"ab"_s, u"10"_s, u"first"_s},
            {u"cc"_s, u"40"_s, u"loop"_s},
            {u"zz"_s, u"99"_s, u"none"_s},
        });

        ImportAttributesCommand command(&model->graphModel(), u"link"_s, &data, 0, {1, 2}, false);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->graphModel().attributeByName(u"Extra"_s)->elementType(), ElementType::Edge);
            QCOMPARE(model->edgeNumericValue(u"Extra"_s, u"a"_s, u"b"_s), 10.0);
            QCOMPARE(model->edgeNumericValue(u"Extra"_s, u"c"_s, u"c"_s), 40.0);
            QVERIFY(!model->edgeHasValue(u"Extra"_s, u"b"_s, u"c"_s));
            QVERIFY(!model->edgeHasValue(u"Extra"_s, u"a"_s, u"c"_s));
            QCOMPARE(model->edgeValue(u"Label"_s, u"c"_s, u"c"_s), u"loop"_s);
            QVERIFY(!model->edgeHasValue(u"Label"_s, u"a"_s, u"c"_s));
        });
    }

    void importEdgeAttributesReplacing()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);

        auto data = tabularData(
        {
            {u"Link"_s, u"weight"_s, u"link"_s},
            {u"ab"_s, u"2.25"_s, u"AB"_s},
            {u"bc"_s, u"0.125"_s, u"BC"_s},
        });

        ImportAttributesCommand command(&model->graphModel(), u"link"_s, &data, 0, {1, 2}, true);
        checkExecuteUndoRedo(*model, command, [&]
        {
            QCOMPARE(model->edgeNumericValue(u"weight"_s, u"a"_s, u"b"_s), 2.25);
            QCOMPARE(model->edgeNumericValue(u"weight"_s, u"b"_s, u"c"_s), 0.125);
            QVERIFY(!model->edgeHasValue(u"weight"_s, u"a"_s, u"c"_s));
            QCOMPARE(model->edgeValue(u"link"_s, u"a"_s, u"b"_s), u"AB"_s);
            QVERIFY(!model->edgeHasValue(u"link"_s, u"c"_s, u"c"_s));
            QVERIFY(!model->graphModel().attributeExists(u"weight 1"_s));
        });
    }

    void patchedVisualisations_data()
    {
        QTest::addColumn<QStringList>("previous");
        QTest::addColumn<QStringList>("visualisations");
        QTest::addColumn<QStringList>("createdAttributeNames");
        QTest::addColumn<QStringList>("expected");

        const QStringList previous{uR"("score" "Size")"_s};

        QTest::newRow("nothing created") << previous <<
            QStringList{previous.at(0), uR"("Cluster" "Colour")"_s} <<
            QStringList{} <<
            QStringList{previous.at(0), uR"("Cluster" "Colour")"_s};

        QTest::newRow("name not in use") << previous <<
            QStringList{previous.at(0), uR"("Cluster" "Colour")"_s} <<
            QStringList{u"Cluster"_s, u"Cluster Size"_s} <<
            QStringList{previous.at(0), uR"("Cluster" "Colour")"_s};

        QTest::newRow("name in use") << previous <<
            QStringList{previous.at(0), uR"("Cluster" "Colour")"_s, uR"("Cluster Size" "Size")"_s} <<
            QStringList{u"Cluster(1)"_s, u"Cluster Size(2)"_s} <<
            QStringList{previous.at(0), u"\"Cluster(1)\" \"Colour\""_s, u"\"Cluster Size(2)\" \"Size\""_s};

        QTest::newRow("previous visualisations unchanged") <<
            QStringList{uR"("Cluster" "Size")"_s} <<
            QStringList{uR"("Cluster" "Size")"_s, uR"("Cluster" "Colour")"_s} <<
            QStringList{u"Cluster(1)"_s} <<
            QStringList{uR"("Cluster" "Size")"_s, u"\"Cluster(1)\" \"Colour\""_s};

        QTest::newRow("regular expression characters in name") << QStringList{} <<
            QStringList{u"\"Score (raw)\" \"Colour\""_s, uR"("A+B" "Size")"_s} <<
            QStringList{u"Score (raw)(1)"_s, u"A+B(1)"_s} <<
            QStringList{u"\"Score (raw)(1)\" \"Colour\""_s, u"\"A+B(1)\" \"Size\""_s};

        QTest::newRow("flags and parameters kept") << QStringList{} <<
            QStringList{uR"([component] "Cluster" "Colour" with gradient = "x")"_s} <<
            QStringList{u"Cluster(1)"_s} <<
            QStringList{u"[component] \"Cluster(1)\" \"Colour\" with gradient = \"x\""_s};
    }

    void patchedVisualisations()
    {
        QFETCH(const QStringList, previous);
        QFETCH(const QStringList, visualisations);
        QFETCH(QStringList, createdAttributeNames);
        QFETCH(const QStringList, expected);

        const std::vector<QString> created(createdAttributeNames.begin(), createdAttributeNames.end());
        auto patched = ApplyVisualisationsCommand::patchedVisualisations(previous, visualisations, created);
        QVERIFY2(visualisationsEquivalent(patched, expected), qPrintable(patched.join(u"\n"_s)));
    }

    // The names a transform gives the attributes it creates, when they're already in use
    void patchedVisualisationsForTransform()
    {
        auto model = createModel();
        QCOMPARE(model->applyTransforms({u"LouvainCluster"_s, u"LouvainCluster"_s}), QStringList{});

        auto created = model->graphModel().addedOrChangedAttributeNamesAtTransformIndex(1);
        std::ranges::sort(created);
        QCOMPARE(created, (std::vector<QString>{u"LouvainCluster(1)"_s, u"LouvainClusterSize(1)"_s}));

        auto patched = ApplyVisualisationsCommand::patchedVisualisations({},
            {u"LouvainCluster Colour"_s, u"LouvainClusterSize Size"_s}, created);
        QVERIFY2(visualisationsEquivalent(patched,
            {u"\"LouvainCluster(1)\" Colour"_s, u"\"LouvainClusterSize(1)\" Size"_s}),
            qPrintable(patched.join(u"\n"_s)));
    }

    void applyTransforms()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);
        auto document = createDocument();

        const QStringList transforms
        {
            uR"(RemoveEdges where $weight > 2)"_s,
            u"PageRank"_s
        };

        const auto edges = model->edgeNames();

        ApplyTransformsCommand command(&model->graphModel(), document.get(), {}, transforms);
        QVERIFY(command.execute());
        QCOMPARE(model->edgeNames(), (QStringList{u"a-b"_s, u"b-c"_s}));
        QVERIFY(model->graphModel().attributeExists(u"NodePageRank"_s));
        QCOMPARE(document->transforms(), transforms);
        QCOMPARE(document->graphTransformConfigurationsFromUI(), transforms);

        command.undo();
        QCOMPARE(model->edgeNames(), edges);
        QVERIFY(!model->graphModel().attributeExists(u"NodePageRank"_s));
        QCOMPARE(document->transforms(), QStringList{});

        QVERIFY(command.execute());
        QCOMPARE(model->edgeNames(), (QStringList{u"a-b"_s, u"b-c"_s}));
        QCOMPARE(document->transforms(), transforms);

        // Removing one of the transforms
        ApplyTransformsCommand removeCommand(&model->graphModel(), document.get(),
            transforms, {transforms.at(1)});
        QVERIFY(removeCommand.execute());
        QCOMPARE(model->edgeNames(), edges);
        QVERIFY(model->graphModel().attributeExists(u"NodePageRank"_s));

        removeCommand.undo();
        QCOMPARE(model->edgeNames(), (QStringList{u"a-b"_s, u"b-c"_s}));
        QCOMPARE(document->transforms(), transforms);
    }

    // As the Document does after a command changes an attribute that a transform refers to
    void applyTransformsDeferred()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);
        auto document = createDocument();

        const QStringList transforms{uR"(RemoveEdges where $weight > 2)"_s};
        ApplyTransformsCommand command(&model->graphModel(), document.get(), {}, transforms);
        QVERIFY(command.execute());

        // There's nothing to do
        QVERIFY(!model->graphModel().transformRebuildRequired());
        ApplyTransformsCommand nothingToDo(&model->graphModel(), document.get());
        QVERIFY(nothingToDo.execute());
        QCOMPARE(document->transforms(), transforms);

        AttributeEdits edits;
        edits.setEdgeValue(model->edgeId(u"a"_s, u"b"_s), u"5"_s);
        const auto description = model->graphModel().attributeByName(u"weight"_s)->description();
        EditAttributeCommand editCommand(&model->graphModel(), u"weight"_s, edits, ValueType::Unknown, description);
        QVERIFY(editCommand.execute());
        QVERIFY(model->graphModel().transformRebuildRequired());

        // The edge is only removed once the transforms are applied again
        QCOMPARE(model->edgeNames(), (QStringList{u"a-b"_s, u"b-c"_s}));

        ApplyTransformsCommand rebuild(&model->graphModel(), document.get());
        QVERIFY(rebuild.execute());
        QVERIFY(!model->graphModel().transformRebuildRequired());
        QCOMPARE(model->edgeNames(), QStringList{u"b-c"_s});
        QCOMPARE(document->transforms(), transforms);
    }

    void cancelApplyTransforms()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);
        auto document = createDocument();

        const auto edges = model->edgeNames();
        const auto state = model->state();

        const QStringList transforms{uR"(RemoveEdges where $weight > 2)"_s};
        CancellingApplyTransformsCommand command(&model->graphModel(), document.get(), {}, transforms);
        command.execute();
        QVERIFY(command.cancelled());

        QCOMPARE(model->edgeNames(), edges);
        QCOMPARE(model->state(), state);
        QCOMPARE(document->transforms(), QStringList{});

        // Changing the graph applies the transforms again, which must still be those from before
        model->graphModel().mutableGraph().performTransaction([&](IMutableGraph& graph)
        {
            graph.removeEdge(model->edgeId(u"c"_s, u"c"_s));
        });

        QCOMPARE(model->edgeNames(), (QStringList{u"a-b"_s, u"a-c"_s, u"b-c"_s}));
    }

    // The command is cancelled just after the transforms have been applied
    // MCL checks whether it's cancelled during each of its iterations, and stops
    void cancelMcl()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);
        auto document = createDocument();

        const auto state = model->state();
        const QStringList transforms{u"MCLCluster with Granularity = 2.2"_s};

        CancellingRunningTransformCommand command(&model->graphModel(), document.get(), {}, transforms);
        command.execute();
        QVERIFY(command.cancelled());

        QVERIFY(!model->graphModel().attributeExists(u"MCLCluster"_s));
        QCOMPARE(model->state(), state);
        QCOMPARE(document->transforms(), QStringList{});

        // It completes when not cancelled
        ApplyTransformsCommand applyCommand(&model->graphModel(), document.get(), {}, transforms);
        QVERIFY(applyCommand.execute());
        QVERIFY(model->graphModel().attributeExists(u"MCLCluster"_s));
        QVERIFY(model->nodeHasValue(u"MCLCluster"_s, u"a"_s));
    }

    void cancelApplyTransformsAfterBuild()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);
        auto document = createDocument();

        const auto edges = model->edgeNames();

        const QStringList transforms{uR"(RemoveEdges where $weight > 2)"_s};
        ApplyTransformsCommand command(&model->graphModel(), document.get(), {}, transforms);

        QObject::connect(&model->graphModel().graph(), &Graph::graphChanged, document.get(),
            [&command] { command.cancel(); }, Qt::SingleShotConnection);

        command.execute();
        QVERIFY(command.cancelled());

        QCOMPARE(model->edgeNames(), edges);
        QCOMPARE(document->transforms(), QStringList{});
    }

    // The command is cancelled after the transforms have been applied, but before it completes
    void cancelApplyTransformsLate()
    {
        auto model = createModel();
        QVERIFY(model != nullptr);
        auto document = createDocument();

        const auto edges = model->edgeNames();

        const QStringList transforms{uR"(RemoveEdges where $weight > 2)"_s};
        ApplyTransformsCommand command(&model->graphModel(), document.get(), {}, transforms);

        auto connection = QObject::connect(document.get(), &Document::taskAddedToExecutor,
            document.get(), [&command] { command.cancel(); }, Qt::DirectConnection);

        command.execute();
        QObject::disconnect(connection);
        QVERIFY(command.cancelled());

        // Either way, the graph and the Document must agree on which transforms are applied
        const bool applied = model->edgeNames() != edges;
        QCOMPARE(document->transforms(), applied ? transforms : QStringList{});

        // Later changes to the graph still apply the transforms
        model->graphModel().mutableGraph().performTransaction([&](IMutableGraph& graph)
        {
            graph.removeEdge(model->edgeId(u"a"_s, u"b"_s));
        });

        QCOMPARE(model->edgeNames(), applied ? QStringList{u"b-c"_s} :
            (QStringList{u"a-c"_s, u"b-c"_s, u"c-c"_s}));
    }
};

QTEST_MAIN(TestCommands)
#include "tst_commands.moc"
