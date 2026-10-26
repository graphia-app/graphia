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

#include "app/graph/graphmodel.h"
#include "app/ui/searchmanager.h"

#include <QTest>
#include <QSignalSpy>
#include <QString>
#include <QStringList>

#include <map>
#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
template<typename T>
void addSearchableAttribute(TestGraphModel& model, const QString& name, const std::map<QString, T>& values,
    bool searchable = true)
{
    model.addNodeAttribute(name, values);

    if(searchable)
        model.graphModel().attributeByName(name)->setFlag(AttributeFlag::Searchable);
}

std::unique_ptr<TestGraphModel> createModel()
{
    auto model = std::make_unique<TestGraphModel>(QStringList{u"a-b"_s, u"c"_s, u"d"_s});

    addSearchableAttribute<QString>(*model, u"Description"_s, {{u"a"_s, u"Red apple"_s},
        {u"b"_s, u"Green pear"_s}, {u"c"_s, u"red grape"_s}, {u"d"_s, u"Blue berry"_s}});
    addSearchableAttribute<QString>(*model, u"Origin"_s, {{u"a"_s, u"Kent"_s},
        {u"b"_s, u"Kent"_s}, {u"c"_s, u"Bordeaux"_s}, {u"d"_s, u"Maine"_s}});
    addSearchableAttribute<QString>(*model, u"Hidden"_s, {{u"a"_s, u"x"_s},
        {u"b"_s, u"x"_s}, {u"c"_s, u"x"_s}, {u"d"_s, u"apple"_s}}, false);
    model->addEdgeAttribute<double>(u"Weight"_s, {{{u"a"_s, u"b"_s}, 1.0}});

    model->applyTransforms({});

    return model;
}

QStringList namesOf(TestGraphModel& model, const NodeIdSet& nodeIds)
{
    QStringList names;
    names.reserve(static_cast<qsizetype>(nodeIds.size()));
    for(auto nodeId : nodeIds)
        names.append(model.graphModel().nodeName(nodeId));

    names.sort();
    return names;
}
} // namespace

class TestSearchManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void find_data()
    {
        QTest::addColumn<QString>("term");
        QTest::addColumn<int>("options");
        QTest::addColumn<QStringList>("attributeNames");
        QTest::addColumn<QStringList>("expected");

        const auto none = 0;
        const auto matchCase = static_cast<int>(FindOptions::MatchCase);
        const auto wholeWords = static_cast<int>(FindOptions::MatchWholeWords);
        const auto regex = static_cast<int>(FindOptions::MatchUsingRegex);

        QTest::newRow("substring") << u"re"_s << none << QStringList{} <<
            QStringList{u"a"_s, u"b"_s, u"c"_s};
        QTest::newRow("case insensitive") << u"RED"_s << none << QStringList{} <<
            QStringList{u"a"_s, u"c"_s};
        QTest::newRow("match case") << u"Red"_s << matchCase << QStringList{} <<
            QStringList{u"a"_s};
        QTest::newRow("whole words") << u"pea"_s << wholeWords << QStringList{} << QStringList{};
        QTest::newRow("whole word") << u"pear"_s << wholeWords << QStringList{} << QStringList{u"b"_s};
        QTest::newRow("regex") << u"^(red|blue)"_s << regex << QStringList{} <<
            QStringList{u"a"_s, u"c"_s, u"d"_s};
        QTest::newRow("regex case") << u"^(red|blue)"_s << (regex | matchCase) << QStringList{} <<
            QStringList{u"c"_s};
        QTest::newRow("regex characters escaped") << u"^red"_s << none << QStringList{} << QStringList{};
        QTest::newRow("invalid regex") << u"(red"_s << regex << QStringList{} << QStringList{};
        QTest::newRow("one attribute") << u"n"_s << none << QStringList{u"Origin"_s} <<
            QStringList{u"a"_s, u"b"_s, u"d"_s};
        QTest::newRow("not searchable") << u"apple"_s << none << QStringList{} << QStringList{u"a"_s};
        QTest::newRow("only unsearchable") << u"apple"_s << none << QStringList{u"Hidden"_s} << QStringList{};
        QTest::newRow("unknown attribute") << u"apple"_s << none << QStringList{u"Unknown"_s} << QStringList{};
        QTest::newRow("no match") << u"kiwi"_s << none << QStringList{} << QStringList{};
    }

    void find()
    {
        QFETCH(const QString, term);
        QFETCH(const int, options);
        QFETCH(const QStringList, attributeNames);
        QFETCH(const QStringList, expected);

        auto model = createModel();
        SearchManager searchManager(model->graphModel());

        searchManager.findNodes(term, static_cast<FindOptions>(options), attributeNames, FindSelectStyle::None);
        QCOMPARE(namesOf(*model, searchManager.foundNodeIds()), expected);
        QCOMPARE(searchManager.active(), true);
    }

    void clear()
    {
        auto model = createModel();
        SearchManager searchManager(model->graphModel());
        const QSignalSpy spy(&searchManager, &SearchManager::foundNodeIdsChanged);

        searchManager.findNodes(u"red"_s, {}, {}, FindSelectStyle::None);
        QCOMPARE(spy.count(), 1);

        // Finding the same nodes again isn't a change
        searchManager.findNodes(u"RED"_s, {}, {}, FindSelectStyle::None);
        QCOMPARE(spy.count(), 1);

        searchManager.findNodes({}, {}, {}, FindSelectStyle::None);
        QCOMPARE(spy.count(), 2);
        QVERIFY(searchManager.foundNodeIds().empty());
        QVERIFY(!searchManager.active());
    }

    // Finding any node of a merged set finds all of them
    void mergedNodes()
    {
        auto model = createModel();
        QCOMPARE(model->applyTransforms({u"ContractEdges where $Weight == 1"_s}), QStringList{});

        SearchManager searchManager(model->graphModel());

        searchManager.findNodes(u"pear"_s, {}, {}, FindSelectStyle::None);
        QCOMPARE(namesOf(*model, searchManager.foundNodeIds()), (QStringList{u"a"_s, u"b"_s}));

        searchManager.findNodes(u"apple"_s, {}, {}, FindSelectStyle::None);
        QCOMPARE(namesOf(*model, searchManager.foundNodeIds()), (QStringList{u"a"_s, u"b"_s}));
    }

    void refresh()
    {
        auto model = createModel();
        SearchManager searchManager(model->graphModel());

        searchManager.findNodes(u"Kent"_s, {}, {u"Origin"_s}, FindSelectStyle::None);
        QCOMPARE(namesOf(*model, searchManager.foundNodeIds()), (QStringList{u"a"_s, u"b"_s}));

        model->graphModel().mutableGraph().removeNode(model->nodeId(u"b"_s));
        model->applyTransforms({});

        searchManager.refresh();
        QCOMPARE(namesOf(*model, searchManager.foundNodeIds()), (QStringList{u"a"_s}));
    }

    // Searching all attributes includes those created since the search began
    void refreshWithNewAttribute()
    {
        auto model = createModel();
        SearchManager searchManager(model->graphModel());

        searchManager.findNodes(u"cluster"_s, {}, {}, FindSelectStyle::None);
        QVERIFY(searchManager.foundNodeIds().empty());

        addSearchableAttribute<QString>(*model, u"Group"_s, {{u"a"_s, u"Cluster 1"_s},
            {u"b"_s, u"Cluster 1"_s}, {u"c"_s, u"Cluster 2"_s}, {u"d"_s, u"None"_s}});

        searchManager.refresh();
        QCOMPARE(namesOf(*model, searchManager.foundNodeIds()), (QStringList{u"a"_s, u"b"_s, u"c"_s}));
    }
};

QTEST_MAIN(TestSearchManager)
#include "tst_searchmanager.moc"
