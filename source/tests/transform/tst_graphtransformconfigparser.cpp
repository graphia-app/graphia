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

#include "app/transform/graphtransformconfig.h"
#include "app/transform/graphtransformconfigparser.h"

#include <QTest>
#include <QString>

#include <variant>

using namespace Qt::Literals::StringLiterals;

class TestGraphTransformConfigParser : public QObject
{
    Q_OBJECT

private slots:
    void roundTrip_data()
    {
        QTest::addColumn<QString>("config");

        QTest::newRow("flag and terminal condition") <<
            uR"([pinned] "Remove Components" where $"Component Size" <= 1)"_s;
        QTest::newRow("attribute and int parameter") <<
            uR"("k-NN" using $"Pearson Correlation Value" with "k" = 5)"_s;
        QTest::newRow("float parameter") <<
            uR"("MCL Cluster" with "Granularity" = 2.2)"_s;
        QTest::newRow("string parameter") <<
            uR"("Louvain Cluster" with "Weighted" = "No")"_s;
        QTest::newRow("compound condition") <<
            uR"("Remove Nodes" where ($"Node Degree" > 3 and $"Node Name" includes "abc") or $"Weight" hasValue)"_s;
        QTest::newRow("all string ops") <<
            uR"("Remove Nodes" where ($"A" starts "x" or $"A" ends "y") and ($"A" matches "^z" || $"A" matchesCaseInsensitive "w"))"_s;
        // Not a raw string, which moc misparses here, missing the test class
        QTest::newRow("quotes in attribute name") <<
            u"RemoveNodes where $\"Score \\\"raw\\\"\" > 1"_s; // NOLINT modernize-raw-string-literal
    }

    void roundTrip()
    {
        QFETCH(const QString, config);

        GraphTransformConfigParser parser;
        QVERIFY2(parser.parse(config), qPrintable(parser.failedInput()));

        const auto& original = parser.result();
        auto serialised = original.asString();

        GraphTransformConfigParser reparser;
        QVERIFY2(reparser.parse(serialised), qPrintable(serialised));
        QVERIFY2(reparser.result().equals(original, "ignoreInertFlags"_no), qPrintable(serialised));
        QCOMPARE(reparser.result().asString(), serialised);
    }

    void parsesFields()
    {
        GraphTransformConfigParser parser;
        QVERIFY(parser.parse(uR"([pinned] "Remove Components" where $"Component Size" <= 1)"_s));

        const auto& config = parser.result();
        QCOMPARE(config._flags.size(), 1);
        QCOMPARE(config._flags.front(), u"pinned"_s);
        QCOMPARE(config._action, u"Remove Components"_s);
        QVERIFY(config.isFlagSet(u"pinned"_s));
        QVERIFY(config.hasCondition());

        const auto* condition = std::get_if<GraphTransformConfig::TerminalCondition>(&config._condition);
        QVERIFY(condition != nullptr);
        QCOMPARE(std::get<GraphTransformConfig::AttributeReference>(condition->_lhs)._name, u"Component Size"_s);
        QVERIFY(std::get<ConditionFnOp::Numerical>(condition->_op) == ConditionFnOp::Numerical::LessThanOrEqual);
        QCOMPARE(std::get<int>(condition->_rhs), 1);
    }

    void parsesParameters()
    {
        GraphTransformConfigParser parser;
        QVERIFY(parser.parse(uR"("k-NN" using $"Correlation" with "k" = 5 "Rank Order" = "Ascending")"_s));

        const auto& config = parser.result();
        QCOMPARE(config._action, u"k-NN"_s);
        QCOMPARE(config._attributes.size(), 1);
        QVERIFY(!config.hasCondition());

        const auto* k = config.parameterByName(u"k"_s);
        QVERIFY(k != nullptr);
        QCOMPARE(std::get<int>(k->_value), 5);
        QVERIFY(config.parameterHasValue(u"Rank Order"_s, u"Ascending"_s));
        QVERIFY(!config.hasParameter(u"Missing"_s));
    }

    // Names, and string values, are only quoted when they aren't identifiers
    void writtenWithoutUnnecessaryQuotes()
    {
        GraphTransformConfigParser parser;
        QVERIFY(parser.parse(uR"("KNN" using $"Node Weight" with "RankOrder" = "Ascending" where $"Kind" == "a b" or $"Kind" == "c")"_s));
        QCOMPARE(parser.result().asString(),
            uR"(KNN using $"Node Weight" with RankOrder = Ascending where $Kind == "a b" or $Kind == c)"_s);
    }

    void opDisplayText_data()
    {
        QTest::addColumn<QString>("op");
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("phrase");

        QTest::newRow("==") << u"=="_s << u"="_s << u"%1 = %2"_s;
        QTest::newRow("!=") << u"!="_s << u"≠"_s << u"%1 ≠ %2"_s;
        QTest::newRow("<=") << u"<="_s << u"≤"_s << u"%1 ≤ %2"_s;
        QTest::newRow("&&") << u"&&"_s << u"and"_s << u"%1 and %2"_s;
        QTest::newRow("or") << u"or"_s << u"or"_s << u"%1 or %2"_s;
        QTest::newRow("matchesCaseInsensitive") << u"matchesCaseInsensitive"_s <<
            u"Matches Case Insensitive Regex"_s << u"%1 Matches Case Insensitive Regex %2"_s;
        QTest::newRow("hasValue") << u"hasValue"_s << u"Has Value"_s << u"%1 Has Value"_s;
        QTest::newRow("unknown") << u"foo"_s << u"foo"_s << u"%1 foo %2"_s;
    }

    void opDisplayText()
    {
        QFETCH(const QString, op);
        QFETCH(const QString, name);
        QFETCH(const QString, phrase);

        const auto text = GraphTransformConfigParser::opDisplayText(op);
        QCOMPARE(text._name, name);
        QCOMPARE(text._phrase, phrase);
    }

    void rejectsMalformed_data()
    {
        QTest::addColumn<QString>("config");

        QTest::newRow("empty") << QString();
        QTest::newRow("missing condition") << uR"("Remove Edges" where)"_s;
        QTest::newRow("missing action") << uR"(where $"x" < 1)"_s;
        // Not a raw string literal, as moc fails to parse one containing an unbalanced quote
        QTest::newRow("unterminated string") << u"\"Remove Edges"_s;
        QTest::newRow("missing parameter value") << uR"("k-NN" with "k" =)"_s;
        QTest::newRow("unknown operator") << uR"("Remove Edges" where $"x" ~ 1)"_s;
        QTest::newRow("unbalanced parentheses") << uR"("Remove Edges" where ($"x" < 1)"_s;
    }

    void rejectsMalformed()
    {
        QFETCH(const QString, config);

        GraphTransformConfigParser parser;
        QVERIFY(!parser.parse(config, false));
        QVERIFY(!parser.success());
    }
};

QTEST_GUILESS_MAIN(TestGraphTransformConfigParser)
#include "tst_graphtransformconfigparser.moc"
