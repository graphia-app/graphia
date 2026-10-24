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

#include "app/graph/elementiddistinctsetcollection.h"

#include <QTest>
#include <QString>
#include <QStringList>

#include <map>
#include <memory>
#include <utility>

using namespace Qt::Literals::StringLiterals;

namespace
{
// A path of four nodes, with node attributes that group them into pairs
std::unique_ptr<TestGraphModel> createPathGraph()
{
    auto model = std::make_unique<TestGraphModel>(QStringList{u"a-b"_s, u"b-c"_s, u"c-d"_s});

    model->addNodeAttribute<QString>(u"Group"_s, {{u"a"_s, u"X"_s}, {u"b"_s, u"X"_s}, {u"c"_s, u"Y"_s}, {u"d"_s, u"Y"_s}});
    model->addNodeAttribute<int>(u"Score"_s, {{u"a"_s, 1}, {u"b"_s, 3}, {u"c"_s, 10}, {u"d"_s, 20}});
    model->addNodeAttribute<double>(u"Value"_s, {{u"a"_s, 1.5}, {u"b"_s, 2.0}, {u"c"_s, -3.7}, {u"d"_s, 0.0}});
    model->addNodeAttribute<QString>(u"Id"_s, {{u"a"_s, u"gene_1"_s}, {u"b"_s, u"gene_2"_s}, {u"c"_s, u"prot_3"_s}, {u"d"_s, u"other"_s}});
    model->addEdgeAttribute<int>(u"Weight"_s, {{{u"a"_s, u"b"_s}, 1}, {{u"b"_s, u"c"_s}, 2}, {{u"c"_s, u"d"_s}, 3}});

    return model;
}

// A complete graph of four nodes, where every edge has a distinct weight
std::unique_ptr<TestGraphModel> createWeightedCompleteGraph()
{
    auto model = std::make_unique<TestGraphModel>(QStringList
        {u"a-b"_s, u"a-c"_s, u"a-d"_s, u"b-c"_s, u"b-d"_s, u"c-d"_s});

    model->addEdgeAttribute<int>(u"Weight"_s,
    {
        {{u"a"_s, u"b"_s}, 6}, {{u"a"_s, u"c"_s}, 5}, {{u"a"_s, u"d"_s}, 4},
        {{u"b"_s, u"c"_s}, 3}, {{u"b"_s, u"d"_s}, 2}, {{u"c"_s, u"d"_s}, 1},
    });

    return model;
}

std::unique_ptr<TestGraphModel> createCompleteGraph(int numNodes)
{
    QStringList edges;
    for(int i = 0; i < numNodes; i++)
    {
        for(int j = i + 1; j < numNodes; j++)
            edges.append(u"n%1-n%2"_s.arg(i).arg(j));
    }

    return std::make_unique<TestGraphModel>(edges);
}

QStringList mergedNodeNames(const TestGraphModel& model, const QString& nodeName)
{
    auto nodeId = model.nodeId(nodeName);

    // The merged set is only complete when queried from its head
    if(model.graph().typeOf(nodeId) == MultiElementType::Tail)
    {
        for(auto otherNodeId : model.graph().nodeIds())
        {
            if(model.graph().typeOf(otherNodeId) != MultiElementType::Head)
                continue;

            auto mergedNodeIds = model.graph().mergedNodeIdsForNodeId(otherNodeId);
            if(std::ranges::find(mergedNodeIds, nodeId) != mergedNodeIds.end())
            {
                nodeId = otherNodeId;
                break;
            }
        }
    }

    const auto mergedNodeIds = model.graph().mergedNodeIdsForNodeId(nodeId);

    QStringList names;
    names.reserve(mergedNodeIds.size());
    for(auto mergedNodeId : mergedNodeIds)
        names.append(const_cast<TestGraphModel&>(model).graphModel().nodeName(mergedNodeId));

    names.sort();
    return names;
}
} // namespace

class TestAttributeTransforms : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void nearestNeighbours_data()
    {
        QTest::addColumn<QString>("transform");
        QTest::addColumn<QStringList>("expectedEdges");

        // Each node keeps its top ranked edges, and an edge is kept if either end keeps it
        QTest::newRow("k-NN k = 1")
            << u"KNN using $Weight with k = 1"_s
            << QStringList{u"a-b"_s, u"a-c"_s, u"a-d"_s};
        QTest::newRow("k-NN k = 2")
            << u"KNN using $Weight with k = 2"_s
            << QStringList{u"a-b"_s, u"a-c"_s, u"a-d"_s, u"b-c"_s, u"b-d"_s};
        QTest::newRow("k-NN ascending")
            << u"KNN using $Weight with k = 1 RankOrder = Ascending"_s
            << QStringList{u"a-d"_s, u"b-d"_s, u"c-d"_s};
        QTest::newRow("%-NN percentage")
            << u"PercentNN using $Weight with Percent = 50 Minimum = 1"_s
            << QStringList{u"a-b"_s, u"a-c"_s, u"a-d"_s};
        QTest::newRow("%-NN minimum")
            << u"PercentNN using $Weight with Percent = 10 Minimum = 2"_s
            << QStringList{u"a-b"_s, u"a-c"_s, u"a-d"_s, u"b-c"_s, u"b-d"_s};
    }

    void nearestNeighbours()
    {
        QFETCH(const QString, transform);
        QFETCH(const QStringList, expectedEdges);

        auto model = createWeightedCompleteGraph();
        QCOMPARE(model->applyTransforms({transform}), QStringList{});
        QCOMPARE(model->edgeNames(), expectedEdges);
    }

    void nearestNeighbourRanks()
    {
        auto model = createWeightedCompleteGraph();
        QCOMPARE(model->applyTransforms({u"KNN using $Weight with k = 2"_s}), QStringList{});

        auto check = [&model](const QString& from, const QString& to, int source, int target, double mean)
        {
            QCOMPARE(model->edgeNumericValue(u"KNNSourceRank"_s, from, to), static_cast<double>(source));
            QCOMPARE(model->edgeNumericValue(u"KNNTargetRank"_s, from, to), static_cast<double>(target));
            QCOMPARE(model->edgeNumericValue(u"KNNMeanRank"_s, from, to), mean);
        };

        check(u"a"_s, u"b"_s, 1, 1, 1.0);
        check(u"a"_s, u"c"_s, 2, 1, 1.5);
        check(u"b"_s, u"c"_s, 2, 2, 2.0);
        check(u"a"_s, u"d"_s, 0, 1, 1.0);
        check(u"b"_s, u"d"_s, 0, 2, 2.0);
    }

    void edgeReductionRetainsAll()
    {
        auto model = createCompleteGraph(10);
        QCOMPARE(model->applyTransforms({u"EdgeReduction with Percentage = 100 Minimum = 1"_s}), QStringList{});
        QCOMPARE(model->graph().numEdges(), 45);
    }

    void edgeReductionMinimum()
    {
        auto model = createCompleteGraph(10);
        QCOMPARE(model->applyTransforms({u"EdgeReduction with Percentage = 0 Minimum = 3"_s}), QStringList{});

        QVERIFY(model->graph().numEdges() < 45);
        for(auto nodeId : model->graph().nodeIds())
            QVERIFY(model->graph().nodeById(nodeId).degree() >= 3);
    }

    void edgeReductionIsDeterministic()
    {
        auto first = createCompleteGraph(10);
        auto second = createCompleteGraph(10);
        const QStringList transforms = {u"EdgeReduction with Percentage = 20 Minimum = 1"_s};

        QCOMPARE(first->applyTransforms(transforms), QStringList{});
        QCOMPARE(second->applyTransforms(transforms), QStringList{});
        QCOMPARE(first->edgeNames(), second->edgeNames());
    }

    void contractByAttribute()
    {
        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms({u"ContractByAttribute using $Group"_s}), QStringList{});

        QCOMPARE(mergedNodeNames(*model, u"a"_s), (QStringList{u"a"_s, u"b"_s}));
        QCOMPARE(mergedNodeNames(*model, u"c"_s), (QStringList{u"c"_s, u"d"_s}));
        QCOMPARE(model->graph().numEdges(), 1);
        QCOMPARE(model->graph().numComponents(), 1);
    }

    void separateByAttribute()
    {
        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms({u"SeparateByAttribute using $Group"_s}), QStringList{});

        QCOMPARE(model->edgeNames(), (QStringList{u"a-b"_s, u"c-d"_s}));
        QCOMPARE(model->graph().numComponents(), 2);
    }

    void typeCast_data()
    {
        QTest::addColumn<QString>("transform");
        QTest::addColumn<QStringList>("expected");

        QTest::newRow("float to integer")
            << uR"(TypeCast using $Value with Type = Integer Name = "Cast")"_s
            << QStringList{u"1"_s, u"2"_s, u"-3"_s, u"0"_s};
        QTest::newRow("float to string")
            << uR"(TypeCast using $Value with Type = String Name = "Cast")"_s
            << QStringList{u"1.5"_s, u"2"_s, u"-3.7"_s, u"0"_s};
        QTest::newRow("integer to float")
            << uR"(TypeCast using $Score with Type = Float Name = "Cast")"_s
            << QStringList{u"1"_s, u"3"_s, u"10"_s, u"20"_s};
        QTest::newRow("string to integer")
            << uR"(TypeCast using $Group with Type = Integer Name = "Cast")"_s
            << QStringList{u"0"_s, u"0"_s, u"0"_s, u"0"_s};
    }

    void typeCast()
    {
        QFETCH(const QString, transform);
        QFETCH(const QStringList, expected);

        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms({transform}), QStringList{});

        QStringList actual;
        actual.reserve(4);
        for(const auto& node : {u"a"_s, u"b"_s, u"c"_s, u"d"_s})
            actual.append(model->nodeValue(u"Cast"_s, node));

        QCOMPARE(actual, expected);
    }

    void averageAttribute()
    {
        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms({u"AverageAttribute using $Group $Score"_s}), QStringList{});

        QCOMPARE(model->nodeNumericValue(u"MeanScoreOfGroup"_s, u"a"_s), 2.0);
        QCOMPARE(model->nodeNumericValue(u"MeanScoreOfGroup"_s, u"b"_s), 2.0);
        QCOMPARE(model->nodeNumericValue(u"MeanScoreOfGroup"_s, u"c"_s), 15.0);
        QCOMPARE(model->nodeNumericValue(u"MeanScoreOfGroup"_s, u"d"_s), 15.0);
    }

    void averageAttributeWithEmptyValues()
    {
        auto model = createPathGraph();
        model->addNodeAttribute<QString>(u"Partial Group"_s,
            {{u"a"_s, u"X"_s}, {u"b"_s, u"X"_s}, {u"c"_s, u""_s}, {u"d"_s, u"Y"_s}});

        QCOMPARE(model->applyTransforms({uR"(AverageAttribute using $"Partial Group" $Score)"_s}), QStringList{});

        QCOMPARE(model->nodeNumericValue(u"MeanScoreOfPartialGroup"_s, u"a"_s), 2.0);
        QVERIFY(!model->nodeHasValue(u"MeanScoreOfPartialGroup"_s, u"c"_s));
        QCOMPARE(model->nodeNumericValue(u"MeanScoreOfPartialGroup"_s, u"d"_s), 20.0);
    }

    void combineAttributes_data()
    {
        QTest::addColumn<QString>("transform");
        QTest::addColumn<QStringList>("expected");

        QTest::newRow("default")
            << uR"(CombineAttributes using $Group $Score with Name = "Combined")"_s
            << QStringList{u"X 1"_s, u"X 3"_s, u"Y 10"_s, u"Y 20"_s};
        QTest::newRow("reordered")
            << uR"(CombineAttributes using $Group $Score with Name = "Combined" AttributeValue = "\2:\1")"_s
            << QStringList{u"1:X"_s, u"3:X"_s, u"10:Y"_s, u"20:Y"_s};
        QTest::newRow("numeric")
            << uR"(CombineAttributes using $Score $Score with Name = "Combined" AttributeValue = "\1\2")"_s
            << QStringList{u"11"_s, u"33"_s, u"1010"_s, u"2020"_s};
    }

    void combineAttributes()
    {
        QFETCH(const QString, transform);
        QFETCH(const QStringList, expected);

        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms({transform}), QStringList{});

        QStringList actual;
        actual.reserve(4);
        for(const auto& node : {u"a"_s, u"b"_s, u"c"_s, u"d"_s})
            actual.append(model->nodeValue(u"Combined"_s, node));

        QCOMPARE(actual, expected);
    }

    void combinedNumericValuesAreNumbers()
    {
        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms(
            {uR"(CombineAttributes using $Score $Score with Name = "Combined" AttributeValue = "\1\2")"_s}),
            QStringList{});

        QCOMPARE(model->graphModel().attributeValueByName(u"Combined"_s).valueType(), ValueType::Int);
        QCOMPARE(model->nodeNumericValue(u"Combined"_s, u"d"_s), 2020.0);
    }

    void attributeSynthesis()
    {
        // Outside QCOMPARE, as MSVC wrongly sees escape sequences in raw strings in macro arguments
        const auto transform =
            uR"(AttributeSynthesis using $Id with Name = "Prefix" RegularExpression = "^([a-z]+)_\d+$" AttributeValue = "\1")"_s;

        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms({transform}), QStringList{});

        QCOMPARE(model->nodeValue(u"Prefix"_s, u"a"_s), u"gene"_s);
        QCOMPARE(model->nodeValue(u"Prefix"_s, u"b"_s), u"gene"_s);
        QCOMPARE(model->nodeValue(u"Prefix"_s, u"c"_s), u"prot"_s);
        QCOMPARE(model->nodeValue(u"Prefix"_s, u"d"_s), u""_s);
    }

    void attributeSynthesisNonMatchingHaveNoValue()
    {
        // Outside QCOMPARE, as MSVC wrongly sees escape sequences in raw strings in macro arguments
        const auto stringTransform =
            uR"(AttributeSynthesis using $Id with Name = "Prefix" RegularExpression = "^([a-z]+)_\d+$" AttributeValue = "\1")"_s;
        const auto intTransform =
            uR"(AttributeSynthesis using $Id with Name = "Number" RegularExpression = "^[a-z]+_(\d+)$" AttributeValue = "\1")"_s;

        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms({stringTransform, intTransform}), QStringList{});

        QVERIFY(model->nodeHasValue(u"Prefix"_s, u"a"_s));
        QVERIFY(!model->nodeHasValue(u"Prefix"_s, u"d"_s));

        QCOMPARE(model->graphModel().attributeValueByName(u"Number"_s).valueType(), ValueType::Int);
        QCOMPARE(model->nodeNumericValue(u"Number"_s, u"c"_s), 3.0);
        QVERIFY(model->nodeHasValue(u"Number"_s, u"c"_s));
        QVERIFY(!model->nodeHasValue(u"Number"_s, u"d"_s));

        QCOMPARE(model->applyTransforms({stringTransform, intTransform, u"RemoveNodes where $Number hasValue"_s}), QStringList{});
        QCOMPARE(model->graph().numNodes(), 1);
    }

    void forwardAttribute()
    {
        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms(
        {
            u"ContractEdges where $Weight == 1"_s,
            u"ForwardAttribute using $Score"_s
        }), QStringList{});

        // a and b are merged, so they now share the value of whichever is the head
        auto aScore = model->nodeValue(u"Score"_s, u"a"_s);
        QCOMPARE(model->nodeValue(u"Score"_s, u"b"_s), aScore);
        QVERIFY(aScore == u"1"_s || aScore == u"3"_s);

        // Nodes that aren't merged keep their own values
        QCOMPARE(model->nodeValue(u"Score"_s, u"c"_s), u"10"_s);
        QCOMPARE(model->nodeValue(u"Score"_s, u"d"_s), u"20"_s);
    }

    void forwardEdgeAttribute()
    {
        // Contracting a-b makes a-c and b-c parallel, so they're merged into one edge
        auto model = std::make_unique<TestGraphModel>(QStringList{u"a-b"_s, u"a-c"_s, u"b-c"_s});
        model->addEdgeAttribute<int>(u"Weight"_s, {{{u"a"_s, u"b"_s}, 1}, {{u"a"_s, u"c"_s}, 5}, {{u"b"_s, u"c"_s}, 7}});

        QCOMPARE(model->applyTransforms(
        {
            u"ContractEdges where $Weight == 1"_s,
            u"ForwardAttribute using $Weight"_s
        }), QStringList{});

        auto acWeight = model->edgeValue(u"Weight"_s, u"a"_s, u"c"_s);
        QCOMPARE(model->edgeValue(u"Weight"_s, u"b"_s, u"c"_s), acWeight);
        QVERIFY(acWeight == u"5"_s || acWeight == u"7"_s);
    }

    void booleanNodeAttribute()
    {
        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms({uR"(BooleanNodeAttribute with Name = "High" where $Score > 2)"_s}), QStringList{});

        QCOMPARE(model->nodeValue(u"High"_s, u"a"_s), u"False"_s);
        QCOMPARE(model->nodeValue(u"High"_s, u"b"_s), u"True"_s);
        QCOMPARE(model->nodeValue(u"High"_s, u"c"_s), u"True"_s);
        QCOMPARE(model->nodeValue(u"High"_s, u"d"_s), u"True"_s);

        QCOMPARE(model->graphModel().attributeValueByName(u"High"_s).elementType(), ElementType::Node);
    }

    // Each of the transforms that make attributes, applied to edge attributes
    void edgeAttributes_data()
    {
        QTest::addColumn<QString>("transform");
        QTest::addColumn<QString>("attributeName");
        QTest::addColumn<QStringList>("expected");

        QTest::newRow("boolean")
            << uR"(BooleanEdgeAttribute with Name = "Heavy" where $Weight > 1)"_s
            << u"Heavy"_s << QStringList{u"False"_s, u"True"_s, u"True"_s};
        QTest::newRow("type cast")
            << uR"(TypeCast using $Weight with Type = String Name = "Cast")"_s
            << u"Cast"_s << QStringList{u"1"_s, u"2"_s, u"3"_s};
        QTest::newRow("average")
            << u"AverageAttribute using $Kind $Weight"_s
            << u"MeanWeightOfKind"_s << QStringList{u"1.5"_s, u"1.5"_s, u"3"_s};
        QTest::newRow("combine")
            << uR"(CombineAttributes using $Kind $Weight with Name = "Combined")"_s
            << u"Combined"_s << QStringList{u"x 1"_s, u"x 2"_s, u"y 3"_s};
        QTest::newRow("synthesis")
            << uR"(AttributeSynthesis using $Label with Name = "Suffix" RegularExpression = "^e_(.*)$" AttributeValue = "\1")"_s
            << u"Suffix"_s << QStringList{u"one"_s, u"two"_s, u"three"_s};
    }

    void edgeAttributes()
    {
        QFETCH(const QString, transform);
        QFETCH(const QString, attributeName);
        QFETCH(const QStringList, expected);

        auto model = createPathGraph();
        model->addEdgeAttribute<QString>(u"Kind"_s, {{{u"a"_s, u"b"_s}, u"x"_s}, {{u"b"_s, u"c"_s}, u"x"_s}, {{u"c"_s, u"d"_s}, u"y"_s}});
        model->addEdgeAttribute<QString>(u"Label"_s,
            {{{u"a"_s, u"b"_s}, u"e_one"_s}, {{u"b"_s, u"c"_s}, u"e_two"_s}, {{u"c"_s, u"d"_s}, u"e_three"_s}});

        QCOMPARE(model->applyTransforms({transform}), QStringList{});
        QCOMPARE(model->graphModel().attributeValueByName(attributeName).elementType(), ElementType::Edge);

        QStringList actual;
        actual.reserve(3);
        for(const auto& [from, to] : {std::pair{u"a"_s, u"b"_s}, std::pair{u"b"_s, u"c"_s}, std::pair{u"c"_s, u"d"_s}})
            actual.append(model->edgeValue(attributeName, from, to));

        QCOMPARE(actual, expected);
    }

    void floatResults_data()
    {
        QTest::addColumn<QString>("transform");
        QTest::addColumn<QString>("attributeName");

        QTest::newRow("synthesis")
            << uR"(AttributeSynthesis using $Value with Name = "Synthesised" RegularExpression = "(.*)$" AttributeValue = "\1")"_s
            << u"Synthesised"_s;
        QTest::newRow("combine")
            << uR"(CombineAttributes using $Value $Score with Name = "Combined" AttributeValue = "\1")"_s
            << u"Combined"_s;
    }

    // When every value made looks like a non-integer number, the attribute is a float attribute
    void floatResults()
    {
        QFETCH(const QString, transform);
        QFETCH(const QString, attributeName);

        auto model = createPathGraph();
        QCOMPARE(model->applyTransforms({transform}), QStringList{});

        QCOMPARE(model->graphModel().attributeValueByName(attributeName).valueType(), ValueType::Float);
        QCOMPARE(model->nodeNumericValue(attributeName, u"a"_s), 1.5);
        QCOMPARE(model->nodeNumericValue(attributeName, u"c"_s), -3.7);
    }

    void invalidConfigs_data()
    {
        QTest::addColumn<QString>("transform");
        QTest::addColumn<bool>("valid");

        QTest::newRow("type cast")
            << uR"(TypeCast using $Value with Type = String Name = "Cast")"_s << true;
        QTest::newRow("type cast invalid name")
            << uR"(TypeCast using $Value with Type = String Name = "1 Cast")"_s << false;
        QTest::newRow("synthesis")
            << uR"(AttributeSynthesis using $Id with Name = "S" RegularExpression = "(.*)$" AttributeValue = "\1")"_s << true;
        QTest::newRow("synthesis invalid name")
            << uR"(AttributeSynthesis using $Id with Name = "S!" RegularExpression = "(.*)$" AttributeValue = "\1")"_s << false;
        QTest::newRow("synthesis invalid regex")
            << u"AttributeSynthesis using $Id with Name = \"S\" RegularExpression = \"(.*\" AttributeValue = \"\\1\""_s << false; // NOLINT modernize-raw-string-literal
        QTest::newRow("combine")
            << uR"(CombineAttributes using $Group $Score with Name = "C")"_s << true;
        QTest::newRow("combine one attribute")
            << uR"(CombineAttributes using $Group with Name = "C")"_s << false;
        QTest::newRow("combine invalid name")
            << uR"(CombineAttributes using $Group $Score with Name = "")"_s << false;
        QTest::newRow("average")
            << u"AverageAttribute using $Group $Score"_s << true;
        QTest::newRow("average one attribute")
            << u"AverageAttribute using $Group"_s << false;
        QTest::newRow("average node and edge")
            << u"AverageAttribute using $Group $Weight"_s << false;
        QTest::newRow("boolean")
            << uR"(BooleanNodeAttribute with Name = "B" where $Score > 2)"_s << true;
        QTest::newRow("boolean invalid name")
            << uR"(BooleanNodeAttribute with Name = "-B" where $Score > 2)"_s << false;
        QTest::newRow("boolean no condition")
            << uR"(BooleanNodeAttribute with Name = "B")"_s << false;
        QTest::newRow("forward")
            << u"ForwardAttribute using $Score"_s << true;
        QTest::newRow("forward no attribute")
            << u"ForwardAttribute"_s << false;
    }

    // The UI checks configs with graphTransformIsValid; an invalid config that's applied anyway
    // makes nothing, and says why
    void invalidConfigs()
    {
        QFETCH(const QString, transform);
        QFETCH(const bool, valid);

        auto model = createPathGraph();
        QCOMPARE(model->graphModel().graphTransformIsValid(transform), valid);

        if(valid)
            return;

        const auto attributeNames = model->graphModel().attributeNames();
        const auto alerts = model->applyTransforms({transform});

        QVERIFY(!alerts.isEmpty());

        QVERIFY(model->graphModel().attributeNames() == attributeNames);
    }
};

QTEST_MAIN(TestAttributeTransforms)
#include "tst_attributetransforms.moc"
