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
#include "tests/common/xlsxwriter.h"

#include "shared/loading/tabulardata.h"
#include "shared/loading/xlsxtabulardataparser.h"
#include "shared/utils/typeidentity.h"

#include <QTest>
#include <QByteArray>
#include <QFile>
#include <QList>
#include <QRect>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>

#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
using Cells = QList<QStringList>;

std::unique_ptr<TextDelimitedTabularDataParser> createParser(const QString& type)
{
    if(type == u"CSV"_s) return std::make_unique<CsvFileParser>();
    if(type == u"TSV"_s) return std::make_unique<TsvFileParser>();
    if(type == u"TXT"_s) return std::make_unique<TxtFileParser>();
    if(type == u"SSV"_s) return std::make_unique<SsvFileParser>();

    return nullptr;
}

Cells cellsOf(const TabularData& tabularData)
{
    Cells cells;
    cells.reserve(static_cast<qsizetype>(tabularData.numRows()));

    for(size_t row = 0; row < tabularData.numRows(); row++)
    {
        auto& rowCells = cells.emplace_back();
        rowCells.reserve(static_cast<qsizetype>(tabularData.numColumns()));

        for(size_t column = 0; column < tabularData.numColumns(); column++)
            rowCells.append(tabularData.valueAt(column, row));
    }

    return cells;
}

TabularData tabularDataOf(const Cells& cells)
{
    TabularData tabularData;
    tabularData.reserve(static_cast<size_t>(cells.at(0).size()), static_cast<size_t>(cells.size()));

    for(qsizetype row = 0; row < cells.size(); row++)
    {
        for(qsizetype column = 0; column < cells.at(row).size(); column++)
        {
            tabularData.setValueAt(static_cast<size_t>(column), static_cast<size_t>(row),
                cells.at(row).at(column));
        }
    }

    return tabularData;
}

Cells cellsFrom(const QString& csv)
{
    const auto rows = csv.split(u'\n');

    Cells cells;
    cells.reserve(rows.size());
    for(const auto& row : rows)
        cells.append(row.split(u','));

    return cells;
}
} // namespace

class TestTabularData : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir _dir;

    QUrl writeFile(const QByteArray& content, const QString& name = u"data"_s)
    {
        auto fileName = _dir.filePath(name);

        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return {};

        file.write(content);
        return QUrl::fromLocalFile(fileName);
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        QVERIFY(_dir.isValid());
    }

    void parse_data()
    {
        QTest::addColumn<QString>("type");
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<Cells>("expected");

        QTest::newRow("CSV") << u"CSV"_s << QByteArray("a,b,c\n1,2,3\n") <<
            Cells{{u"a"_s, u"b"_s, u"c"_s}, {u"1"_s, u"2"_s, u"3"_s}};
        QTest::newRow("CSV no final newline") << u"CSV"_s << QByteArray("a,b\n1,2") <<
            Cells{{u"a"_s, u"b"_s}, {u"1"_s, u"2"_s}};
        QTest::newRow("CSV CRLF") << u"CSV"_s << QByteArray("a,b\r\n1,2\r\n") <<
            Cells{{u"a"_s, u"b"_s}, {u"1"_s, u"2"_s}};
        QTest::newRow("CSV CR") << u"CSV"_s << QByteArray("a,b\r1,2\r") <<
            Cells{{u"a"_s, u"b"_s}, {u"1"_s, u"2"_s}};
        QTest::newRow("CSV empty cells") << u"CSV"_s << QByteArray("a,,c\n,b,\n") <<
            Cells{{u"a"_s, u""_s, u"c"_s}, {u""_s, u"b"_s, u""_s}};
        QTest::newRow("CSV ragged") << u"CSV"_s << QByteArray("a\n1,2,3\n") <<
            Cells{{u"a"_s, u""_s, u""_s}, {u"1"_s, u"2"_s, u"3"_s}};
        QTest::newRow("CSV quoted") << u"CSV"_s << QByteArray("\"a,b\",c\n\"\",\"d\"\n") <<
            Cells{{u"a,b"_s, u"c"_s}, {u""_s, u"d"_s}};
        QTest::newRow("CSV escaped quotes") << u"CSV"_s << QByteArray("\"say \"\"hi\"\"\",\"\"\"\"\n") <<
            Cells{{u"say \"hi\""_s, u"\""_s}};
        // Values are trimmed, even when quoted
        QTest::newRow("CSV whitespace") << u"CSV"_s << QByteArray("\" a \", b c \n") <<
            Cells{{u"a"_s, u"b c"_s}};
        QTest::newRow("CSV UTF-8") << u"CSV"_s << QByteArray("caf\xc3\xa9,\xe2\x86\x92\n") <<
            Cells{{u"café"_s, u"→"_s}};
        // QString drops the byte order mark at the start of the first value
        QTest::newRow("CSV BOM") << u"CSV"_s << QByteArray("\xef\xbb\xbf" "a,b\n") <<
            Cells{{u"a"_s, u"b"_s}};
        QTest::newRow("TSV") << u"TSV"_s << QByteArray("a\tb c\t\n1\t\t3\n") <<
            Cells{{u"a"_s, u"b c"_s, u""_s}, {u"1"_s, u""_s, u"3"_s}};
        QTest::newRow("SSV") << u"SSV"_s << QByteArray("a;b,c\n") <<
            Cells{{u"a"_s, u"b,c"_s}};

        // Runs of whitespace are a single delimiter
        QTest::newRow("TXT") << u"TXT"_s << QByteArray("a  b\t\tc\n 1 2 3\n") <<
            Cells{{u"a"_s, u"b"_s, u"c"_s}, {u"1"_s, u"2"_s, u"3"_s}};
    }

    void parse()
    {
        QFETCH(const QString, type);
        QFETCH(const QByteArray, content);
        QFETCH(const Cells, expected);

        auto parser = createParser(type);
        QVERIFY(parser->parse(writeFile(content)));
        QCOMPARE(cellsOf(parser->tabularData()), expected);
    }

    void invalidUtf8()
    {
        auto parser = createParser(u"CSV"_s);
        QVERIFY(parser->parse(writeFile(QByteArray("a\xff,b\n"))));

        auto cells = cellsOf(parser->tabularData());
        QCOMPARE(cells.size(), 1);
        QCOMPARE(cells.at(0).size(), 2);
        QCOMPARE(cells.at(0).at(1), u"b"_s);
    }

    void emptyFile()
    {
        auto parser = createParser(u"CSV"_s);
        QVERIFY(!parser->parse(writeFile({})));
        QVERIFY(!parser->failureReason().isEmpty());
    }

    void missingFile()
    {
        auto parser = createParser(u"CSV"_s);
        QVERIFY(!parser->parse(QUrl::fromLocalFile(_dir.filePath(u"missing"_s))));
    }

    void canLoad_data()
    {
        QTest::addColumn<QString>("type");
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<bool>("expected");

        QTest::newRow("CSV") << u"CSV"_s << QByteArray("a,b\n1,2\n") << true;
        QTest::newRow("CSV single column") << u"CSV"_s << QByteArray("a\n1\n") << false;
        QTest::newRow("CSV as TSV") << u"TSV"_s << QByteArray("a,b\n1,2\n") << false;
        QTest::newRow("TSV") << u"TSV"_s << QByteArray("a\tb\n1\t2\n") << true;
        QTest::newRow("CSV ragged") << u"CSV"_s << QByteArray("a,b\n1,2,3,4,5,6,7\n") << false;
    }

    void canLoad()
    {
        QFETCH(const QString, type);
        QFETCH(const QByteArray, content);
        QFETCH(const bool, expected);

        auto url = writeFile(content);

        if(type == u"CSV"_s)
            QCOMPARE(CsvFileParser::canLoad(url), expected);
        else if(type == u"TSV"_s)
            QCOMPARE(TsvFileParser::canLoad(url), expected);
        else
            QFAIL("Unknown type");
    }

    void xlsx_data()
    {
        QTest::addColumn<Cells>("cells");
        QTest::addColumn<Cells>("expected");

        const Cells table{{u"Name"_s, u"Value"_s, u"Notes"_s},
            {u"a"_s, u"1"_s, u"first"_s}, {u"b"_s, u"2.5"_s, u"second"_s}};
        QTest::newRow("Table") << table << table;

        QTest::newRow("Gaps") <<
            Cells{{u"a"_s, u""_s, u"c"_s}, {u""_s, u"2"_s, u""_s}, {u""_s, u""_s, u""_s}, {u"d"_s}} <<
            Cells{{u"a"_s, u""_s, u"c"_s}, {u""_s, u"2"_s, u""_s}, {u""_s, u""_s, u""_s}, {u"d"_s, u""_s, u""_s}};

        QTest::newRow("Shared strings") <<
            Cells{{u"x"_s, u"x"_s}, {u"y"_s, u"x"_s}} << Cells{{u"x"_s, u"x"_s}, {u"y"_s, u"x"_s}};

        QTest::newRow("Escaping and Unicode") <<
            Cells{{u"a & b"_s, u"<c>"_s, u"\"d\""_s, u"café →"_s}} <<
            Cells{{u"a & b"_s, u"<c>"_s, u"\"d\""_s, u"café →"_s}};

        QTest::newRow("Numbers") <<
            Cells{{u"-1"_s, u"0.1"_s, u"1e-07"_s, u"123456789012"_s}} <<
            Cells{{u"-1"_s, u"0.1"_s, u"1e-07"_s, u"123456789012"_s}};

        QTest::newRow("Wide") <<
            Cells{QStringList(30, u"v"_s)} << Cells{QStringList(30, u"v"_s)};

        QTest::newRow("Empty") << Cells{} << Cells{};
    }

    void xlsx()
    {
        QFETCH(const Cells, cells);
        QFETCH(const Cells, expected);

        const auto fileName = _dir.filePath(u"data.xlsx"_s);
        QVERIFY(XlsxWriter::write(fileName, cells));
        const auto url = QUrl::fromLocalFile(fileName);

        QVERIFY(XlsxTabularDataParser::canLoad(url));
        QCOMPARE(TabularData::contentIdentityOf(url), u"XLSX"_s);

        XlsxTabularDataParser parser;
        QVERIFY(parser.parse(url));
        QCOMPARE(cellsOf(parser.tabularData()), expected);
    }

    void xlsxNotXlsx()
    {
        // A CSV file with an XLSX extension
        const auto url = writeFile("a,b\n1,2\n", u"data.xlsx"_s);
        QVERIFY(!XlsxTabularDataParser::canLoad(url));

        XlsxTabularDataParser parser;
        QVERIFY(!parser.parse(url));
        QVERIFY(!parser.failureReason().isEmpty());

        QVERIFY(!XlsxTabularDataParser::canLoad(QUrl::fromLocalFile(_dir.filePath(u"missing.xlsx"_s))));
    }

    void rowLimit_data()
    {
        QTest::addColumn<QString>("type");

        QTest::newRow("CSV") << u"CSV"_s;
        QTest::newRow("XLSX") << u"XLSX"_s;
    }

    void rowLimit()
    {
        QFETCH(const QString, type);

        Cells cells;
        cells.reserve(10);
        for(int row = 0; row < 10; row++)
            cells.append({u"r%1"_s.arg(row), QString::number(row)});

        TabularData tabularData;

        if(type == u"XLSX"_s)
        {
            const auto fileName = _dir.filePath(u"rows.xlsx"_s);
            QVERIFY(XlsxWriter::write(fileName, cells));

            XlsxTabularDataParser parser;
            parser.setRowLimit(3);
            QVERIFY(parser.parse(QUrl::fromLocalFile(fileName)));
            tabularData = std::move(parser.tabularData());
        }
        else
        {
            QByteArray csv;
            for(const auto& row : cells)
                csv += row.join(u',').toUtf8() + "\n";

            CsvFileParser parser;
            parser.setRowLimit(3);
            QVERIFY(parser.parse(writeFile(csv)));
            tabularData = std::move(parser.tabularData());
        }

        // Both read one more row than the limit, which is only used to look at
        // the start of a file, so it doesn't matter
        QCOMPARE(tabularData.numRows(), 4u);
        QCOMPARE(tabularData.valueAt(0, 3), u"r3"_s);
    }

    // The data rectangle is what the correlation plugin takes to be the data, by default
    void dataRect_data()
    {
        QTest::addColumn<QString>("csv");
        QTest::addColumn<QRect>("numerical");
        QTest::addColumn<QRect>("nonNumerical");

        QTest::newRow("Typical") << u"Name,c1,c2,c3\na,1,2,3\nb,4,5,6"_s <<
            QRect(1, 1, 3, 2) << QRect(0, 1, 1, 2);
        QTest::newRow("Annotation column") << u"Name,Type,c1,c2\na,x,1,2\nb,y,3,4"_s <<
            QRect(2, 1, 2, 2) << QRect();
        QTest::newRow("Missing values") << u"Name,c1,c2\na,1,\nb,,4\nc,5,6"_s <<
            QRect(1, 1, 2, 3) << QRect(0, 1, 1, 3);
        QTest::newRow("Scientific notation") << u"Name,c1,c2\na,1e-3,-2.5E+2\nb,0.5,7"_s <<
            QRect(1, 1, 2, 2) << QRect(0, 1, 1, 2);

        // Rows above a non-numerical value are left out, if that gives the larger area
        QTest::newRow("Non-numerical value") << u"Name,c1,c2,c3\na,1,x,3\nb,4,5,6\nc,7,8,9"_s <<
            QRect(1, 2, 3, 2) << QRect();

        // The first row and column are taken to be names, even if they're numbers
        QTest::newRow("All numerical") << u"1,2,3\n4,5,6\n7,8,9"_s <<
            QRect(1, 1, 2, 2) << QRect();

        // The data must be at the bottom right, so nothing is found here
        QTest::newRow("Trailing annotation column") << u"Name,c1,c2,Notes\na,1,2,x\nb,3,4,y"_s <<
            QRect() << QRect();

        QTest::newRow("Discrete") << u"Name,s1,s2\na,x,y\nb,z,w"_s <<
            QRect() << QRect(1, 1, 2, 2);
        QTest::newRow("One row") << u"a,1,2"_s << QRect(1, 0, 2, 1) << QRect(0, 0, 1, 1);
        QTest::newRow("One value") << u"1"_s << QRect(0, 0, 1, 1) << QRect();
    }

    void dataRect()
    {
        QFETCH(const QString, csv);
        QFETCH(const QRect, numerical);
        QFETCH(const QRect, nonNumerical);

        const auto tabularData = tabularDataOf(cellsFrom(csv));

        QCOMPARE(tabularData.findLargestNumericalDataRect(), numerical);

        // Only used when there's no numerical data
        if(numerical.isEmpty())
            QCOMPARE(tabularData.findLargestNonNumericalDataRect(), nonNumerical);
    }

    void typeIdentities()
    {
        const auto tabularData = tabularDataOf(cellsFrom(
            u"Name,Int,Float,String,Empty,Mixed\n"
            "a,1,1.5,x,,1\n"
            "b,2,2,y,,z\n"
            "c,-3,1e3,1,,2.5"_s));

        using Type = TypeIdentity::Type;

        // The first row is skipped, as a header
        const auto columnTypeIdentities = tabularData.columnTypeIdentities();
        std::vector<Type> columnTypes;
        columnTypes.reserve(columnTypeIdentities.size());
        for(const auto& identity : columnTypeIdentities)
            columnTypes.push_back(identity.type());

        QCOMPARE(columnTypes, (std::vector<Type>{Type::String, Type::Int, Type::Float,
            Type::String, Type::Unknown, Type::String}));

        QCOMPARE(tabularData.columnTypeIdentity(5, 3).type(), Type::Float);
        QCOMPARE(tabularData.columnTypeIdentity(1, 0).type(), Type::String);

        // The first column is skipped, as names
        const auto rowTypeIdentities = tabularData.rowTypeIdentities();
        std::vector<Type> rowTypes;
        rowTypes.reserve(rowTypeIdentities.size());
        for(const auto& identity : rowTypeIdentities)
            rowTypes.push_back(identity.type());

        QCOMPARE(rowTypes, (std::vector<Type>{Type::String, Type::String, Type::String, Type::Float}));
        QCOMPARE(tabularData.rowTypeIdentity(1, 4).type(), Type::Int);
        QCOMPARE(tabularData.rowTypeIdentity(2, 4).type(), Type::String);
        QCOMPARE(tabularData.rowTypeIdentity(3, 0).type(), Type::String);
    }

    void duplicates()
    {
        const auto tabularData = tabularDataOf(cellsFrom(
            u"Name,Unique,Repeated,Empty\n"
            "a,1,x,\n"
            "b,2,x,\n"
            "a,3,y,"_s));

        // Empty values aren't duplicates
        QCOMPARE(tabularData.columnDuplicates(), (std::vector<bool>{true, false, true, false}));
        QVERIFY(!tabularData.columnHasDuplicates(0, 2));

        QCOMPARE(tabularData.rowDuplicates(), (std::vector<bool>{false, false, false, false}));
        QVERIFY(!tabularData.rowHasDuplicates(1, 0));

        const auto rows = tabularDataOf(cellsFrom(u"Name,c1,c2\na,1,1\nb,1,2"_s));
        QCOMPARE(rows.rowDuplicates(), (std::vector<bool>{false, true, false}));
    }

    // How many of the values are in a column, or row, which is how the key for importing
    // attributes or annotations is found
    void matchPercentage()
    {
        const auto tabularData = tabularDataOf(cellsFrom(
            u"Key,Value\n"
            "a,1\n"
            "b,2\n"
            "c,3"_s));

        // The header isn't matched
        QCOMPARE(tabularData.columnMatchPercentage(0, {u"a"_s, u"b"_s, u"c"_s}), 100);
        QCOMPARE(tabularData.columnMatchPercentage(0, {u"a"_s, u"b"_s, u"c"_s, u"d"_s}), 75);
        QCOMPARE(tabularData.columnMatchPercentage(0, {u"Key"_s}), 0);
        QCOMPARE(tabularData.columnMatchPercentage(1, {u"a"_s}), 0);

        // Any match at all is at least 1%
        QStringList many;
        many.reserve(1000);
        for(int i = 0; i < 1000; i++)
            many.append(u"v%1"_s.arg(i));

        many.append(u"a"_s);
        QCOMPARE(tabularData.columnMatchPercentage(0, many), 1);

        const auto rows = tabularDataOf(cellsFrom(u"Name,s1,s2,s3\nType,x,y,z"_s));
        QCOMPARE(rows.rowMatchPercentage(0, {u"s1"_s, u"s2"_s, u"s3"_s}), 100);
        QCOMPARE(rows.rowMatchPercentage(0, {u"s1"_s, u"s4"_s}), 50);
        QCOMPARE(rows.rowMatchPercentage(1, {u"s1"_s}), 0);

        // The values are those of each node or edge, so values are often repeated, e.g. a
        // node attribute with values a, a, a and b is wholly matched by the column
        QCOMPARE(tabularData.columnMatchPercentage(0, {u"a"_s, u"a"_s, u"a"_s, u"b"_s}), 100);
        QCOMPARE(tabularData.columnMatchPercentage(0, {u"a"_s, u"a"_s, u"d"_s, u"d"_s}), 50);
        QCOMPARE(rows.rowMatchPercentage(0, {u"s1"_s, u"s1"_s, u"s2"_s}), 100);

        // Missing values aren't matched, even by empty cells
        const auto withEmpty = tabularDataOf(cellsFrom(u"Key\na\n\nc"_s));
        QCOMPARE(withEmpty.columnMatchPercentage(0, {u""_s, u""_s, u"a"_s}), 33);
        QCOMPARE(withEmpty.columnMatchPercentage(0, {u""_s}), 0);

        QCOMPARE(tabularData.columnMatchPercentage(0, {}), 0);
        QCOMPARE(rows.rowMatchPercentage(0, {}), 0);
    }

    void contentIdentity_data()
    {
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<QString>("expected");

        QTest::newRow("CSV") << QByteArray("a,b,c\n1,2,3\n") << u"CSV"_s;
        QTest::newRow("TSV") << QByteArray("a\tb\tc\n1\t2\t3\n") << u"TSV"_s;
        QTest::newRow("SSV") << QByteArray("a;b;c\n1;2;3\n") << u"SSV"_s;
        QTest::newRow("Quoted commas") << QByteArray("\"a,b\";\"c,d\"\n1;2\n") << u"SSV"_s;
        QTest::newRow("Nothing") << QByteArray("abc\n") << QString();
    }

    void contentIdentity()
    {
        QFETCH(const QByteArray, content);
        QFETCH(const QString, expected);

        QCOMPARE(TabularData::contentIdentityOf(writeFile(content)), expected);
    }
};

QTEST_GUILESS_MAIN(TestTabularData)
#include "tst_tabulardata.moc"
