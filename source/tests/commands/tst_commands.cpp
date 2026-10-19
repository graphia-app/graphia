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
#include "app/commands/cloneattributecommand.h"
#include "app/commands/deletenodescommand.h"
#include "app/commands/editattributecommand.h"
#include "app/commands/importattributescommand.h"
#include "app/commands/removeattributescommand.h"
#include "app/ui/selectionmanager.h"

#include "shared/loading/gmlfileparser.h"
#include "shared/loading/tabulardata.h"

#include <QTest>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <functional>
#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
// a -> b -> c, with a -> c and c -> c, where b has no score
const QByteArray Gml = R"(graph
[
    node [ id 0 label "a" score 0.123456789 count 1 kind "x" ]
    node [ id 1 label "b" count 2 kind "y" ]
    node [ id 2 label "c" score 2.5 count 3 kind "z" ]
    edge [ source 0 target 1 weight 1.5 ]
    edge [ source 1 target 2 weight 0.333333333 ]
    edge [ source 0 target 2 weight 3 ]
    edge [ source 2 target 2 weight 4 ]
])";

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
};

QTEST_MAIN(TestCommands)
#include "tst_commands.moc"
