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

#include "app/attributes/attribute.h"
#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphcomponent.h"

#include <QString>
#include <QStringList>
#include <QTest>

#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

// The attributes of nodes, edges and components, through which transforms,
// visualisations and the UI read the values of elements
class TestAttribute : public QObject
{
    Q_OBJECT

    // Two components, a-b-c and d
    std::unique_ptr<TestGraphModel> _model;

    const IGraphComponent& component(const QString& nodeName) const
    {
        const auto& graph = _model->graph();
        return *graph.componentById(graph.componentIdOfNode(_model->nodeId(nodeName)));
    }

    // A string attribute with the given values for the nodes a, b, c and d
    Attribute stringAttribute(const QStringList& values) const
    {
        std::map<NodeId, QString> valuesById;
        const auto ids = nodeIds();
        for(size_t i = 0; i < ids.size() && i < static_cast<size_t>(values.size()); i++)
            valuesById.emplace(ids.at(i), values.at(static_cast<int>(i)));

        Attribute attribute;
        attribute.setStringValueFn([valuesById](NodeId nodeId) { return valuesById.at(nodeId); });
        return attribute;
    }

    static QStringList sharedValues(const std::vector<IAttribute::SharedValue>& sharedValues)
    {
        QStringList values;
        values.reserve(static_cast<qsizetype>(sharedValues.size()));
        for(const auto& sharedValue : sharedValues)
            values.append(u"%1:%2"_s.arg(sharedValue._value).arg(sharedValue._count));

        return values;
    }

    std::vector<NodeId> nodeIds() const
    {
        return {_model->nodeId(u"a"_s), _model->nodeId(u"b"_s), _model->nodeId(u"c"_s), _model->nodeId(u"d"_s)};
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        _model = std::make_unique<TestGraphModel>(QStringList{u"a-b"_s, u"b-c"_s, u"d"_s});
        _model->applyTransforms({});
    }

    void cleanupTestCase()
    {
        _model.reset();
    }

    void types()
    {
        const Attribute invalid;
        QVERIFY(!invalid.isValid());
        QCOMPARE(invalid.elementType(), ElementType::None);
        QCOMPARE(invalid.valueType(), ValueType::Unknown);

        Attribute attribute;
        attribute.setIntValueFn([](NodeId) { return 1; });
        QVERIFY(attribute.isValid());
        QCOMPARE(attribute.elementType(), ElementType::Node);
        QCOMPARE(attribute.valueType(), ValueType::Int);
        QVERIFY(attribute.isOfElementType<NodeId>());
        QVERIFY(!attribute.isOfElementType<EdgeId>());

        // Setting a value function replaces any other
        attribute.setFloatValueFn([](EdgeId) { return 1.0; });
        QCOMPARE(attribute.elementType(), ElementType::Edge);
        QCOMPARE(attribute.valueType(), ValueType::Float);
        QVERIFY(attribute.isOfElementType<EdgeId>());

        attribute.setStringValueFn([](const IGraphComponent&) { return u"x"_s; });
        QCOMPARE(attribute.elementType(), ElementType::Component);
        QCOMPARE(attribute.valueType(), ValueType::String);
        QVERIFY(attribute.isOfElementType<const IGraphComponent&>());

        attribute.setIntValueFn([](const IGraphComponent&) { return 1; });
        QCOMPARE(attribute.valueType(), ValueType::Int);
        attribute.setFloatValueFn([](const IGraphComponent&) { return 1.0; });
        QCOMPARE(attribute.valueType(), ValueType::Float);
        attribute.setStringValueFn([](EdgeId) { return u"x"_s; });
        QCOMPARE(attribute.elementType(), ElementType::Edge);
        QCOMPARE(attribute.valueType(), ValueType::String);
    }

    void values()
    {
        const auto& c = component(u"c"_s);
        const auto& d = component(u"d"_s);

        Attribute componentInt;
        componentInt.setIntValueFn([](const IGraphComponent& component) { return static_cast<int>(component.numNodes()); });
        QCOMPARE(componentInt.valueOf<int>(c), 3);
        QCOMPARE(componentInt.floatValueOf(d), 1.0);
        QCOMPARE(componentInt.numericValueOf(c), 3.0);
        QCOMPARE(componentInt.stringValueOf(c), u"3"_s);

        Attribute componentFloat;
        componentFloat.setFloatValueFn([](const IGraphComponent& component) { return static_cast<double>(component.numNodes()) / 2.0; });
        QCOMPARE(componentFloat.valueOf<double>(c), 1.5);
        QCOMPARE(componentFloat.intValueOf(c), 1);
        QCOMPARE(componentFloat.numericValueOf(d), 0.5);
        QCOMPARE(componentFloat.stringValueOf(c), u"1.5"_s);

        Attribute componentString;
        componentString.setStringValueFn([](const IGraphComponent& component)
            { return QString::number(component.numEdges()); });
        QCOMPARE(componentString.valueOf<QString>(c), u"2"_s);
        QCOMPARE(componentString.stringValueOf(d), u"0"_s);
        QVERIFY(std::isnan(componentString.numericValueOf(c)));

        // A value function can be passed the attribute itself
        auto a = _model->nodeId(u"a"_s);
        Attribute self;
        self.setIntValueFn([](NodeId, const IAttribute& attribute) { return attribute.userDefined() ? 1 : 0; });
        QCOMPARE(self.valueOf<int>(a), 0);
        self.setUserDefined(true);
        QCOMPARE(self.valueOf<int>(a), 1);
    }

    void missingValues()
    {
        const auto& c = component(u"c"_s);
        const auto& d = component(u"d"_s);

        Attribute attribute;
        attribute.setIntValueFn([](const IGraphComponent&) { return 0; });
        QVERIFY(!attribute.hasMissingValues());
        QVERIFY(!attribute.valueMissingOf(c));

        attribute.setValueMissingFn([](const IGraphComponent& component) { return component.numNodes() == 1uz; });
        QVERIFY(attribute.hasMissingValues());
        QVERIFY(!attribute.valueMissingOf(c));
        QVERIFY(attribute.valueMissingOf(d));

        // A missing value function for another type of element replaces it, but is ignored
        QTest::ignoreMessage(QtDebugMsg, "Setting value missing function with mismatched element type");
        attribute.setValueMissingFn([](NodeId) { return true; });
        QVERIFY(!attribute.hasMissingValues());
        QVERIFY(!attribute.valueMissingOf(d));

        QTest::ignoreMessage(QtDebugMsg, "Setting value missing function with mismatched element type");
        attribute.setValueMissingFn([](EdgeId) { return true; });
        QVERIFY(!attribute.hasMissingValues());

        QVERIFY(!Attribute().hasMissingValues());
    }

    void setValues()
    {
        const auto& c = component(u"c"_s);

        QString setTo;
        Attribute attribute;
        attribute.setStringValueFn([](const IGraphComponent&) { return u""_s; });
        QVERIFY(!attribute.editable());

        attribute.setSetValueFn([&setTo](const IGraphComponent& component, const QString& value)
            { setTo = u"%1 %2"_s.arg(component.numNodes()).arg(value); });
        QVERIFY(attribute.editable());
        attribute.setValueOf(c, u"x"_s);
        QCOMPARE(setTo, u"3 x"_s);

        QTest::ignoreMessage(QtDebugMsg, "Setting set value function with mismatched element type");
        attribute.setSetValueFn([&setTo](NodeId, const QString& value) { setTo = value; });
        QVERIFY(attribute.editable());

        QTest::ignoreMessage(QtDebugMsg, "Setting set value function with mismatched element type");
        attribute.setSetValueFn([&setTo](EdgeId, const QString& value) { setTo = value; });
        attribute.setValueOf(_model->edgeId(u"a"_s, u"b"_s), u"y"_s);
        QCOMPARE(setTo, u"y"_s);
    }

    void ranges()
    {
        Attribute intAttribute;
        intAttribute.setIntValueFn([](NodeId) { return 0; });
        intAttribute.setFlag(AttributeFlag::AutoRange);

        auto& intRange = intAttribute.intRange();
        QVERIFY(!intRange.hasMin());
        QVERIFY(!intRange.hasMax());
        QVERIFY(!intRange.hasRange());
        QCOMPARE(intRange.min(), std::numeric_limits<int>::lowest());
        QCOMPARE(intRange.max(), std::numeric_limits<int>::max());

        intRange.setMin(-2);
        QVERIFY(intRange.hasMin());
        QVERIFY(!intRange.hasRange());
        QCOMPARE(intRange.min(), -2);

        // Setting the range explicitly turns off setting it automatically
        QVERIFY(!intAttribute.testFlag(AttributeFlag::AutoRange));

        intRange.setMax(5);
        QVERIFY(intRange.hasRange());
        QCOMPARE(intAttribute.numericRange().min(), -2.0);
        QCOMPARE(intAttribute.numericRange().max(), 5.0);
        QVERIFY(intAttribute.numericRange().hasRange());

        Attribute floatAttribute;
        floatAttribute.setFloatValueFn([](NodeId) { return 0.0; });

        const auto& numericRange = floatAttribute.numericRange();
        QVERIFY(!numericRange.hasMin());
        QVERIFY(!numericRange.hasMax());
        QVERIFY(!numericRange.hasRange());
        QCOMPARE(numericRange.min(), std::numeric_limits<double>::lowest());
        QCOMPARE(numericRange.max(), std::numeric_limits<double>::max());

        floatAttribute.floatRange().setMax(2.5);
        QVERIFY(numericRange.hasMax());
        QVERIFY(!numericRange.hasRange());
        QCOMPARE(numericRange.max(), 2.5);
        floatAttribute.floatRange().setMin(-0.5);
        QVERIFY(numericRange.hasRange());
        QCOMPARE(numericRange.min(), -0.5);

        // A string attribute has no numeric range
        const auto strings = stringAttribute({});
        QVERIFY(!strings.numericRange().hasMin());
        QVERIFY(!strings.numericRange().hasMax());
        QVERIFY(!strings.numericRange().hasRange());

        // A copy has a range of its own
        Attribute copy(floatAttribute);
        copy.floatRange().setMax(10.0);
        QCOMPARE(copy.numericRange().max(), 10.0);
        QCOMPARE(floatAttribute.numericRange().max(), 2.5);

        Attribute assigned;
        assigned = copy;
        assigned.floatRange().setMin(-10.0);
        QCOMPARE(assigned.numericRange().min(), -10.0);
        QCOMPARE(copy.numericRange().min(), -0.5);
    }

    void autoRange()
    {
        const std::vector<double> values = {2.5, -1.0, 7.0, 0.0};

        Attribute floatAttribute;
        floatAttribute.setFloatValueFn([&values](NodeId nodeId) { return values.at(static_cast<size_t>(nodeId)); });
        floatAttribute.setFlag(AttributeFlag::AutoRange);
        floatAttribute.autoSetRangeForElements(nodeIds());

        QCOMPARE(floatAttribute.numericRange().min(), -1.0);
        QCOMPARE(floatAttribute.numericRange().max(), 7.0);
        QVERIFY(floatAttribute.testFlag(AttributeFlag::AutoRange));

        Attribute intAttribute;
        intAttribute.setIntValueFn([](NodeId nodeId) { return static_cast<int>(nodeId) * 2; });
        intAttribute.autoSetRangeForElements(nodeIds());
        QCOMPARE(intAttribute.intRange().min(), 0);
        QCOMPARE(intAttribute.intRange().max(), 6);
        QVERIFY(!intAttribute.testFlag(AttributeFlag::AutoRange));

        const auto [min, max] = intAttribute.findRangeforElements(nodeIds());
        QCOMPARE(min, 0.0);
        QCOMPARE(max, 6.0);

        // With no elements, there's no range
        Attribute empty;
        empty.setFloatValueFn([](NodeId) { return 0.0; });
        empty.autoSetRangeForElements(std::vector<NodeId>{});
        QVERIFY(!empty.numericRange().hasMin());
        QVERIFY(!empty.numericRange().hasMax());

        // Nor does a string attribute
        auto strings = stringAttribute({u"a"_s, u"b"_s, u"c"_s, u"d"_s});
        strings.autoSetRangeForElements(nodeIds());
        const auto [stringMin, stringMax] = strings.findRangeforElements(nodeIds());
        QCOMPARE(stringMin, std::numeric_limits<double>::max());
        QCOMPARE(stringMax, std::numeric_limits<double>::lowest());
    }

    void sharedValues()
    {
        // Ordered by how often each value occurs, then numerically; empty values are ignored
        auto attribute = stringAttribute({u"10"_s, u"9"_s, u"10"_s, u""_s});
        QCOMPARE(sharedValues(attribute.findSharedValuesForElements(nodeIds())),
            (QStringList{u"10:2"_s, u"9:1"_s}));

        attribute = stringAttribute({u"x10"_s, u"x9"_s, u"y"_s, u""_s});
        QCOMPARE(sharedValues(attribute.findSharedValuesForElements(nodeIds())),
            (QStringList{u"x9:1"_s, u"x10:1"_s, u"y:1"_s}));

        // ...unless every value is unique
        QVERIFY(attribute.findSharedValuesForElements(nodeIds(), true).empty());
        attribute.updateSharedValuesForElements(nodeIds());
        QVERIFY(attribute.sharedValues().empty());

        attribute = stringAttribute({u"x"_s, u"y"_s, u"x"_s, u"y"_s});
        attribute.updateSharedValuesForElements(nodeIds());
        QCOMPARE(sharedValues(attribute.sharedValues()), (QStringList{u"x:2"_s, u"y:2"_s}));
    }

    void statistics()
    {
        Attribute attribute;
        attribute.setIntValueFn([](NodeId nodeId) { return static_cast<int>(nodeId) + 1; });

        const auto statistics = attribute.findStatisticsforElements(nodeIds(), "includeValues"_yes);
        QCOMPARE(statistics._min, 1.0);
        QCOMPARE(statistics._max, 4.0);
        QCOMPARE(statistics._mean, 2.5);
        QCOMPARE(statistics._values.size(), 4uz);
    }

    void parameters()
    {
        Attribute attribute;
        QVERIFY(!attribute.hasParameter());
        QVERIFY(attribute.parameterValue().isEmpty());

        attribute.setValidParameterValues({u"x"_s, u"y"_s});
        QVERIFY(attribute.hasParameter());
        QCOMPARE(attribute.validParameterValues(), (QStringList{u"x"_s, u"y"_s}));
        QVERIFY(attribute.parameterValue().isEmpty());

        QVERIFY(!attribute.setParameterValue(u"z"_s));
        QVERIFY(attribute.parameterValue().isEmpty());

        QVERIFY(attribute.setParameterValue(u"y"_s));
        QCOMPARE(attribute.parameterValue(), u"y"_s);

        // The value is cleared if it's no longer valid
        attribute.setValidParameterValues({u"x"_s});
        QVERIFY(attribute.parameterValue().isEmpty());
    }

    void parseAttributeName_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<Attribute::EdgeNodeType>("type");
        QTest::addColumn<QString>("attributeName");
        QTest::addColumn<QString>("parameter");
        QTest::addColumn<QString>("written");
        QTest::addColumn<QString>("prettified");

        using T = Attribute::EdgeNodeType;

        QTest::newRow("plain")      << u"Degree"_s << T::None << u"Degree"_s << u""_s
            << u"Degree"_s << u"Degree"_s;
        QTest::newRow("dollar")     << uR"($"Node Degree")"_s << T::None << u"Node Degree"_s << u""_s
            << uR"("Node Degree")"_s << u"Node Degree"_s;
        QTest::newRow("source")     << uR"($source."Weight")"_s << T::Source << u"Weight"_s << u""_s
            << u"source.Weight"_s << u"Source\u2009›\u2009Weight"_s;
        QTest::newRow("target")     << uR"(target."Weight")"_s << T::Target << u"Weight"_s << u""_s
            << u"target.Weight"_s << u"Target\u2009›\u2009Weight"_s;
        QTest::newRow("parameter")  << uR"("Enrichment"."Cluster")"_s << T::None << u"Enrichment"_s
            << u"Cluster"_s << u"Enrichment.Cluster"_s << u"Enrichment\u2009›\u2009Cluster"_s;
        QTest::newRow("source and parameter") << uR"($source."Enrichment"."Cluster")"_s << T::Source
            << u"Enrichment"_s << u"Cluster"_s << u"source.Enrichment.Cluster"_s
            << u"Source\u2009›\u2009Enrichment\u2009›\u2009Cluster"_s;
        QTest::newRow("parameter needing quotes") << uR"($source.Enrichment."Cluster A")"_s << T::Source
            << u"Enrichment"_s << u"Cluster A"_s << uR"(source.Enrichment."Cluster A")"_s
            << u"Source\u2009›\u2009Enrichment\u2009›\u2009Cluster A"_s;

        // Only the quotes around a name are removed, not those within it; these aren't
        // raw strings, as moc misparses those with escaped quotes, missing the test class
        QTest::newRow("quotes in name") << u"Score \"raw\""_s << T::None // NOLINT modernize-raw-string-literal
            << u"Score \"raw\""_s << u""_s << u"\"Score \\\"raw\\\"\""_s << u"Score raw"_s; // NOLINT modernize-raw-string-literal
        QTest::newRow("quoted quotes in name") << u"$\"Score \\\"raw\\\"\""_s << T::None // NOLINT modernize-raw-string-literal
            << u"Score \"raw\""_s << u""_s << u"\"Score \\\"raw\\\"\""_s << u"Score \\raw\\"_s; // NOLINT modernize-raw-string-literal
        QTest::newRow("dot in quotes") << uR"($"a.b".c)"_s << T::None << u"a.b"_s << u"c"_s
            << uR"("a.b".c)"_s << u"a\u2009›\u2009b\u2009›\u2009c"_s;
    }

    void parseAttributeName()
    {
        QFETCH(const QString, name);
        QFETCH(const Attribute::EdgeNodeType, type);
        QFETCH(const QString, attributeName);
        QFETCH(const QString, parameter);
        QFETCH(const QString, written);
        QFETCH(const QString, prettified);

        const auto parsed = Attribute::parseAttributeName(name);
        QCOMPARE(parsed._type, type);
        QCOMPARE(parsed._name, attributeName);
        QCOMPARE(parsed._parameter, parameter);

        QCOMPARE(Attribute::writtenAttributeName(name), written);
        QCOMPARE(Attribute::prettify(name), prettified);
    }

    void edgeNodesAttribute()
    {
        const auto& graph = _model->graphModel().mutableGraph();
        auto ab = _model->edgeId(u"a"_s, u"b"_s);
        auto bc = _model->edgeId(u"b"_s, u"c"_s);

        Attribute intAttribute;
        intAttribute.setIntValueFn([](NodeId nodeId) { return static_cast<int>(nodeId) * 10; });
        intAttribute.setDescription(u"Ten times the id"_s);

        auto source = Attribute::edgeNodesAttribute(graph, intAttribute, Attribute::EdgeNodeType::Source);
        auto target = Attribute::edgeNodesAttribute(graph, intAttribute, Attribute::EdgeNodeType::Target);
        QCOMPARE(source.elementType(), ElementType::Edge);
        QCOMPARE(source.valueType(), ValueType::Int);
        QCOMPARE(source.description(), u"Ten times the id"_s);
        QCOMPARE(source.valueOf<int>(ab), static_cast<int>(_model->nodeId(u"a"_s)) * 10);
        QCOMPARE(target.valueOf<int>(ab), static_cast<int>(_model->nodeId(u"b"_s)) * 10);
        QCOMPARE(source.valueOf<int>(bc), static_cast<int>(_model->nodeId(u"b"_s)) * 10);

        Attribute floatAttribute;
        floatAttribute.setFloatValueFn([](NodeId nodeId) { return static_cast<int>(nodeId) / 2.0; });
        target = Attribute::edgeNodesAttribute(graph, floatAttribute, Attribute::EdgeNodeType::Target);
        QCOMPARE(target.valueType(), ValueType::Float);
        QCOMPARE(target.valueOf<double>(bc), static_cast<int>(_model->nodeId(u"c"_s)) / 2.0);

        auto strings = stringAttribute({u"a"_s, u"b"_s, u"c"_s, u"d"_s});
        source = Attribute::edgeNodesAttribute(graph, strings, Attribute::EdgeNodeType::Source);
        QCOMPARE(source.valueType(), ValueType::String);
        QCOMPARE(source.valueOf<QString>(bc), u"b"_s);

        QVERIFY(!Attribute::edgeNodesAttribute(graph, intAttribute, Attribute::EdgeNodeType::None).isValid());
        QVERIFY(!Attribute::edgeNodesAttribute(graph, source, Attribute::EdgeNodeType::Source).isValid());
        QVERIFY(!Attribute::edgeNodesAttribute(graph, Attribute(), Attribute::EdgeNodeType::Source).isValid());
    }

    // Values shown differently to how they're stored, as cluster and component identifiers are
    // when translated, and what's typed as a value shown is stored again as the value
    void displayValues()
    {
        Attribute numbered;
        numbered.setDisplayValueFns(Attribute::numberedDisplayValueFns(u"Cluster "_s, u"Kluster nr %1 (k)"_s));
        QCOMPARE(numbered.displayValue(u"Cluster 3"_s), u"Kluster nr 3 (k)"_s);
        QCOMPARE(numbered.storedValue(u"Kluster nr 3 (k)"_s), u"Cluster 3"_s);
        QCOMPARE(numbered.storedValue(u"Kluster nr 12 (k)"_s), u"Cluster 12"_s);

        // Anything else is left as it is
        QCOMPARE(numbered.displayValue(u"Cluster x"_s), u"Cluster x"_s);
        QCOMPARE(numbered.displayValue(u"Kluster nr 3 (k)"_s), u"Kluster nr 3 (k)"_s);
        QCOMPARE(numbered.storedValue(u"Kluster nr x (k)"_s), u"Kluster nr x (k)"_s);
        QCOMPARE(numbered.storedValue(u"Kluster nr  (k)"_s), u"Kluster nr  (k)"_s);
        QCOMPARE(numbered.storedValue(u"Kluster nr 3"_s), u"Kluster nr 3"_s);
        QCOMPARE(numbered.storedValue(u"Cluster 3"_s), u"Cluster 3"_s);

        // The number first, or last
        numbered.setDisplayValueFns(Attribute::numberedDisplayValueFns(u"Component "_s, u"%1. komponenten"_s));
        QCOMPARE(numbered.displayValue(u"Component 5"_s), u"5. komponenten"_s);
        QCOMPARE(numbered.storedValue(u"5. komponenten"_s), u"Component 5"_s);
        numbered.setDisplayValueFns(Attribute::numberedDisplayValueFns(u"Component "_s, u"Komponent %1"_s));
        QCOMPARE(numbered.storedValue(u"Komponent 7"_s), u"Component 7"_s);

        Attribute mapped;
        mapped.setDisplayValueFns(Attribute::mappedDisplayValueFns({{u"True"_s, u"Sant"_s}, {u"False"_s, u"Falskt"_s}}));
        QCOMPARE(mapped.displayValue(u"True"_s), u"Sant"_s);
        QCOMPARE(mapped.displayValue(u"False"_s), u"Falskt"_s);
        QCOMPARE(mapped.displayValue(u"Maybe"_s), u"Maybe"_s);
        QCOMPARE(mapped.storedValue(u"Falskt"_s), u"False"_s);
        QCOMPARE(mapped.storedValue(u"Kanske"_s), u"Kanske"_s);

        const Attribute plain;
        QCOMPARE(plain.displayValue(u"x"_s), u"x"_s);
        QCOMPARE(plain.storedValue(u"x"_s), u"x"_s);

        // The graph model does the same for its attributes, by name
        TestGraphModel model({u"a-b"_s});
        model.graphModel().createAttribute(u"Shown"_s)
            .setStringValueFn([](NodeId) { return u"True"_s; })
            .setDisplayValueFns(Attribute::mappedDisplayValueFns({{u"True"_s, u"Sant"_s}}));

        QCOMPARE(model.graphModel().attributeDisplayValue(u"Shown"_s, u"True"_s), u"Sant"_s);
        QCOMPARE(model.graphModel().attributeStoredValue(u"Shown"_s, u"Sant"_s), u"True"_s);
        QCOMPARE(model.graphModel().attributeDisplayValue(u"Missing"_s, u"True"_s), u"True"_s);
        QCOMPARE(model.graphModel().attributeStoredValue(u"Missing"_s, u"Sant"_s), u"Sant"_s);
    }
};

QTEST_MAIN(TestAttribute)
#include "tst_attribute.moc"
