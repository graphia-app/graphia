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

#include "app/attributes/enrichmenttablemodel.h"

#include <QTest>
#include <QString>
#include <QStringList>

using namespace Qt::Literals::StringLiterals;

namespace
{
using Results = EnrichmentTableModel::Results;

// Selections A and B, observed, expected, over-representation, Fisher's and Bonferroni adjusted
const EnrichmentTableModel::Table Data
{
    {u"a1"_s,   u"b1"_s, 3.0, 1.5, 2.0, 0.01,  0.02},  // Enriched
    {u"a2"_s,   u"b1"_s, 1.0, 2.0, 0.5, 0.9,   1.0},   // Under-represented
    {u"a10"_s,  u"b2"_s, 6.0, 2.0, 3.0, 0.001, 0.003}, // Enriched
    {u"a3"_s,   u"b2"_s, 4.0, 2.0, 2.0, 0.04,  0.08},  // Not significant once adjusted
};

// The values of a column, in the order they're presented, excluding the header
QStringList column(const EnrichmentTableModel& model, Results result)
{
    QStringList values;
    values.reserve(model.rowCount());
    for(int row = 1; row < model.rowCount(); row++)
        values.append(model.data(row, result).toString());

    return values;
}
} // namespace

class TestEnrichmentTableModel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void setTableData()
    {
        EnrichmentTableModel model;
        model.setTableData(Data, u"A"_s, u"B"_s);

        QCOMPARE(model.tableData(), Data);
        QCOMPARE(model.columnCount(), static_cast<int>(Results::NumResultColumns));

        // The first row is the header, which names the selections generically until they're given names
        QCOMPARE(model.data(0, Results::SelectionA).toString(), u"Selection A"_s);
        QCOMPARE(model.data(0, Results::SelectionB).toString(), u"Selection B"_s);

        model.setSelectionDisplayNames(u"Attribute A"_s, u"Attribute B"_s);
        QCOMPARE(model.data(0, Results::SelectionA).toString(), u"Attribute A"_s);
        QCOMPARE(model.data(0, Results::SelectionB).toString(), u"Attribute B"_s);
    }

    // The values of the selections can be shown differently to how they're stored
    void displayValues()
    {
        EnrichmentTableModel model;
        model.setTableData(Data, u"A"_s, u"B"_s);
        model.setAttributeDisplayValueFn([](const QString& attributeName, const QString& value)
            { return u"%1:%2"_s.arg(attributeName, value); });

        QCOMPARE(column(model, Results::SelectionA), (QStringList{u"A:a1"_s, u"A:a10"_s}));
        QCOMPARE(column(model, Results::SelectionB), (QStringList{u"B:b1"_s, u"B:b2"_s}));

        // The data itself is unchanged
        QCOMPARE(model.tableData(), Data);
    }

    void enrichedOnly()
    {
        EnrichmentTableModel model;
        model.setTableData(Data, u"A"_s, u"B"_s);

        // By default, only enriched results are presented
        QCOMPARE(column(model, Results::SelectionA), (QStringList{u"a1"_s, u"a10"_s}));

        QVERIFY(model.setProperty("enrichedOnly", false));
        QCOMPARE(column(model, Results::SelectionA), (QStringList{u"a1"_s, u"a2"_s, u"a10"_s, u"a3"_s}));

        QVERIFY(model.data(model.index(1, 0), EnrichmentTableModel::EnrichedRole).toBool());
        QVERIFY(!model.data(model.index(2, 0), EnrichmentTableModel::EnrichedRole).toBool());
        QVERIFY(!model.data(model.index(4, 0), EnrichmentTableModel::EnrichedRole).toBool());

        // The data itself isn't filtered
        QCOMPARE(model.tableData(), Data);
    }

    void sort()
    {
        EnrichmentTableModel model;
        model.setTableData(Data, u"A"_s, u"B"_s);
        QVERIFY(model.setProperty("enrichedOnly", false));

        // Strings are sorted with numbers in order
        QVERIFY(model.setProperty("sortColumn", static_cast<int>(Results::SelectionA)));
        QCOMPARE(column(model, Results::SelectionA), (QStringList{u"a1"_s, u"a2"_s, u"a3"_s, u"a10"_s}));

        QVERIFY(model.setProperty("ascendingSortOrder", false));
        QCOMPARE(column(model, Results::SelectionA), (QStringList{u"a10"_s, u"a3"_s, u"a2"_s, u"a1"_s}));

        QVERIFY(model.setProperty("sortColumn", static_cast<int>(Results::Fishers)));
        QVERIFY(model.setProperty("ascendingSortOrder", true));
        QCOMPARE(column(model, Results::SelectionA), (QStringList{u"a10"_s, u"a1"_s, u"a3"_s, u"a2"_s}));

        // The data itself isn't sorted
        QCOMPARE(model.tableData(), Data);
    }

    void rowFromAttributeSets()
    {
        EnrichmentTableModel model;
        model.setTableData(Data, u"A"_s, u"B"_s);

        // Rows are as presented
        QCOMPARE(model.rowFromAttributeSets(u"a10"_s, u"b2"_s), 2);
        QCOMPARE(model.rowFromAttributeSets(u"a2"_s, u"b1"_s), -1);
        QCOMPARE(model.rowFromAttributeSets(u"a1"_s, u"b2"_s), -1);

        QVERIFY(model.setProperty("enrichedOnly", false));
        QCOMPARE(model.rowFromAttributeSets(u"a2"_s, u"b1"_s), 2);
    }
};

QTEST_GUILESS_MAIN(TestEnrichmentTableModel)
#include "tst_enrichmenttablemodel.moc"
