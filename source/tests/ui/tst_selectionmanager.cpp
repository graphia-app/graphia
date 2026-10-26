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

#include "app/ui/selectionmanager.h"

#include <QTest>
#include <QSignalSpy>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

class TestSelectionManager : public QObject
{
    Q_OBJECT

    std::unique_ptr<TestGraphModel> _model;
    std::unique_ptr<SelectionManager> _selectionManager;

    NodeIdSet nodeIds(const QStringList& names) const
    {
        NodeIdSet ids;
        for(const auto& name : names)
            ids.insert(_model->nodeId(name));

        return ids;
    }

    QStringList names(const NodeIdSet& ids) const
    {
        QStringList names;
        names.reserve(static_cast<qsizetype>(ids.size()));
        for(auto nodeId : ids)
            names.append(_model->graphModel().nodeName(nodeId));

        names.sort();
        return names;
    }

    QStringList selected() const { return names(_selectionManager->selectedNodes()); }

    NodeId id(const QString& name) const { return _model->nodeId(name); }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void init()
    {
        _model = std::make_unique<TestGraphModel>(QStringList{u"a-b"_s, u"b-c"_s, u"c-d"_s, u"e"_s});
        _model->addEdgeAttribute<double>(u"Weight"_s,
            {{{u"a"_s, u"b"_s}, 1.0}, {{u"b"_s, u"c"_s}, 2.0}, {{u"c"_s, u"d"_s}, 2.0}});
        _model->applyTransforms({});

        _selectionManager = std::make_unique<SelectionManager>(_model->graphModel());
    }

    void cleanup()
    {
        _selectionManager.reset();
        _model.reset();
    }

    void selectAndDeselect()
    {
        const QSignalSpy spy(_selectionManager.get(), &SelectionManager::selectionChanged);

        QVERIFY(!_selectionManager->nodesAreSelected());
        QVERIFY(_selectionManager->selectNode(id(u"a"_s)));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<const SelectionManager*>(), _selectionManager.get());

        // Selecting again changes nothing
        QVERIFY(!_selectionManager->selectNode(id(u"a"_s)));
        QCOMPARE(spy.count(), 1);

        QVERIFY(_selectionManager->nodeIsSelected(id(u"a"_s)));
        QVERIFY(!_selectionManager->nodeIsSelected(id(u"b"_s)));
        QVERIFY(_selectionManager->nodesAreSelected());
        QCOMPARE(_selectionManager->numNodesSelected(), 1u);

        QVERIFY(_selectionManager->selectNodes(nodeIds({u"a"_s, u"c"_s})));
        QVERIFY(_selectionManager->selectNodes(std::vector<NodeId>{id(u"d"_s)}));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"c"_s, u"d"_s}));
        QCOMPARE(spy.count(), 3);

        QVERIFY(_selectionManager->deselectNode(id(u"a"_s)));
        QVERIFY(!_selectionManager->deselectNode(id(u"a"_s)));
        QVERIFY(_selectionManager->deselectNodes(std::vector<NodeId>{id(u"d"_s), id(u"e"_s)}));
        QCOMPARE(selected(), QStringList{u"c"_s});
        QCOMPARE(spy.count(), 5);

        QVERIFY(_selectionManager->deselectNodes(nodeIds({u"c"_s})));
        QVERIFY(!_selectionManager->nodesAreSelected());
    }

    void toggle()
    {
        QVERIFY(_selectionManager->toggleNode(id(u"a"_s)));
        QCOMPARE(selected(), QStringList{u"a"_s});
        QVERIFY(!_selectionManager->toggleNode(id(u"a"_s)));
        QCOMPARE(selected(), QStringList{});
    }

    void selectAllAndClear()
    {
        const QSignalSpy spy(_selectionManager.get(), &SelectionManager::selectionChanged);

        QVERIFY(_selectionManager->selectAllNodes());
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s}));
        QVERIFY(!_selectionManager->selectAllNodes());

        QVERIFY(_selectionManager->clearNodeSelection());
        QVERIFY(!_selectionManager->clearNodeSelection());
        QCOMPARE(selected(), QStringList{});
        QCOMPARE(spy.count(), 2);
    }

    void invert()
    {
        _selectionManager->selectNodes(nodeIds({u"a"_s, u"b"_s}));
        _selectionManager->invertNodeSelection();
        QCOMPARE(selected(), (QStringList{u"c"_s, u"d"_s, u"e"_s}));

        _selectionManager->clearNodeSelection();
        _selectionManager->invertNodeSelection();
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s}));
    }

    void mask()
    {
        const QSignalSpy maskSpy(_selectionManager.get(), &SelectionManager::nodesMaskChanged);

        _selectionManager->selectNodes(nodeIds({u"a"_s, u"d"_s}));

        // Applying a mask deselects what's outside it
        QVERIFY(!_selectionManager->nodesMaskActive());
        _selectionManager->setNodesMask(nodeIds({u"a"_s, u"b"_s, u"c"_s}));
        QVERIFY(_selectionManager->nodesMaskActive());
        QCOMPARE(maskSpy.count(), 1);
        QCOMPARE(selected(), QStringList{u"a"_s});

        // Nothing outside the mask can be selected
        QVERIFY(!_selectionManager->selectNode(id(u"d"_s)));
        QVERIFY(_selectionManager->selectNodes(nodeIds({u"b"_s, u"e"_s})));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s}));

        QVERIFY(_selectionManager->selectAllNodes());
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        _selectionManager->deselectNode(id(u"a"_s));
        _selectionManager->invertNodeSelection();
        QCOMPARE(selected(), QStringList{u"a"_s});

        _selectionManager->clearNodesMask();
        QVERIFY(!_selectionManager->nodesMaskActive());
        QCOMPARE(maskSpy.count(), 2);
        QVERIFY(_selectionManager->selectNode(id(u"d"_s)));

        // A mask that isn't applied leaves the selection alone, but still
        // restricts what can be selected afterwards
        _selectionManager->setNodesMask(std::vector<NodeId>{id(u"e"_s)}, false);
        QCOMPARE(selected(), (QStringList{u"a"_s, u"d"_s}));
        QVERIFY(!_selectionManager->selectNode(id(u"b"_s)));
        QVERIFY(_selectionManager->selectNode(id(u"e"_s)));

        // Selecting all with a mask deselects what's outside it
        QVERIFY(_selectionManager->selectAllNodes());
        QCOMPARE(selected(), QStringList{u"e"_s});
    }

    void unselectedNodes()
    {
        QCOMPARE(names(_selectionManager->unselectedNodes()),
            (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s}));

        _selectionManager->selectNodes(nodeIds({u"a"_s, u"c"_s}));
        QCOMPARE(names(_selectionManager->unselectedNodes()), (QStringList{u"b"_s, u"d"_s, u"e"_s}));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"c"_s}));

        _selectionManager->selectAllNodes();
        QCOMPARE(names(_selectionManager->unselectedNodes()), QStringList{});
        QCOMPARE(_selectionManager->numNodesSelected(), 5u);
    }

    void mergedNodes()
    {
        QCOMPARE(_model->applyTransforms({u"ContractEdges where $Weight == 1"_s}), QStringList{});

        auto head = _model->graph().typeOf(id(u"a"_s)) == MultiElementType::Head ? id(u"a"_s) : id(u"b"_s);
        auto tail = head == id(u"a"_s) ? id(u"b"_s) : id(u"a"_s);
        QVERIFY(_model->graph().typeOf(tail) == MultiElementType::Tail);

        // Selecting a merged node selects everything merged with it
        QVERIFY(_selectionManager->selectNode(head));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s}));

        QVERIFY(_selectionManager->deselectNode(head));
        QCOMPARE(selected(), QStringList{});

        // Tails can't be selected or deselected on their own
        QVERIFY(!_selectionManager->selectNode(tail));
        QCOMPARE(selected(), QStringList{});

        _selectionManager->selectNode(head);
        QVERIFY(!_selectionManager->deselectNode(tail));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s}));

        // Selecting everything includes the tails
        _selectionManager->clearNodeSelection();
        _selectionManager->selectAllNodes();
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s}));
    }

    void removedNodesAreDeselected()
    {
        const QSignalSpy spy(_selectionManager.get(), &SelectionManager::selectionChanged);

        _selectionManager->selectNodes(nodeIds({u"a"_s, u"c"_s}));
        QCOMPARE(spy.count(), 1);

        auto a = id(u"a"_s);
        _model->graphModel().mutableGraph().performTransaction([a](IMutableGraph& graph)
        {
            graph.removeNode(a);
        });

        QCOMPARE(selected(), QStringList{u"c"_s});
        QCOMPARE(spy.count(), 2);
    }

    void numNodesSelectedAsString()
    {
        QCOMPARE(_selectionManager->numNodesSelectedAsString(), QString());

        _selectionManager->selectNode(id(u"a"_s));
        QCOMPARE(_selectionManager->numNodesSelectedAsString(), u"Selected a"_s);

        _selectionManager->selectNode(id(u"b"_s));
        QCOMPARE(_selectionManager->numNodesSelectedAsString(), u"2 Nodes Selected"_s);

        _selectionManager->clearNodeSelection();
        _model->graphModel().setNodeName(id(u"e"_s), {});
        _selectionManager->selectNode(id(u"e"_s));
        QCOMPARE(_selectionManager->numNodesSelectedAsString(), u"1 Node Selected"_s);
    }

    void suppressSignals()
    {
        const QSignalSpy spy(_selectionManager.get(), &SelectionManager::selectionChanged);

        // Only the next change is silent
        _selectionManager->suppressSignals();
        QVERIFY(_selectionManager->selectNode(id(u"a"_s)));
        QCOMPARE(spy.count(), 0);

        QVERIFY(_selectionManager->selectNode(id(u"b"_s)));
        QCOMPARE(spy.count(), 1);

        _selectionManager->suppressSignals();
        _selectionManager->invertNodeSelection();
        QCOMPARE(spy.count(), 1);
        _selectionManager->invertNodeSelection();
        QCOMPARE(spy.count(), 2);
    }

    void forEachSelectedNode()
    {
        _selectionManager->selectNodes(nodeIds({u"a"_s, u"b"_s, u"c"_s}));

        int count = 0;
        _selectionManager->forEachSelectedNode([&count](NodeId) { count++; return true; });
        QCOMPARE(count, 3);

        // Returning false stops
        count = 0;
        _selectionManager->forEachSelectedNode([&count](NodeId) { count++; return false; });
        QCOMPARE(count, 1);
    }
};

QTEST_MAIN(TestSelectionManager)
#include "tst_selectionmanager.moc"
