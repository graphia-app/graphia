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

#include <QTest>
#include <QByteArray>
#include <QFile>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>

#include <memory>

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
