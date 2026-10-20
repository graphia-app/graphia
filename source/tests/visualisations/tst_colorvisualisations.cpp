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

#include "shared/ui/visualisations/colorgradient.h"
#include "shared/ui/visualisations/colorpalette.h"
#include "shared/utils/color.h"

#include <QTest>
#include <QColor>
#include <QString>

using namespace Qt::Literals::StringLiterals;

namespace
{
bool colorsClose(const QColor& a, const QColor& b)
{
    return std::abs(a.red() - b.red()) <= 1 &&
        std::abs(a.green() - b.green()) <= 1 &&
        std::abs(a.blue() - b.blue()) <= 1;
}
} // namespace

class TestColorVisualisations : public QObject
{
    Q_OBJECT

private slots:
    void gradient_data()
    {
        QTest::addColumn<double>("value");
        QTest::addColumn<QColor>("expected");

        QTest::newRow("below") << -1.0 << QColor(u"#ff0000"_s);
        QTest::newRow("first stop") << 0.0 << QColor(u"#ff0000"_s);
        QTest::newRow("between") << 0.25 << QColor(u"#808000"_s);
        QTest::newRow("middle stop") << 0.5 << QColor(u"#00ff00"_s);
        QTest::newRow("between again") << 0.75 << QColor(u"#008080"_s);
        QTest::newRow("last stop") << 1.0 << QColor(u"#0000ff"_s);
        QTest::newRow("above") << 2.0 << QColor(u"#0000ff"_s);
    }

    void gradient()
    {
        QFETCH(const double, value);
        QFETCH(const QColor, expected);

        // The stops are out of order, as JSON objects are unordered
        const ColorGradient gradient(u"{\"1\": \"#0000ff\", \"0\": \"#ff0000\", \"0.5\": \"#00ff00\"}"_s);

        auto actual = gradient.get(value);
        QVERIFY2(colorsClose(actual, expected), qPrintable(actual.name() + u" != "_s + expected.name()));
    }

    void gradientDegenerate()
    {
        QVERIFY(!ColorGradient().get(0.5).isValid());
        QVERIFY(!ColorGradient(u"[1, 2]"_s).get(0.5).isValid());
        QVERIFY(!ColorGradient(u"not json"_s).get(0.5).isValid());

        const ColorGradient single(u"{\"0.5\": \"#123456\"}"_s);
        QCOMPARE(single.get(0.0), QColor(u"#123456"_s));
        QCOMPARE(single.get(1.0), QColor(u"#123456"_s));
    }

    void paletteAutoColors()
    {
        const ColorPalette palette(u"{\"autoColors\": [\"#ff0000\", \"#00ff00\", \"#0000ff\"]}"_s);

        QCOMPARE(palette.get(u"x"_s, 0).name(), u"#ff0000"_s);
        QCOMPARE(palette.get(u"x"_s, 1).name(), u"#00ff00"_s);
        QCOMPARE(palette.get(u"x"_s, 2).name(), u"#0000ff"_s);

        // Beyond the colours given, they're varied, rather than repeated
        auto fourth = palette.get(u"x"_s, 3);
        QVERIFY(fourth.isValid());
        QVERIFY(fourth.name() != u"#ff0000"_s);
        QVERIFY(palette.get(u"x"_s, 4).name() != fourth.name());

        // Without an index, it's derived from the value, consistently
        QCOMPARE(palette.get(u"Thing 2"_s, -1), palette.get(u"Thing 2"_s, -1));
        QVERIFY(palette.get(u"Thing 2"_s, -1) != palette.get(u"Thing 3"_s, -1));
    }

    void paletteFixedColors()
    {
        const ColorPalette palette(u"{\"autoColors\": [\"#ff0000\"], "
            "\"fixedColors\": {\"a\": \"#00ff00\", \"b\": \"#0000ff\"}}"_s);

        QCOMPARE(palette.get(u"a"_s, 0).name(), u"#00ff00"_s);
        QCOMPARE(palette.get(u"b"_s, 0).name(), u"#0000ff"_s);
        QCOMPARE(palette.get(u"c"_s, 0).name(), u"#ff0000"_s);
    }

    void paletteDefaultColor()
    {
        const ColorPalette palette(u"{\"autoColors\": [\"#ff0000\", \"#00ff00\"], "
            "\"defaultColor\": \"#808080\"}"_s);

        QCOMPARE(palette.get(u"x"_s, 1).name(), u"#00ff00"_s);
        QCOMPARE(palette.get(u"x"_s, 2).name(), u"#808080"_s);
        QCOMPARE(palette.get(u"x"_s, 5).name(), u"#808080"_s);
    }

    void paletteWithoutAutoColors()
    {
        const ColorPalette fixedOnly(u"{\"fixedColors\": {\"a\": \"#00ff00\"}}"_s);
        QCOMPARE(fixedOnly.get(u"a"_s, 0).name(), u"#00ff00"_s);
        QCOMPARE(fixedOnly.get(u"b"_s, 0).name(), u::colorForString(u"b"_s).name());

        const ColorPalette withDefault(u"{\"fixedColors\": {}, \"defaultColor\": \"#808080\"}"_s);
        QCOMPARE(withDefault.get(u"b"_s, 0).name(), u"#808080"_s);

        QCOMPARE(ColorPalette(u"{}"_s).get(u"b"_s, 0).name(), u::colorForString(u"b"_s).name());
        QCOMPARE(ColorPalette().get(u"b"_s, 0).name(), u::colorForString(u"b"_s).name());
    }
};

QTEST_GUILESS_MAIN(TestColorVisualisations)
#include "tst_colorvisualisations.moc"
