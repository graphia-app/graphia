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

#include "app/ui/visualisations/colorvisualisationchannel.h"
#include "app/ui/visualisations/textcolorvisualisationchannel.h"
#include "app/ui/visualisations/visualisationconfig.h"
#include "app/ui/visualisations/visualisationconfigparser.h"

#include <QTest>
#include <QString>

#include <memory>

using namespace Qt::Literals::StringLiterals;

class TestVisualisationConfigParser : public QObject
{
    Q_OBJECT

private slots:
    void roundTrip_data()
    {
        QTest::addColumn<QString>("config");

        QTest::newRow("Minimal") << u"\"Node Degree\" \"Size\""_s;
        QTest::newRow("Flags") << u"[component, invert] \"Node Degree\" \"Colour\""_s;
        QTest::newRow("Number parameter") << u"\"Node Degree\" \"Size\" with min = 2.5"_s;
        QTest::newRow("Negative exponent") << u"\"Node Degree\" \"Size\" with min = -1e-7"_s;
        QTest::newRow("String parameter") << u"\"Node Name\" \"Text\" with size = \"large\""_s;
        QTest::newRow("JSON parameter") <<
            u"\"Node Name\" \"Colour\" with palette = \"{\\\"colors\\\":[\\\"#FF0000\\\"]}\""_s;
        QTest::newRow("Integer parameter") << u"\"Node Degree\" \"Size\" with min = 2"_s;
        QTest::newRow("Several parameters") << u"\"Node Degree\" \"Colour\" with a = 1 b = \"x\" c = 3"_s;
        QTest::newRow("Attribute parameter") << u"\"Pearson Value\".\"x\" \"Size\""_s;
        QTest::newRow("Non-ASCII") << u"\"Größe\" \"Size\" with label = \"→\""_s;
        QTest::newRow("Unquoted names") << u"NodeDegree Size with min = 2.5"_s;
        QTest::newRow("Unquoted string") << u"NodeName Text with size = large"_s;
    }

    void roundTrip()
    {
        QFETCH(const QString, config);

        VisualisationConfigParser parser;
        QVERIFY2(parser.parse(config), qPrintable(parser.failedInput()));
        const auto& original = parser.result();

        // The string form must parse to the same configuration, and stay the same
        const auto string = original.asString();
        VisualisationConfigParser reparser;
        QVERIFY2(reparser.parse(string), qPrintable(string));
        QVERIFY2(reparser.result().equals(original), qPrintable(string));
        QCOMPARE(reparser.result().asString(), string);
    }

    void invalid_data()
    {
        QTest::addColumn<QString>("config");

        QTest::newRow("Empty") << QString();
        QTest::newRow("No channel") << u"\"Node Degree\""_s;
        QTest::newRow("Unterminated") << u"\"Node Degree\" \"Size"_s;
        QTest::newRow("Unclosed flags") << u"[component \"Node Degree\" \"Size\""_s;
        QTest::newRow("No parameters") << u"\"Node Degree\" \"Size\" with"_s;
        QTest::newRow("No parameter value") << u"\"Node Degree\" \"Size\" with min ="_s;
        QTest::newRow("Unquoted string with spaces") << u"\"Node Degree\" \"Size\" with min = very large"_s;
        QTest::newRow("Trailing text") << u"\"Node Degree\" \"Size\" extra"_s;
    }

    void invalid()
    {
        QFETCH(const QString, config);

        VisualisationConfigParser parser;
        QVERIFY(!parser.parse(config, false));
        QVERIFY(!parser.success());
    }

    void parsed()
    {
        VisualisationConfigParser parser;
        QVERIFY(parser.parse(u"[component] \"Node Degree\" \"Colour\" with gradient = \"g\" min = 2.5"_s));

        const auto& config = parser.result();
        QCOMPARE(config._attributeName, u"Node Degree"_s);
        QCOMPARE(config._channelName, u"Colour"_s);
        QVERIFY(config.isFlagSet(u"component"_s));
        QVERIFY(!config.isFlagSet(u"invert"_s));
        QCOMPARE(config.parameterValue(u"gradient"_s), u"g"_s);
        QCOMPARE(config.parameterValue(u"min"_s), u"2.5"_s);
        QVERIFY(config.parameterValue(u"max"_s).isEmpty());
    }

    void display()
    {
        VisualisationChannelFactories factories;
        factories.emplace(u"Colour"_s, std::make_unique<ColorVisualisationChannelFactory>());
        factories.emplace(u"TextColour"_s, std::make_unique<TextColorVisualisationChannelFactory>());

        QCOMPARE(VisualisationConfigParser::parseForDisplay(
            u"[component, invert] NodeDegree Colour with min = 2.5"_s, factories),
            u"NodeDegree using Colour (Applied Per Component, Inverted)"_s);

        // The channel is shown by its display name, as is the attribute, given a function for it
        QCOMPARE(VisualisationConfigParser::parseForDisplay(u"NodeDegree TextColour"_s, factories,
            [](const QString& name) { return name == u"NodeDegree"_s ? u"Node Degree"_s : name; }),
            u"Node Degree using Text Colour"_s);

        // A channel that isn't known is shown by its name
        QCOMPARE(VisualisationConfigParser::parseForDisplay(u"\"Node Degree\" Size"_s, factories),
            u"Node Degree using Size"_s);
    }
};

QTEST_GUILESS_MAIN(TestVisualisationConfigParser)
#include "tst_visualisationconfigparser.moc"
