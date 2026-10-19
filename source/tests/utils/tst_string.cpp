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

#include "shared/utils/string.h"

#include <QTest>
#include <QLocale>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// Numbers are formatted in the system locale, so expected values, written as in the C
// locale with "," separating groups, are converted to it
QString localised(const QString& string)
{
    const auto locale = QLocale::system();
    QString result;

    for(auto c : string)
    {
        if(c == u'.')
            result += locale.decimalPoint();
        else if(c == u',')
        {
            if(!locale.numberOptions().testFlag(QLocale::OmitGroupSeparator))
                result += locale.groupSeparator();
        }
        else if(c == u'-')
            result += locale.negativeSign();
        else
            result += c;
    }

    return result;
}
} // namespace

class TestString : public QObject
{
    Q_OBJECT

private slots:
    void isNumeric_data()
    {
        QTest::addColumn<QString>("string");
        QTest::addColumn<bool>("numeric");
        QTest::addColumn<bool>("integer");

        QTest::newRow("int") << u"42"_s << true << true;
        QTest::newRow("negative") << u"-42"_s << true << true;
        QTest::newRow("float") << u"1.5"_s << true << false;
        QTest::newRow("exponent") << u"1e-3"_s << true << false;
        QTest::newRow("empty") << QString() << false << false;
        QTest::newRow("text") << u"abc"_s << false << false;
    }

    void isNumeric()
    {
        QFETCH(const QString, string);
        QFETCH(const bool, numeric);
        QFETCH(const bool, integer);

        QCOMPARE(u::isNumeric(string), numeric);
        QCOMPARE(u::isNumeric(string.toStdString()), numeric);
        QCOMPARE(u::isInteger(string), integer);
        QCOMPARE(u::isInteger(string.toStdString()), integer);

        QCOMPARE(std::isnan(u::toNumber(string)), !numeric);
        QCOMPARE(std::isnan(u::toNumber(string.toStdString())), !numeric);

        if(numeric)
        {
            QCOMPARE(u::toNumber(string), string.toDouble());
            QCOMPARE(u::toNumber(string.toStdString()), string.toDouble());
        }
    }

    void getline()
    {
        std::istringstream stream("a\nb\r\nc\rd");
        std::vector<std::string> lines;
        std::string line;

        // Unlike std::getline, the end is signalled by eof() alone
        while(!u::getline(stream, line).eof())
            lines.push_back(line);

        QCOMPARE(lines, (std::vector<std::string>{"a", "b", "c", "d"}));
    }

    void formatNumber_data()
    {
        QTest::addColumn<double>("value");
        QTest::addColumn<QString>("expected");

        QTest::newRow("zero") << 0.0 << u"0"_s;
        QTest::newRow("int") << 42.0 << u"42"_s;
        QTest::newRow("small") << 0.000123 << u"0.00012"_s;
        QTest::newRow("trailing zeroes") << 0.5 << u"0.5"_s;
        QTest::newRow("hundreds") << 123.456 << u"123.5"_s;
        QTest::newRow("thousands") << 1234.56 << u"1,235"_s;
        QTest::newRow("negative") << -1.25 << u"-1.25"_s;
    }

    void formatNumber()
    {
        QFETCH(const double, value);
        QFETCH(const QString, expected);

        QCOMPARE(u::formatNumber(value), localised(expected));
    }

    void formatNumberScientific_data()
    {
        QTest::addColumn<double>("value");
        QTest::addColumn<QString>("expected");

        QTest::newRow("zero") << 0.0 << u"0"_s;
        QTest::newRow("int") << 42.0 << u"42"_s;
        QTest::newRow("fraction") << 0.25 << u"0.25"_s;
    }

    void formatNumberScientific()
    {
        QFETCH(const double, value);
        QFETCH(const QString, expected);

        QCOMPARE(u::formatNumberScientific(value), localised(expected));
    }

    void formatNumberSIPostfix()
    {
        QCOMPARE(u::formatNumberSIPostfix(999.0), u"999"_s);
        QCOMPARE(u::formatNumberSIPostfix(12345.0), u"12.3k"_s);
        QCOMPARE(u::formatNumberSIPostfix(2.5e6), u"2.5M"_s);
        QCOMPARE(u::formatNumberSIPostfix(3e9), u"3.0B"_s);
    }

    void hex()
    {
        QVERIFY(u::isHex(u"00ff7A"_s));
        QVERIFY(!u::isHex(u"0ff"_s));
        QVERIFY(!u::isHex(u"0g"_s));
        QVERIFY(!u::isHex(QString()));

        QCOMPARE(u::hexToString(u"4869"_s), u"Hi"_s);
        QCOMPARE(u::hexToString(u"zz"_s), QString());
        QCOMPARE(u::hexToBytes(u"00ff"_s), (std::vector<std::byte>{std::byte{0x00}, std::byte{0xff}}));
        QCOMPARE(u::bytesToHex(std::string{"\x00\x7f\xff", 3}), std::string{"007fff"});
    }

    void findUniqueName_data()
    {
        QTest::addColumn<QStringList>("existing");
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("expected");

        QTest::newRow("unused") << QStringList{u"a"_s} << u"b"_s << u"b"_s;
        QTest::newRow("used") << QStringList{u"a"_s} << u"a"_s << u"a(1)"_s;
        QTest::newRow("used twice") << QStringList{u"a"_s, u"a(1)"_s} << u"a"_s << u"a(2)"_s;
        QTest::newRow("numbered") << QStringList{u"a(1)"_s} << u"a(1)"_s << u"a(2)"_s;
        QTest::newRow("empty") << QStringList{QString()} << QString() << QString();
        QTest::newRow("percent and digit") << QStringList{u"Rank %1"_s} << u"Rank %1"_s << u"Rank %1(1)"_s;
    }

    void findUniqueName()
    {
        QFETCH(const QStringList, existing);
        QFETCH(const QString, name);
        QFETCH(const QString, expected);

        QCOMPARE(u::findUniqueName(existing, name), expected);
    }

    void escapeQuotes()
    {
        QCOMPARE(u::escapeQuotes(u"say \"hi\""_s), u"say \\\"hi\\\""_s);
    }

    void numericCompare_data()
    {
        QTest::addColumn<QString>("a");
        QTest::addColumn<QString>("b");
        QTest::addColumn<int>("expected");

        QTest::newRow("equal") << u"a1"_s << u"a1"_s << 0;
        QTest::newRow("text") << u"a"_s << u"b"_s << -1;
        QTest::newRow("numbers") << u"2"_s << u"10"_s << -1;
        QTest::newRow("numbers in text") << u"Thing 2"_s << u"Thing 10"_s << -1;
        QTest::newRow("several numbers") << u"v1.10"_s << u"v1.9"_s << 1;
        QTest::newRow("prefix") << u"a"_s << u"a1"_s << -1;
        QTest::newRow("number before text") << u"1"_s << u"a"_s << -1;
        QTest::newRow("leading zeroes") << u"a01"_s << u"a1"_s << -1;
        QTest::newRow("empty") << QString() << u"a"_s << -1;
        QTest::newRow("both empty") << QString() << QString() << 0;
        QTest::newRow("empty and number") << u""_s << u"1"_s << -1;
        QTest::newRow("large numbers") << u"x9999999999"_s << u"x10000000000"_s << -1;
        QTest::newRow("large numbers, then text") << u"x3000000000b"_s << u"x4000000000a"_s << -1;
        QTest::newRow("large numbers with leading zeroes") << u"x0010000000000"_s << u"x9999999999"_s << 1;
    }

    void numericCompare()
    {
        QFETCH(const QString, a);
        QFETCH(const QString, b);
        QFETCH(const int, expected);

        auto sign = [](int v) { return (v > 0) - (v < 0); };

        QCOMPARE(sign(u::numericCompare(a, b)), expected);
        QCOMPARE(sign(u::numericCompare(b, a)), -expected);
    }

    void numericCompareCaseInsensitive()
    {
        QCOMPARE(u::numericCompareCaseInsensitive(u"Thing 2"_s, u"thing 2"_s), 0);
        QVERIFY(u::numericCompareCaseInsensitive(u"thing 2"_s, u"Thing 10"_s) < 0);
        QVERIFY(u::numericCompare(u"Thing 2"_s, u"thing 2"_s) != 0);
    }

    void numericSort()
    {
        QStringList values{u"x10"_s, u"x9"_s, u"x1"_s, u"y"_s, u"x"_s};
        std::ranges::sort(values, [](const auto& a, const auto& b) { return u::numericCompare(a, b) < 0; });
        QCOMPARE(values, (QStringList{u"x"_s, u"x1"_s, u"x9"_s, u"x10"_s, u"y"_s}));
    }
};

QTEST_GUILESS_MAIN(TestString)
#include "tst_string.moc"
