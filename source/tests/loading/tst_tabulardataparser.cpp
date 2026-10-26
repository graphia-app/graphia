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

#include "app/ui/qml/Graphia/Loading/adjacencymatrixtabulardataparser.h"
#include "app/ui/qml/Graphia/Loading/tabulardataparser.h"

#include "shared/attributes/valuetype.h"
#include "shared/loading/tabulardata.h"

#include <QAbstractItemModel>
#include <QByteArray>
#include <QFile>
#include <QList>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QVariantMap>
#include <QVector>

#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// A column of each type, a column with a duplicated value, one with no header, and a repeated header
const QByteArray Csv =
    "Name,Count,Score,Label,,Count\n"
    "a,1,1.5,x,q,4\n"
    "b,2,2.5,x,r,5\n"
    "c,3,3.5,y,s,6\n";
} // namespace

// The parsers the dialogs for importing tables, and for opening pairwise and matrix files, use
class TestTabularDataParser : public QObject
{
    Q_OBJECT

    QTemporaryDir _dir;

    QUrl write(const QString& fileName, const QByteArray& content)
    {
        auto filePath = _dir.filePath(fileName);
        QFile file(filePath);
        if(!file.open(QIODevice::WriteOnly) || file.write(content) != content.size())
            return {};

        return QUrl::fromLocalFile(filePath);
    }

    static QAbstractItemModel* model(TabularDataParser& parser) { return parser.tableModel(); }

    static bool parse(TabularDataParser& parser, const QUrl& url)
    {
        const int timeout = 10000;

        QSignalSpy complete(&parser, &TabularDataParser::completeChanged);
        parser.parse(url);

        return QTest::qWaitFor([&]
        {
            return !complete.isEmpty() && parser.property("complete").toBool() && !parser.busy();
        }, timeout);
    }

    // The header values a model lists, and the index of each in the table
    static std::pair<QStringList, std::vector<int>> headers(const TabularDataHeaderModel* model)
    {
        QStringList values;
        std::vector<int> indices;

        for(int row = 0; row < model->rowCount({}); row++)
        {
            auto index = model->index(row);
            values.append(model->data(index, Qt::DisplayRole).toString());
            indices.push_back(model->indexFor(index));
        }

        return {values, indices};
    }

    // The estimate for the lowest threshold, i.e. every edge
    static std::pair<double, double> nodesAndUniqueEdges(const AdjacencyMatrixTabularDataParser& parser)
    {
        const auto estimate = parser.property("graphSizeEstimate").toMap();
        const auto numNodes = estimate.value(u"numNodes"_s).value<QVector<double>>();
        const auto numUniqueEdges = estimate.value(u"numUniqueEdges"_s).value<QVector<double>>();

        if(numNodes.isEmpty() || numUniqueEdges.isEmpty())
            return {0.0, 0.0};

        return {numNodes.front(), numUniqueEdges.front()};
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        QVERIFY(_dir.isValid());
    }

    void parse()
    {
        TabularDataParser parser;
        const QSignalSpy dataLoaded(&parser, &TabularDataParser::dataLoaded);
        QVERIFY(parse(parser, write(u"table.csv"_s, Csv)));

        QVERIFY(!parser.property("failed").toBool());
        QCOMPARE(dataLoaded.count(), 1);

        const auto data = parser.property("data").value<std::shared_ptr<TabularData>>();
        QVERIFY(data != nullptr);
        QCOMPARE(data->numColumns(), 6uz);
        QCOMPARE(data->numRows(), 4uz);

        auto* tableModel = model(parser);
        QCOMPARE(tableModel->rowCount(), 4);
        QCOMPARE(tableModel->columnCount(), 6);
        QCOMPARE(tableModel->data(tableModel->index(2, 2)).toString(), u"2.5"_s);

        // A table whose file has the wrong extension
        QVERIFY(parse(parser, write(u"table.dat"_s, Csv)));
        QVERIFY(!parser.property("failed").toBool());
        QCOMPARE(model(parser)->columnCount(), 6);
    }

    // The headers of the columns, i.e. the first row, as offered to choose the key column
    void headers()
    {
        TabularDataParser parser;
        QVERIFY(parse(parser, write(u"table.csv"_s, Csv)));

        auto check = [&parser](int valueTypes, const QStringList& skip,
            const QStringList& expectedValues, const std::vector<int>& expectedIndices)
        {
            const std::unique_ptr<TabularDataHeaderModel> model(parser.rowHeaders(valueTypes, skip));
            const auto [values, indices] = headers(model.get());

            return values == expectedValues && indices == expectedIndices;
        };

        // The column without a header isn't offered
        QVERIFY(check(static_cast<int>(ValueType::All), {},
            {u"Name"_s, u"Count"_s, u"Score"_s, u"Label"_s, u"Count"_s}, {0, 1, 2, 3, 5}));
        QVERIFY(check(static_cast<int>(ValueType::Identifier), {},
            {u"Name"_s, u"Count"_s, u"Label"_s, u"Count"_s}, {0, 1, 3, 5}));
        QVERIFY(check(static_cast<int>(ValueType::Numerical), {},
            {u"Count"_s, u"Score"_s, u"Count"_s}, {1, 2, 5}));
        QVERIFY(check(static_cast<int>(ValueType::String), {u"Name"_s},
            {u"Label"_s}, {3}));

        const std::unique_ptr<TabularDataHeaderModel> model(parser.rowHeaders());
        QCOMPARE(model->modelIndexOf(3).row(), 3);
        QCOMPARE(model->modelIndexOf(5).row(), 4);
        QVERIFY(!model->modelIndexOf(4).isValid());
        QVERIFY(model->roleNames().values().contains("indexRole"));

        // The headers of the rows, i.e. the first column
        const std::unique_ptr<TabularDataHeaderModel> rowHeaders(parser.columnHeaders(
            static_cast<int>(ValueType::All), {u"b"_s}));
        const auto [values, indices] = headers(rowHeaders.get());
        QCOMPARE(values, (QStringList{u"Name"_s, u"a"_s, u"c"_s}));
        QCOMPARE(indices, (std::vector<int>{0, 1, 3}));
    }

    void duplicates()
    {
        TabularDataParser parser;
        QVERIFY(parse(parser, write(u"table.csv"_s, Csv)));

        // Below the header
        QCOMPARE(parser.columnHasDuplicates(0), false);
        QCOMPARE(parser.columnHasDuplicates(3), true);

        // Right of the header
        QCOMPARE(parser.rowHasDuplicates(0), true);
        QCOMPARE(parser.rowHasDuplicates(1), false);
    }

    void failure()
    {
        TabularDataParser parser;
        const QSignalSpy failedChanged(&parser, &TabularDataParser::failedChanged);
        QVERIFY(parse(parser, write(u"empty.csv"_s, {})));

        QVERIFY(parser.property("failed").toBool());
        QCOMPARE(failedChanged.count(), 1);
        QVERIFY(!parser.property("failureReason").toString().isEmpty());

        // Parsing again starts afresh
        QVERIFY(parse(parser, write(u"table.csv"_s, Csv)));
        QVERIFY(!parser.property("failed").toBool());
        QCOMPARE(model(parser)->rowCount(), 4);

        parser.reset();
        QVERIFY(!parser.property("complete").toBool());
        QVERIFY(parser.property("data").value<std::shared_ptr<TabularData>>()->empty());
    }

    // The dialog for opening a matrix or edge list estimates the size of the graph
    void matrix_data()
    {
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<bool>("binary");
        QTest::addColumn<double>("numNodes");
        QTest::addColumn<double>("numUniqueEdges");

        QTest::newRow("Edge list") << QByteArray("0,1,1\n1,2,1\n2,0,1\n") << true << 3.0 << 3.0;

        QTest::newRow("Weighted matrix") << QByteArray(
            "0,0.5,0\n"
            "0.5,0,2\n"
            "0,2,0\n") << false << 3.0 << 2.0;

        // In each, a and b are connected
        QTest::newRow("Row headers") << QByteArray(
            "a,0,1,0\n"
            "b,1,0,0\n"
            "c,0,0,0\n") << true << 2.0 << 1.0;

        QTest::newRow("Column headers") << QByteArray(
            "a,b,c\n"
            "0,1,0\n"
            "1,0,0\n"
            "0,0,0\n") << true << 2.0 << 1.0;

        QTest::newRow("Both headers") << QByteArray(
            ",a,b,c\n"
            "a,0,1,0\n"
            "b,1,0,0\n"
            "c,0,0,0\n") << true << 2.0 << 1.0;
    }

    void matrix()
    {
        QFETCH(const QByteArray, content);
        QFETCH(const bool, binary);
        QFETCH(const double, numNodes);
        QFETCH(const double, numUniqueEdges);

        AdjacencyMatrixTabularDataParser parser;
        QVERIFY(parse(parser, write(u"matrix.csv"_s, content)));
        QVERIFY(!parser.property("failed").toBool());

        QCOMPARE(parser.property("binaryMatrix").toBool(), binary);
        QCOMPARE(nodesAndUniqueEdges(parser), std::make_pair(numNodes, numUniqueEdges));
    }

    void notAMatrix()
    {
        AdjacencyMatrixTabularDataParser parser;
        QVERIFY(parse(parser, write(u"matrix.csv"_s, "a,b\nc,d\n")));

        QVERIFY(parser.property("failed").toBool());
        QCOMPARE(parser.property("failureReason").toString(), u"Matrix has non-numeric values."_s);
    }
};

QTEST_MAIN(TestTabularDataParser)
#include "tst_tabulardataparser.moc"
