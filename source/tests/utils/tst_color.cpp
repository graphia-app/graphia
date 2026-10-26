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

#include "shared/utils/color.h"

#include <QTest>
#include <QColor>
#include <QString>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

class TestColor : public QObject
{
    Q_OBJECT

private slots:
    void contrastingColor_data()
    {
        QTest::addColumn<QColor>("color");
        QTest::addColumn<QColor>("expected");

        QTest::newRow("black") << QColor(Qt::black) << QColor(Qt::white);
        QTest::newRow("white") << QColor(Qt::white) << QColor(Qt::black);
        QTest::newRow("yellow") << QColor(Qt::yellow) << QColor(Qt::black);
        QTest::newRow("dark blue") << QColor(Qt::darkBlue) << QColor(Qt::white);
        QTest::newRow("transparent") << QColor(Qt::transparent) << QColor(Qt::black);
    }

    void contrastingColor()
    {
        QFETCH(const QColor, color);
        QFETCH(const QColor, expected);

        QCOMPARE(u::contrastingColor(color), expected);
    }

    void colorForString()
    {
        auto a = u::colorForString(u"a"_s);
        auto b = u::colorForString(u"b"_s);

        QVERIFY(a.isValid());
        QCOMPARE(u::colorForString(u"a"_s), a);
        QVERIFY(a != b);

        for(const auto& string : {u""_s, u"a"_s, u"Some longer value"_s, u"ünïcode"_s})
        {
            auto color = u::colorForString(string);
            QCOMPARE(color.hslSaturation(), 255);
            QVERIFY(color.lightness() >= 64 && color.lightness() <= 190);
        }
    }

    void colorForStringValues_data()
    {
        QTest::addColumn<QString>("string");
        QTest::addColumn<int>("hue");
        QTest::addColumn<int>("lightness");

        // The same on every platform, whether char is signed or not
        QTest::newRow("a") << u"a"_s << 58 << 159;
        QTest::newRow("b") << u"b"_s << 185 << 122;
        QTest::newRow("longer") << u"Some longer value"_s << 10 << 110;
        QTest::newRow("unicode") << u"ünïcode"_s << 293 << 85;
    }

    void colorForStringValues()
    {
        QFETCH(const QString, string);
        QFETCH(const int, hue);
        QFETCH(const int, lightness);

        auto color = u::colorForString(string);
        QCOMPARE(color.hslHue(), hue);
        QCOMPARE(color.lightness(), lightness);
    }

    void colorForStringUsesEveryHue()
    {
        int maxHue = 0;
        for(int i = 0; i < 1000; i++)
            maxHue = std::max(maxHue, u::colorForString(u"s%1"_s.arg(i)).hslHue());

        // Hues run from 0 to 359, and blue-violet to pink is above 255
        QVERIFY(maxHue > 300);
    }
};

QTEST_GUILESS_MAIN(TestColor)
#include "tst_color.moc"
