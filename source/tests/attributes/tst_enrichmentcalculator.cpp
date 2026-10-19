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

#include "app/attributes/enrichmentcalculator.h"

#include "shared/commands/icommand.h"

#include <QTest>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>

using namespace Qt::Literals::StringLiterals;

namespace
{
uint64_t binomial(uint64_t n, uint64_t k)
{
    if(k > n)
        return 0;

    uint64_t result = 1;
    for(uint64_t i = 1; i <= std::min(k, n - k); i++)
        result = result * (n - i + 1) / i;

    return result;
}

// Fisher's exact test, two-sided, computed using integers, so that tables whose
// probabilities are equal are identified exactly; this is only practical for small tables
double exactFishers(uint64_t a, uint64_t b, uint64_t c, uint64_t d)
{
    const auto ab = a + b;
    const auto cd = c + d;
    const auto ac = a + c;

    // The probability of each table is proportional to this
    auto weight = [&](uint64_t x) { return binomial(ab, x) * binomial(cd, ac - x); };

    const auto observed = weight(a); // clazy:exclude=compare-member-check
    uint64_t sum = 0;

    for(uint64_t x = ac > cd ? ac - cd : 0; x <= std::min(ac, ab); x++)
    {
        if(weight(x) <= observed) // clazy:exclude=compare-member-check
            sum += weight(x); // clazy:exclude=compare-member-check
    }

    return static_cast<double>(sum) / static_cast<double>(binomial(ab + cd, ac));
}

bool approximatelyEqual(double actual, double expected)
{
    return std::abs(actual - expected) <= 1e-9 * std::max(std::abs(expected), 1e-12);
}

class TestCommand : public ICommand
{
public:
    QString description() const override { return u"Test"_s; }
    bool execute() override { return true; }
};
} // namespace

class TestEnrichmentCalculator : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void fishers_data()
    {
        QTest::addColumn<int>("a");
        QTest::addColumn<int>("b");
        QTest::addColumn<int>("c");
        QTest::addColumn<int>("d");
        QTest::addColumn<double>("expected");

        // Computed exactly, with Python's fractions and math.comb
        QTest::newRow("Small") << 3 << 1 << 1 << 3 << 0.4857142857142857;
        QTest::newRow("Significant") << 1 << 9 << 11 << 3 << 0.0027594561852200836;
        QTest::newRow("Very significant") << 10 << 2 << 3 << 15 << 0.0005367241191434358;
        QTest::newRow("Perfect association") << 5 << 0 << 0 << 5 << 0.007936507936507936;
        QTest::newRow("Zero cell") << 7 << 12 << 0 << 5 << 0.272068511198946;
        QTest::newRow("Large") << 100 << 50 << 40 << 120 << 1.2818103033369866e-13;
    }

    void fishers()
    {
        QFETCH(const int, a);
        QFETCH(const int, b);
        QFETCH(const int, c);
        QFETCH(const int, d);
        QFETCH(const double, expected);

        auto actual = EnrichmentCalculator::fishers(static_cast<size_t>(a), static_cast<size_t>(b),
            static_cast<size_t>(c), static_cast<size_t>(d));
        QVERIFY2(approximatelyEqual(actual, expected),
            qPrintable(u"%1 != %2"_s.arg(actual, 0, 'g', 17).arg(expected, 0, 'g', 17)));
    }

    void overRepresentation()
    {
        // Cluster 1 is a to d, which are mostly of type P
        TestGraphModel model({u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s, u"f"_s, u"g"_s, u"h"_s, u"i"_s});
        model.addNodeAttribute<QString>(u"Cluster"_s, {{u"a"_s, u"1"_s}, {u"b"_s, u"1"_s},
            {u"c"_s, u"1"_s}, {u"d"_s, u"1"_s}, {u"e"_s, u"2"_s}, {u"f"_s, u"2"_s},
            {u"g"_s, u"2"_s}, {u"h"_s, u"2"_s}, {u"i"_s, u""_s}});
        model.addNodeAttribute<QString>(u"Type"_s, {{u"a"_s, u"P"_s}, {u"b"_s, u"P"_s},
            {u"c"_s, u"P"_s}, {u"d"_s, u"Q"_s}, {u"e"_s, u"Q"_s}, {u"f"_s, u"Q"_s},
            {u"g"_s, u"Q"_s}, {u"h"_s, u"Q"_s}, {u"i"_s, u"Q"_s}});
        model.applyTransforms({});

        TestCommand command;
        auto table = EnrichmentCalculator::overRepAgainstEachAttribute(
            u"Cluster"_s, u"Type"_s, &model.graphModel(), command);

        // Every cluster against every type, excluding the missing value
        QCOMPARE(table.size(), 4);

        using Results = EnrichmentTableModel::Results;
        std::map<QString, EnrichmentTableModel::Row> rows;
        for(const auto& row : table)
        {
            rows.emplace(row.at(Results::SelectionA).toString() + u"/"_s +
                row.at(Results::SelectionB).toString(), row);
        }

        QCOMPARE(rows.size(), 4);

        // Cluster 1 and type P: 3 of the 4 nodes in the cluster are of the type, and
        // 3 of the 9 nodes are, so 4 * 3 / 9 are expected, and the contingency table
        // is 3 selected in category, 0 not selected in category, 1 selected not in
        // category, and the remaining 5 neither
        const auto& row = rows.at(u"1/P"_s);
        QCOMPARE(row.at(Results::Observed).toString(), u"3 of 4"_s);
        QVERIFY(row.at(Results::ExpectedTrial).toString().startsWith(u"1.33 ± "_s));
        QVERIFY(row.at(Results::ExpectedTrial).toString().endsWith(u" of 4"_s));
        QVERIFY(approximatelyEqual(row.at(Results::OverRep).toDouble(), 3.0 / (4.0 * 3.0 / 9.0)));

        const auto expectedFishers = exactFishers(3, 0, 1, 5);
        QVERIFY(approximatelyEqual(row.at(Results::Fishers).toDouble(), expectedFishers));

        // There are 2 types, so 2 comparisons for each cluster
        QVERIFY(approximatelyEqual(row.at(Results::BonferroniAdjusted).toDouble(),
            std::min(1.0, expectedFishers * 2.0)));

        // The spread of the expected count is estimated by random sampling, so is only
        // approximately that of the binomial distribution
        auto expectedTrial = row.at(Results::ExpectedTrial).toString();
        auto spread = expectedTrial.section(u' ', 2, 2).toDouble();
        const double p = 3.0 / 9.0;
        QVERIFY2(std::abs(spread - std::sqrt(4.0 * p * (1.0 - p))) < 0.2, qPrintable(expectedTrial));

        QCOMPARE(rows.at(u"2/P"_s).at(Results::Observed).toString(), u"0 of 4"_s);
        QCOMPARE(rows.at(u"2/Q"_s).at(Results::Observed).toString(), u"4 of 4"_s);
    }
};

QTEST_MAIN(TestEnrichmentCalculator)
#include "tst_enrichmentcalculator.moc"
