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
#include "tests/common/testgraphmodel.h"
#include "tests/common/testplugin.h"

#include "app/attributes/conditionfncreator.h"
#include "app/graph/graphmodel.h"
#include "app/transform/graphtransformconfigparser.h"

#include "shared/graph/elementid.h"

#include <QTest>
#include <QString>

#include <array>
#include <cstddef>
#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr size_t NumNodes = 5;

const std::array<int, NumNodes>         intValues     = {0, 1, 2, 3, 4};
const std::array<int, NumNodes>         int2Values    = {4, 1, 0, 3, 5};
const std::array<double, NumNodes>      floatValues   = {-1.5, 0.0, 2.5, 3.0, 10.25};
const std::array<QString, NumNodes>     stringValues  = {u"apple"_s, u"Banana"_s, u"cherry"_s, u""_s, u"apple pie"_s};
const std::array<QString, NumNodes>     string2Values = {u"app"_s, u"an"_s, u"x"_s, u""_s, u"pie"_s};
const std::array<bool, NumNodes>        partialMissing = {false, true, false, true, false};

template<typename T>
auto valueFnFor(const std::array<T, NumNodes>& values)
{
    return [&values](NodeId nodeId) { return values.at(static_cast<size_t>(nodeId)); };
}
} // namespace

class TestConditionFnCreator : public QObject
{
    Q_OBJECT

private:
    TestPlugin _plugin;
    std::unique_ptr<GraphModel> _graphModel;

    // Returns which nodes the condition matches, one character per node, or "invalid"
    // if a condition function can't be created
    QString evaluate(const QString& condition) const
    {
        GraphTransformConfigParser parser;
        if(!parser.parse(uR"("Filter" where )"_s + condition, false))
            return u"unparseable"_s;

        auto conditionFn = CreateConditionFnFor::node(*_graphModel, parser.result()._condition);
        if(conditionFn == nullptr)
            return u"invalid"_s;

        QString result;
        for(size_t i = 0; i < NumNodes; i++)
            result += conditionFn(NodeId(static_cast<int>(i))) ? u'1' : u'0';

        return result;
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();

        _graphModel = std::make_unique<GraphModel>(TestGraphModel::application(), &_plugin);

        _graphModel->createAttribute(u"Int"_s).setIntValueFn(valueFnFor(intValues));
        _graphModel->createAttribute(u"Int2"_s).setIntValueFn(valueFnFor(int2Values));
        _graphModel->createAttribute(u"Float"_s).setFloatValueFn(valueFnFor(floatValues));
        _graphModel->createAttribute(u"String"_s).setStringValueFn(valueFnFor(stringValues));
        _graphModel->createAttribute(u"String2"_s).setStringValueFn(valueFnFor(string2Values));
        _graphModel->createAttribute(u"Partial"_s)
            .setIntValueFn(valueFnFor(intValues))
            .setValueMissingFn(valueFnFor(partialMissing));
        _graphModel->createAttribute(u"Edge Int"_s)
            .setIntValueFn([](EdgeId edgeId) { return static_cast<int>(edgeId); });
    }

    void cleanupTestCase()
    {
        _graphModel.reset();
    }

    void conditions_data()
    {
        QTest::addColumn<QString>("condition");
        QTest::addColumn<QString>("expected");

        // Equality
        QTest::newRow("int ==")                     << uR"($"Int" == 2)"_s                      << u"00100"_s;
        QTest::newRow("int !=")                     << uR"($"Int" != 2)"_s                      << u"11011"_s;
        QTest::newRow("float ==")                   << uR"($"Float" == 2.5)"_s                  << u"00100"_s;
        QTest::newRow("string ==")                  << uR"($"String" == "apple")"_s             << u"10000"_s;
        QTest::newRow("string == is case sensitive") << uR"($"String" == "banana")"_s           << u"00000"_s;
        QTest::newRow("string == empty")            << uR"($"String" == "")"_s                  << u"00010"_s;

        // Numerical, attribute and value of the same type
        QTest::newRow("int <")                      << uR"($"Int" < 2)"_s                       << u"11000"_s;
        QTest::newRow("int >")                      << uR"($"Int" > 2)"_s                       << u"00011"_s;
        QTest::newRow("int <=")                     << uR"($"Int" <= 2)"_s                      << u"11100"_s;
        QTest::newRow("int >=")                     << uR"($"Int" >= 2)"_s                      << u"00111"_s;
        QTest::newRow("float <")                    << uR"($"Float" < 2.5)"_s                   << u"11000"_s;
        QTest::newRow("float >=")                   << uR"($"Float" >= 2.5)"_s                  << u"00111"_s;

        // Numerical, attribute and value of different types
        QTest::newRow("int > float")                << uR"($"Int" > 1.5)"_s                     << u"00111"_s;
        QTest::newRow("int <= float")               << uR"($"Int" <= 1.5)"_s                    << u"11000"_s;
        QTest::newRow("float > int")                << uR"($"Float" > 2)"_s                     << u"00111"_s;

        QTest::newRow("numerical op on string")     << uR"($"String" < 2)"_s                    << u"invalid"_s;

        // String operators
        QTest::newRow("includes")                   << uR"($"String" includes "an")"_s          << u"01000"_s;
        QTest::newRow("excludes")                   << uR"($"String" excludes "an")"_s          << u"10111"_s;
        QTest::newRow("starts")                     << uR"($"String" starts "apple")"_s         << u"10001"_s;
        QTest::newRow("ends")                       << uR"($"String" ends "e")"_s               << u"10001"_s;
        QTest::newRow("matches")                    << uR"($"String" matches "^[a-c]")"_s       << u"10101"_s;
        QTest::newRow("matchesCaseInsensitive")     << uR"($"String" matchesCaseInsensitive "^[a-c]")"_s << u"11101"_s;
        // Not a raw string literal, as moc fails to parse one containing an unbalanced bracket
        QTest::newRow("matches invalid regex")      << u"$\"String\" matches \"[a\""_s      << u"invalid"_s;
        QTest::newRow("string op on int")           << uR"($"Int" includes "1")"_s              << u"01000"_s;
        QTest::newRow("string op on float")         << uR"($"Float" starts "-")"_s              << u"10000"_s;

        // Both sides are attributes
        QTest::newRow("int == int")                 << uR"($"Int" == $"Int2")"_s                << u"01010"_s;
        QTest::newRow("int < int")                  << uR"($"Int" < $"Int2")"_s                 << u"10001"_s;
        QTest::newRow("int >= float attributes")    << uR"($"Int" >= $"Float")"_s               << u"11010"_s;
        QTest::newRow("numerical op on strings")    << uR"($"String" < $"String2")"_s           << u"invalid"_s;

        // Neither side is an attribute
        QTest::newRow("value < value")              << uR"(1 < 2)"_s                            << u"11111"_s;
        QTest::newRow("value == value")             << uR"("a" == "b")"_s                       << u"00000"_s;

        // Unary
        QTest::newRow("hasValue")                   << uR"($"Partial" hasValue)"_s              << u"10101"_s;
        QTest::newRow("hasValue without missing")   << uR"($"Int" hasValue)"_s                  << u"11111"_s;

        // Compound
        QTest::newRow("and")                        << uR"($"Int" > 0 and $"Int" < 4)"_s        << u"01110"_s;
        QTest::newRow("or")                         << uR"($"Int" == 0 or $"String" == "cherry")"_s << u"10100"_s;
        QTest::newRow("parentheses")                << uR"(($"Int" < 2 or $"Int" > 3) and $"String" includes "a")"_s << u"11001"_s;
        QTest::newRow("invalid operand")            << uR"($"Int" > 0 and $"String" < 1)"_s     << u"invalid"_s;

        // Attributes that can't be used
        QTest::newRow("unknown attribute")          << uR"($"Unknown" == 1)"_s                  << u"invalid"_s;
        QTest::newRow("edge attribute")             << uR"($"Edge Int" == 1)"_s                 << u"invalid"_s;
    }

    void conditions()
    {
        QFETCH(const QString, condition);
        QFETCH(const QString, expected);

        QCOMPARE(evaluate(condition), expected);
    }
};

QTEST_MAIN(TestConditionFnCreator)
#include "tst_conditionfncreator.moc"
