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

#include "shared/utils/container.h"

#include <QTest>

#include <map>
#include <set>
#include <vector>

class TestContainer : public QObject
{
    Q_OBJECT

private slots:
    void remove()
    {
        std::vector<int> v{3, 1, 3, 2, 1};

        u::removeByValue(v, 3);
        QCOMPARE(v, (std::vector<int>{1, 2, 1}));

        u::removeDuplicates(v);
        QCOMPARE(v, (std::vector<int>{1, 2}));
    }

    void indexOf()
    {
        const std::vector<int> v{5, 6, 7, 6};
        QCOMPARE(u::indexOf(v, 6), 1);
        QCOMPARE(u::indexOf(v, 8), -1);
    }

    void contains()
    {
        const std::vector<int> v{1, 2, 3};
        const std::set<int> s{1, 2, 3};
        const std::map<int, char> m{{1, 'a'}, {2, 'b'}};

        QVERIFY(u::contains(v, 2));
        QVERIFY(!u::contains(v, 4));
        QVERIFY(u::contains(s, 2));
        QVERIFY(!u::contains(s, 4));
        QVERIFY(u::contains(m, 2));
        QVERIFY(!u::contains(m, 3));

        QVERIFY(u::containsAnyOf(v, {4, 3}));
        QVERIFY(!u::containsAnyOf(v, {4, 5}));
        QVERIFY(u::containsAllOf(v, {1, 3}));
        QVERIFY(!u::containsAllOf(v, {1, 4}));

        QVERIFY(u::containsKey(m, 1));
        QVERIFY(!u::containsKey(m, 3));
        QVERIFY(u::containsValue(m, 'b'));
        QVERIFY(!u::containsValue(m, 'c'));
    }

    void setOperations()
    {
        const std::vector<int> a{1, 2, 3, 3};
        const std::vector<int> b{3, 4};

        QCOMPARE(u::setDifference(a, b), (std::vector<int>{1, 2}));
        QCOMPARE(u::multisetDifference(a, b), (std::vector<int>{1, 2, 3}));
        QCOMPARE(u::setIntersection(a, b), (std::vector<int>{3, 3}));
        QVERIFY(u::setsIntersect(a, b));
        QVERIFY(!u::setsIntersect(a, std::vector<int>{5}));
    }

    void setsDiffer_data()
    {
        QTest::addColumn<std::vector<int>>("a");
        QTest::addColumn<std::vector<int>>("b");
        QTest::addColumn<bool>("differ");

        QTest::newRow("same") << std::vector<int>{1, 2} << std::vector<int>{1, 2} << false;
        QTest::newRow("different order") << std::vector<int>{2, 1} << std::vector<int>{1, 2} << false;
        QTest::newRow("different values") << std::vector<int>{1, 2} << std::vector<int>{1, 3} << true;
        QTest::newRow("different sizes") << std::vector<int>{1, 2} << std::vector<int>{1, 2, 3} << true;
        QTest::newRow("both empty") << std::vector<int>{} << std::vector<int>{} << false;
    }

    void setsDiffer()
    {
        QFETCH(const std::vector<int>, a);
        QFETCH(const std::vector<int>, b);
        QFETCH(const bool, differ);

        QCOMPARE(u::setsDiffer(a, b), differ);
        QCOMPARE(u::setsDiffer(b, a), differ);
        QCOMPARE(u::setsEqual(a, b), !differ);
    }

    void keysAndValues()
    {
        const std::map<int, char> m{{2, 'b'}, {1, 'a'}};

        QCOMPARE(u::keysFor(m), (std::vector<int>{1, 2}));
        QCOMPARE(u::vectorFrom(std::set<int>{3, 1}), (std::vector<int>{1, 3}));
    }

    void reverse()
    {
        const std::vector<int> v{1, 2, 3};
        std::vector<int> reversed;
        reversed.reserve(v.size());

        for(auto i : u::reverse(v))
            reversed.push_back(i);

        QCOMPARE(reversed, (std::vector<int>{3, 2, 1}));
    }

    void sortedIndicesOf()
    {
        QCOMPARE(u::sortedIndicesOf(std::vector<double>{3.0, 1.0, 2.0}), (std::vector<size_t>{1, 2, 0}));
    }

    void rankingOf_data()
    {
        QTest::addColumn<std::vector<double>>("values");
        QTest::addColumn<std::vector<double>>("expected");

        QTest::newRow("empty") << std::vector<double>{} << std::vector<double>{};
        QTest::newRow("distinct") << std::vector<double>{30.0, 10.0, 20.0} << std::vector<double>{3.0, 1.0, 2.0};
        QTest::newRow("ties") << std::vector<double>{5.0, 1.0, 5.0, 3.0, 5.0} <<
            std::vector<double>{4.0, 1.0, 4.0, 2.0, 4.0};
        QTest::newRow("all tied") << std::vector<double>{2.0, 2.0} << std::vector<double>{1.5, 1.5};
    }

    void rankingOf()
    {
        QFETCH(const std::vector<double>, values);
        QFETCH(const std::vector<double>, expected);

        QCOMPARE(u::rankingOf(values), expected);
    }

    void hasUniqueValues()
    {
        QVERIFY(u::hasUniqueValues(std::vector<int>{3, 1, 2}));
        QVERIFY(!u::hasUniqueValues(std::vector<int>{3, 1, 3}));
        QVERIFY(u::hasUniqueValues(std::vector<int>{}));
    }
};

QTEST_GUILESS_MAIN(TestContainer)
#include "tst_container.moc"
