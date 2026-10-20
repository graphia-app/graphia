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
#include "tests/common/testdocument.h"

#include "app/attributes/attributeedits.h"
#include "app/commands/cloneattributecommand.h"
#include "app/commands/editattributecommand.h"
#include "app/commands/removeattributescommand.h"

#include "shared/loading/gmlfileparser.h"
#include "shared/plugins/nodeattributetablemodel.h"

#include <QSignalSpy>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <algorithm>
#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// a -> b -> c, with a -> c, where b has no score
const QByteArray Gml = R"(graph
[
    node [ id 0 label "a" score 0.5 count 1 kind "x" ]
    node [ id 1 label "b" count 2 kind "y" ]
    node [ id 2 label "c" score 2.5 count 3 kind "z" ]
    edge [ source 0 target 1 ]
    edge [ source 1 target 2 ]
    edge [ source 0 target 2 ]
])";
} // namespace

class TestNodeAttributeTableModel : public QObject
{
    Q_OBJECT

    std::unique_ptr<TestGraphModel> _model;
    std::unique_ptr<TestDocument> _document;
    std::unique_ptr<NodeAttributeTableModel> _tableModel;

    // As the generic plugin does, before the file is loaded and after
    void create()
    {
        _model = TestGraphModel::parse(Gml, [](auto* userNodeData, auto* userEdgeData)
            { return std::make_unique<GmlFileParser>(userNodeData, userEdgeData); });
        QVERIFY(_model != nullptr);

        _document = std::make_unique<TestDocument>(*_model);
        _tableModel = std::make_unique<NodeAttributeTableModel>();
        _tableModel->initialise(_document.get(), &_model->graphModel().userNodeData());

        // The table is filled when the transformed graph changes, as it does once loaded
        _model->applyTransforms({uR"(RemoveNodes where $NodeName == "a")"_s});
        _model->applyTransforms({});
        QVERIFY(QTest::qWaitFor([this] { return _tableModel->columnCount() > 0; }));
    }

    int columnFor(const QString& name) const
    {
        return static_cast<int>(_tableModel->property("columnNames").toStringList().indexOf(name));
    }

    int rowFor(const QString& nodeName) const
    {
        return static_cast<int>(_model->graphModel().userNodeData().indexFor(_model->nodeId(nodeName)));
    }

    QVariant value(const QString& nodeName, const QString& column) const
    {
        return _tableModel->data(_tableModel->index(rowFor(nodeName), columnFor(column)));
    }

    QVariant selected(const QString& nodeName) const
    {
        return _tableModel->data(_tableModel->index(rowFor(nodeName), 0), NodeAttributeTableModel::NodeSelectedRole);
    }

    std::vector<int> rows(const QStringList& nodeNames) const
    {
        std::vector<int> result;
        result.reserve(static_cast<size_t>(nodeNames.size()));
        for(const auto& nodeName : nodeNames)
            result.push_back(rowFor(nodeName));

        std::ranges::sort(result);
        return result;
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void cleanup()
    {
        _tableModel.reset();
        _document.reset();
        _model.reset();
    }

    void columnsAndValues()
    {
        create();
        if(QTest::currentTestFailed())
            return;

        // The user's attributes in the order they were loaded, then the calculated ones
        const QStringList expectedColumnNames
        {
            u"NodeName"_s, u"score"_s, u"count"_s, u"kind"_s,
            u"NodeComponentIdentifier"_s, u"NodeDegree"_s, u"NodeInDegree"_s,
            u"NodeMultiplicity"_s, u"NodeOutDegree"_s
        };

        QCOMPARE(_tableModel->property("columnNames").toStringList(), expectedColumnNames);
        QCOMPARE(_tableModel->rowCount(), 3);
        QCOMPARE(_tableModel->columnCount(), expectedColumnNames.size());
        QCOMPARE(_tableModel->columnNameFor(1), u"score"_s);
        QVERIFY(_tableModel->roleNames().values().contains("nodeSelected"));

        QCOMPARE(value(u"a"_s, u"NodeName"_s).toString(), u"a"_s);
        QCOMPARE(value(u"a"_s, u"score"_s).toDouble(), 0.5);
        QVERIFY(!value(u"b"_s, u"score"_s).isValid());
        QCOMPARE(value(u"c"_s, u"kind"_s).toString(), u"z"_s);
        QCOMPARE(value(u"b"_s, u"count"_s).toInt(), 2);
        QCOMPARE(value(u"a"_s, u"NodeDegree"_s).toInt(), 2);
        QCOMPARE(value(u"c"_s, u"NodeInDegree"_s).toInt(), 2);

        // Out of range
        QVERIFY(!_tableModel->data(_tableModel->index(3, 0)).isValid());
        QVERIFY(!_tableModel->data(_tableModel->index(0, 99)).isValid());
    }

    void columnTypes()
    {
        create();
        if(QTest::currentTestFailed())
            return;

        QVERIFY(_tableModel->columnIsNumerical(u"score"_s));
        QVERIFY(_tableModel->columnIsNumerical(u"count"_s));
        QVERIFY(!_tableModel->columnIsNumerical(u"kind"_s));
        QVERIFY(!_tableModel->columnIsNumerical(u"Not an attribute"_s));

        QVERIFY(_tableModel->columnIsFloatingPoint(u"score"_s));
        QVERIFY(!_tableModel->columnIsFloatingPoint(u"count"_s));
        QVERIFY(!_tableModel->columnIsFloatingPoint(u"Not an attribute"_s));

        QVERIFY(_tableModel->columnIsCalculated(u"NodeDegree"_s));
        QVERIFY(!_tableModel->columnIsCalculated(u"score"_s));
        QVERIFY(!_tableModel->columnIsHiddenByDefault(u"score"_s));

        QVERIFY(!_tableModel->descriptionFor(u"NodeDegree"_s).isEmpty());
        QCOMPARE(_tableModel->descriptionFor(u""_s), QString());
        QCOMPARE(_tableModel->descriptionFor(u"Not an attribute"_s), QString());
    }

    // Rows remain for nodes that aren't in the transformed graph, but they're empty
    void removedNodes()
    {
        create();
        if(QTest::currentTestFailed())
            return;

        QSignalSpy dataChanged(_tableModel.get(), &NodeAttributeTableModel::dataChanged);
        _model->applyTransforms({uR"(RemoveNodes where $NodeName == "b")"_s});
        QVERIFY(QTest::qWaitFor([&] { return !dataChanged.isEmpty(); }));

        QCOMPARE(_tableModel->rowCount(), 3);
        QVERIFY(!value(u"b"_s, u"NodeName"_s).isValid());
        QVERIFY(!value(u"b"_s, u"count"_s).isValid());
        QCOMPARE(value(u"a"_s, u"NodeDegree"_s).toInt(), 1);
        QCOMPARE(value(u"c"_s, u"NodeDegree"_s).toInt(), 1);

        _model->applyTransforms({});
        QVERIFY(QTest::qWaitFor([this] { return value(u"b"_s, u"count"_s).isValid(); }));
        QCOMPARE(value(u"a"_s, u"NodeDegree"_s).toInt(), 2);
    }

    void selection()
    {
        create();
        if(QTest::currentTestFailed())
            return;

        QSignalSpy selectionChanged(_tableModel.get(), &NodeAttributeTableModel::selectionChanged);

        _document->selectionManager()->selectNodes(NodeIdSet{_model->nodeId(u"a"_s), _model->nodeId(u"c"_s)});
        _tableModel->onSelectionChanged();
        QVERIFY(QTest::qWaitFor([&] { return selectionChanged.count() == 1; }));

        QCOMPARE(selected(u"a"_s), QVariant(true));
        QCOMPARE(selected(u"b"_s), QVariant(false));
        QCOMPARE(selected(u"c"_s), QVariant(true));

        // Selecting the same nodes again is still passed on
        _tableModel->onSelectionChanged();
        QVERIFY(QTest::qWaitFor([&] { return selectionChanged.count() == 2; }));

        // Selection is kept when the table is updated for other reasons
        _model->applyTransforms({uR"(RemoveNodes where $NodeName == "b")"_s});
        QTest::qWait(0);
        QCOMPARE(selected(u"a"_s), QVariant(true));
        QCOMPARE(selected(u"b"_s), QVariant());

        const auto count = selectionChanged.count();
        _document->selectionManager()->clearNodeSelection();
        _tableModel->onSelectionChanged();
        QVERIFY(QTest::qWaitFor([&] { return selectionChanged.count() > count; }));
        QCOMPARE(selected(u"a"_s), QVariant(false));
    }

    void relatedRows()
    {
        create();
        if(QTest::currentTestFailed())
            return;

        auto sorted = [](std::vector<int> v) { std::ranges::sort(v); return v; };

        // Each includes the rows it's given
        QCOMPARE(sorted(_tableModel->sourcesOf(rows({u"b"_s}))), rows({u"a"_s, u"b"_s}));
        QCOMPARE(sorted(_tableModel->sourcesOf(rows({u"a"_s}))), rows({u"a"_s}));
        QCOMPARE(sorted(_tableModel->targetsOf(rows({u"a"_s}))), rows({u"a"_s, u"b"_s, u"c"_s}));
        QCOMPARE(sorted(_tableModel->targetsOf(rows({u"c"_s}))), rows({u"c"_s}));
        QCOMPARE(sorted(_tableModel->neighboursOf(rows({u"b"_s}))), rows({u"a"_s, u"b"_s, u"c"_s}));
        QCOMPARE(sorted(_tableModel->neighboursOf(rows({}))), rows({}));

        _tableModel->cropTo(rows({u"a"_s, u"c"_s}));
        QCOMPARE(_document->selectionManager()->selectedNodes(),
            (NodeIdSet{_model->nodeId(u"a"_s), _model->nodeId(u"c"_s)}));

        _tableModel->moveFocusToNodeForRowIndex(static_cast<size_t>(rowFor(u"b"_s)));
        QCOMPARE(_document->_focusedNodeIds, std::vector<NodeId>{_model->nodeId(u"b"_s)});

        _tableModel->moveFocusToNodesForRowIndices(rows({u"a"_s, u"b"_s}));
        QCOMPARE(_document->_focusedNodeIds.size(), 2u);
    }

    void attributeAddedAndRemoved()
    {
        create();
        if(QTest::currentTestFailed())
            return;

        QSignalSpy columnNamesChanged(_tableModel.get(), &NodeAttributeTableModel::columnNamesChanged);

        CloneAttributeCommand command(&_model->graphModel(), u"score"_s, u"Copy"_s);
        QVERIFY(command.execute());
        QVERIFY(QTest::qWaitFor([&] { return columnNamesChanged.count() == 1; }));

        QVERIFY(columnFor(u"Copy"_s) >= 0);
        QCOMPARE(_tableModel->columnCount(), 10);
        QCOMPARE(value(u"c"_s, u"Copy"_s).toDouble(), 2.5);
        QVERIFY(!value(u"b"_s, u"Copy"_s).isValid());

        // The other columns are unaffected
        QCOMPARE(value(u"c"_s, u"kind"_s).toString(), u"z"_s);
        QCOMPARE(value(u"a"_s, u"NodeDegree"_s).toInt(), 2);

        command.undo();
        QVERIFY(QTest::qWaitFor([&] { return columnNamesChanged.count() == 2; }));
        QCOMPARE(columnFor(u"Copy"_s), -1);
        QCOMPARE(_tableModel->columnCount(), 9);
        QCOMPARE(value(u"c"_s, u"kind"_s).toString(), u"z"_s);
        QCOMPARE(value(u"a"_s, u"NodeDegree"_s).toInt(), 2);

        RemoveAttributesCommand removeCommand(&_model->graphModel(), {u"count"_s});
        QVERIFY(removeCommand.execute());
        QVERIFY(QTest::qWaitFor([&] { return columnNamesChanged.count() == 3; }));
        QCOMPARE(columnFor(u"count"_s), -1);
        QCOMPARE(value(u"c"_s, u"kind"_s).toString(), u"z"_s);
        QCOMPARE(value(u"a"_s, u"score"_s).toDouble(), 0.5);

        removeCommand.undo();
        QVERIFY(QTest::qWaitFor([&] { return columnNamesChanged.count() == 4; }));
        QCOMPARE(value(u"b"_s, u"count"_s).toInt(), 2);
        QCOMPARE(value(u"c"_s, u"kind"_s).toString(), u"z"_s);
    }

    // A transform's attribute is added as a column
    void transformAttribute()
    {
        create();
        if(QTest::currentTestFailed())
            return;

        QSignalSpy columnNamesChanged(_tableModel.get(), &NodeAttributeTableModel::columnNamesChanged);

        _model->applyTransforms({u"PageRank"_s});
        QVERIFY(QTest::qWaitFor([&] { return columnNamesChanged.count() == 1; }));
        QVERIFY(columnFor(u"NodePageRank"_s) >= 0);
        for(const auto& nodeName : {u"a"_s, u"b"_s, u"c"_s})
            QCOMPARE(value(nodeName, u"NodePageRank"_s).toDouble(), _model->nodeNumericValue(u"NodePageRank"_s, nodeName));

        _model->applyTransforms({});
        QVERIFY(QTest::qWaitFor([&] { return columnNamesChanged.count() == 2; }));
        QCOMPARE(columnFor(u"NodePageRank"_s), -1);
        QCOMPARE(value(u"c"_s, u"kind"_s).toString(), u"z"_s);
    }

    // Only the column that changed is reported as changed
    void attributeEdited()
    {
        create();
        if(QTest::currentTestFailed())
            return;

        QSignalSpy dataChanged(_tableModel.get(), &NodeAttributeTableModel::dataChanged);
        const QSignalSpy modelReset(_tableModel.get(), &NodeAttributeTableModel::modelReset);

        AttributeEdits edits;
        edits.setNodeValue(_model->nodeId(u"b"_s), u"7"_s);

        const auto description = _model->graphModel().attributeByName(u"score"_s)->description();
        EditAttributeCommand command(&_model->graphModel(), u"score"_s, edits, ValueType::Unknown, description);
        QVERIFY(command.execute());
        QVERIFY(QTest::qWaitFor([&] { return !dataChanged.isEmpty(); }));
        QTest::qWait(0);

        QCOMPARE(modelReset.count(), 0);
        QCOMPARE(dataChanged.count(), 1);

        const auto topLeft = dataChanged.at(0).at(0).toModelIndex();
        const auto bottomRight = dataChanged.at(0).at(1).toModelIndex();
        QCOMPARE(topLeft.column(), columnFor(u"score"_s));
        QCOMPARE(bottomRight.column(), columnFor(u"score"_s));
        QCOMPARE(topLeft.row(), 0);
        QCOMPARE(bottomRight.row(), 2);

        QCOMPARE(value(u"b"_s, u"score"_s).toDouble(), 7.0);
        QCOMPARE(value(u"a"_s, u"score"_s).toDouble(), 0.5);
    }
};

QTEST_MAIN(TestNodeAttributeTableModel)
#include "tst_nodeattributetablemodel.moc"
