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

#include "shared/utils/statistics.h"

#include <QTest>

#include <cmath>
#include <vector>

class TestStatistics : public QObject
{
    Q_OBJECT

private slots:
    void statistics()
    {
        // Expected values computed with numpy
        const std::vector<double> values{3.0, -7.5, 2.0, 0.5, 4.0};
        auto s = u::findStatisticsFor(values, "includeValues"_yes);

        QCOMPARE(s._values, values);
        QCOMPARE(s._min, -7.5);
        QCOMPARE(s._max, 4.0);
        QCOMPARE(s._range, 11.5);
        QCOMPARE(s._largestIndex, 1u);
        QCOMPARE(s._sum, 2.0);
        QCOMPARE(s._sumSq, 85.5);
        QCOMPARE(s._sumAllSq, 4.0);
        QCOMPARE(s._mean, 0.4);
        QCOMPARE(s._variance, 16.94);
        QCOMPARE(s._stddev, 4.115823125451336);
        QCOMPARE(s._magnitude, 9.246621004453464);
        QCOMPARE(s._variability, 20.579115627256677);

        // Not all positive
        QVERIFY(std::isnan(s._coefVar));

        QVERIFY(u::findStatisticsFor(values)._values.empty());
    }

    void coefficientOfVariation()
    {
        auto s = u::findStatisticsFor(std::vector<double>{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0});
        QCOMPARE(s._mean, 5.0);
        QCOMPARE(s._stddev, 2.0);
        QCOMPARE(s._coefVar, 0.4);
    }

    void function()
    {
        const std::vector<int> values{1, 2, 3};
        auto s = u::findStatisticsFor(values, [](int v) { return v * 10.0; });

        QCOMPARE(s._sum, 60.0);
        QCOMPARE(s._mean, 20.0);
        QCOMPARE(s._largestIndex, 2u);
    }

    void constant()
    {
        auto s = u::findStatisticsFor(std::vector<double>{5.0, 5.0, 5.0});

        QCOMPARE(s._range, 0.0);
        QCOMPARE(s._variance, 0.0);
        QCOMPARE(s._stddev, 0.0);
        QCOMPARE(s._coefVar, 0.0);
    }

    void inverse()
    {
        auto s = u::findStatisticsFor(std::vector<double>{10.0, 20.0, 15.0});

        QCOMPARE(s.inverse(10.0), 20.0);
        QCOMPARE(s.inverse(20.0), 10.0);
        QCOMPARE(s.inverse(12.0), 18.0);
    }

    void median_data()
    {
        QTest::addColumn<std::vector<double>>("values");
        QTest::addColumn<double>("expected");

        QTest::newRow("empty") << std::vector<double>{} << 0.0;
        QTest::newRow("one") << std::vector<double>{3.0} << 3.0;
        QTest::newRow("odd") << std::vector<double>{3.0, -7.5, 2.0, 0.5, 4.0} << 2.0;
        QTest::newRow("even") << std::vector<double>{3.0, -7.5, 2.0, 0.5} << 1.25;
        QTest::newRow("even, equal middle values") << std::vector<double>{1.0, 2.0, 2.0, 9.0} << 2.0;
    }

    void median()
    {
        QFETCH(const std::vector<double>, values);
        QFETCH(const double, expected);

        QCOMPARE(u::medianOf(values), expected);
    }
};

QTEST_GUILESS_MAIN(TestStatistics)
#include "tst_statistics.moc"
