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

#include "app/ui/visualisations/visualisationmapping.h"

#include "shared/utils/statistics.h"

#include <QTest>
#include <QString>

#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// Mean 5, standard deviation 2, min 2, max 9
u::Statistics sampleStatistics()
{
    return u::findStatisticsFor(std::vector<double>{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0});
}
} // namespace

class TestVisualisationMapping : public QObject
{
    Q_OBJECT

private slots:
    void range_data()
    {
        QTest::addColumn<QString>("descriptor");
        QTest::addColumn<double>("min");
        QTest::addColumn<double>("max");

        QTest::newRow("default") << QString() << 2.0 << 9.0;
        QTest::newRow("minmax") << u"{\"type\": \"minmax\"}"_s << 2.0 << 9.0;
        QTest::newRow("minmax, case insensitive") << u"{\"type\": \"MinMax\"}"_s << 2.0 << 9.0;
        QTest::newRow("stddev") << u"{\"type\": \"stddev\"}"_s << 3.0 << 7.0;
        QTest::newRow("explicit") << u"{\"min\": 3, \"max\": 8}"_s << 3.0 << 8.0;
        QTest::newRow("explicit, clamped to sample") << u"{\"min\": 0, \"max\": 100}"_s << 2.0 << 9.0;
        QTest::newRow("explicit, no range") << u"{\"min\": 4, \"max\": 4}"_s << 2.0 << 9.0;
        QTest::newRow("only min") << u"{\"min\": 4}"_s << 2.0 << 9.0;
        QTest::newRow("not JSON") << u"{min"_s << 2.0 << 9.0;
        QTest::newRow("not an object") << u"[1]"_s << 2.0 << 9.0;
        QTest::newRow("type not a string") << u"{\"type\": 1}"_s << 2.0 << 9.0;
        QTest::newRow("min not a number") << u"{\"min\": \"3\", \"max\": 8}"_s << 2.0 << 9.0;
        QTest::newRow("exponent not a number") << u"{\"min\": 3, \"max\": 8, \"exponent\": \"2\"}"_s << 3.0 << 8.0;
    }

    void range()
    {
        QFETCH(const QString, descriptor);
        QFETCH(const double, min);
        QFETCH(const double, max);

        const VisualisationMapping mapping(sampleStatistics(), descriptor);
        QCOMPARE(mapping.min(), min);
        QCOMPARE(mapping.max(), max);
    }

    void map()
    {
        const VisualisationMapping mapping(sampleStatistics(), u"{\"min\": 3, \"max\": 7}"_s);

        QCOMPARE(mapping.map(3.0), 0.0);
        QCOMPARE(mapping.map(5.0), 0.5);
        QCOMPARE(mapping.map(7.0), 1.0);

        // Values outside the range are clamped
        QCOMPARE(mapping.map(2.0), 0.0);
        QCOMPARE(mapping.map(9.0), 1.0);
    }

    void exponent()
    {
        const VisualisationMapping mapping(sampleStatistics(), u"{\"min\": 3, \"max\": 7, \"exponent\": 2}"_s);

        QCOMPARE(mapping.map(5.0), 0.25);
        QCOMPARE(mapping.map(7.0), 1.0);
    }

    void constantValues()
    {
        const VisualisationMapping mapping(u::findStatisticsFor(std::vector<double>{3.0, 3.0}), QString());

        QCOMPARE(mapping.map(3.0), 0.0);
    }
};

QTEST_GUILESS_MAIN(TestVisualisationMapping)
#include "tst_visualisationmapping.moc"
