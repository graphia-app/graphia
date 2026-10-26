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

#include "app/loading/gmlsaver.h"
#include "app/loading/graphmlsaver.h"
#include "app/loading/jsongraphsaver.h"
#include "app/loading/pairwisesaver.h"

#include "shared/loading/gmlfileparser.h"
#include "shared/loading/graphmlparser.h"
#include "shared/loading/jsongraphparser.h"
#include "shared/loading/pairwisefileparser.h"

#include <QTest>
#include <QByteArray>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>

#include <functional>
#include <map>
#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
const QString A = u"a & b"_s;
const QString B = u"<b>"_s;
const QString C = u"say \"hi\""_s;
const QString D = u"x%1"_s;
const QString E = u"e"_s;

// The graph that is saved in each format
std::unique_ptr<TestGraphModel> createModel()
{
    auto model = std::make_unique<TestGraphModel>(QStringList{A + u"-"_s + B, B + u"-"_s + C,
        C + u"-"_s + D, D + u"-"_s + A, E});

    model->addNodeAttribute<QString>(u"Label Text"_s,
        {{A, u"A & B"_s}, {B, u"<tag>"_s}, {C, u"q\"q"_s}, {D, u""_s}, {E, u"plain"_s}});
    model->addNodeAttribute<int>(u"Count"_s,
        {{A, 1}, {B, -2}, {C, 3}, {D, 2000000001}, {E, 0}});
    model->addNodeAttribute<double>(u"Score"_s,
        {{A, 0.123456789}, {B, 1e-7}, {C, 12345678.0}, {D, -0.5}, {E, 3.0}});
    model->addEdgeAttribute<double>(u"Weight"_s,
        {{{A, B}, 1.23456789}, {{B, C}, 2.0}, {{C, D}, 0.1}, {{D, A}, 7.25}});

    model->applyTransforms({});

    return model;
}

template<typename Saver>
QByteArray save(TestGraphModel& model)
{
    const QTemporaryDir dir;
    auto fileName = dir.filePath(u"graph"_s);

    Saver saver(QUrl::fromLocalFile(fileName), &model.graphModel());
    if(!saver.save())
        return {};

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
        return {};

    return file.readAll();
}

template<typename Parser>
std::unique_ptr<TestGraphModel> reload(const QByteArray& content)
{
    return TestGraphModel::parse(content, [](auto* userNodeData, auto* userEdgeData)
        { return std::make_unique<Parser>(userNodeData, userEdgeData); });
}

// TestGraphModel lists nodes and edges in sorted order
QStringList sorted(QStringList strings)
{
    strings.sort();
    return strings;
}

// Attribute values are stored as floats; the reloaded values must be exactly
// the same, which QCOMPARE doesn't require of floating point values
bool exactlyEqual(double actual, double expected)
{
    return std::equal_to<>()(static_cast<float>(actual), static_cast<float>(expected));
}

// For formats that don't preserve node names, maps each original name, as given by
// the value of an attribute of the reloaded graph, to the name the node now has
std::map<QString, QString> namesByAttribute(TestGraphModel& model, const QString& attributeName)
{
    std::map<QString, QString> names;

    auto attribute = model.graphModel().attributeValueByName(attributeName);
    for(auto nodeId : model.graph().nodeIds())
        names.emplace(attribute.stringValueOf(nodeId), model.graphModel().nodeName(nodeId));

    return names;
}
} // namespace

class TestSavers : public QObject
{
    Q_OBJECT

private:
    const QStringList _nodes = sorted({A, B, C, D, E});
    const QStringList _edges = sorted({A + u"-"_s + B, B + u"-"_s + C, C + u"-"_s + D, D + u"-"_s + A});

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void gml()
    {
        auto model = createModel();
        auto reloaded = reload<GmlFileParser>(save<GMLSaver>(*model));
        QVERIFY(reloaded != nullptr);

        QCOMPARE(reloaded->nodeNames(), _nodes);
        QCOMPARE(reloaded->edgeNames(), _edges);

        // Non-alphanumeric characters are removed from the names of attributes
        QCOMPARE(reloaded->nodeValue(u"LabelText"_s, A), u"A & B"_s);
        QCOMPARE(reloaded->nodeValue(u"LabelText"_s, B), u"<tag>"_s);
        QCOMPARE(reloaded->nodeValue(u"LabelText"_s, C), u"q\"q"_s);
        QCOMPARE(reloaded->nodeValue(u"LabelText"_s, E), u"plain"_s);

        QCOMPARE(reloaded->nodeValue(u"Count"_s, B), u"-2"_s);
        QCOMPARE(reloaded->nodeValue(u"Count"_s, D), u"2000000001"_s);

        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, A), 0.123456789));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, B), 1e-7));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, C), 12345678.0));

        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, A, B), 1.23456789));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, D, A), 7.25));
    }

    void gmlAttributeNamesThatClash()
    {
        TestGraphModel model({u"a-b"_s});

        // These are the same once non-alphanumeric characters are removed, or once a name
        // starting with a digit has "Attribute" put before it, or are GML's own keys
        model.addNodeAttribute<QString>(u"Gene Name"_s, {{u"a"_s, u"space a"_s}, {u"b"_s, u"space b"_s}});
        model.addNodeAttribute<QString>(u"Gene-Name"_s, {{u"a"_s, u"dash a"_s}, {u"b"_s, u"dash b"_s}});
        model.addNodeAttribute<QString>(u"1st"_s, {{u"a"_s, u"digit a"_s}, {u"b"_s, u"digit b"_s}});
        model.addNodeAttribute<QString>(u"Attribute1st"_s, {{u"a"_s, u"prefixed a"_s}, {u"b"_s, u"prefixed b"_s}});
        model.addNodeAttribute<QString>(u"label"_s, {{u"a"_s, u"label a"_s}, {u"b"_s, u"label b"_s}});
        model.addNodeAttribute<int>(u"id"_s, {{u"a"_s, 100}, {u"b"_s, 200}});
        model.addEdgeAttribute<QString>(u"source"_s, {{{u"a"_s, u"b"_s}, u"from"_s}});
        model.addEdgeAttribute<QString>(u"target"_s, {{{u"a"_s, u"b"_s}, u"to"_s}});
        model.applyTransforms({});

        auto reloaded = reload<GmlFileParser>(save<GMLSaver>(model));
        QVERIFY(reloaded != nullptr);

        QCOMPARE(reloaded->nodeNames(), (QStringList{u"a"_s, u"b"_s}));
        QCOMPARE(reloaded->edgeNames(), QStringList{u"a-b"_s});

        // Each is given a name of its own, in the order of the attributes' names
        QCOMPARE(reloaded->nodeValue(u"GeneName"_s, u"a"_s), u"space a"_s);
        QCOMPARE(reloaded->nodeValue(u"GeneName1"_s, u"b"_s), u"dash b"_s);
        QCOMPARE(reloaded->nodeValue(u"Attribute1st"_s, u"a"_s), u"digit a"_s);
        QCOMPARE(reloaded->nodeValue(u"Attribute1st1"_s, u"b"_s), u"prefixed b"_s);
        QCOMPARE(reloaded->nodeValue(u"label1"_s, u"a"_s), u"label a"_s);
        QCOMPARE(reloaded->nodeValue(u"id1"_s, u"b"_s), u"200"_s);
        QCOMPARE(reloaded->edgeValue(u"source1"_s, u"a"_s, u"b"_s), u"from"_s);
        QCOMPARE(reloaded->edgeValue(u"target1"_s, u"a"_s, u"b"_s), u"to"_s);
    }

    void graphML()
    {
        auto model = createModel();
        auto reloaded = reload<GraphMLParser>(save<GraphMLSaver>(*model));
        QVERIFY(reloaded != nullptr);

        // Nodes are named by their GraphML ids, with the original names as descriptions
        auto names = namesByAttribute(*reloaded, u"Description"_s);
        QStringList descriptions;
        descriptions.reserve(static_cast<qsizetype>(names.size()));
        for(const auto& [description, name] : names)
            descriptions.append(description);

        QCOMPARE(descriptions, _nodes);

        // The names are unique, so are used as the ids
        QCOMPARE(reloaded->nodeNames(), _nodes);

        QStringList edges;
        for(const auto& edge : _edges)
        {
            auto ends = edge.split(u'-');
            edges.append(names.at(ends.at(0)) + u"-"_s + names.at(ends.at(1)));
        }

        edges.sort();
        QCOMPARE(reloaded->edgeNames(), edges);

        QCOMPARE(reloaded->nodeValue(u"Label Text"_s, names.at(A)), u"A & B"_s);
        QCOMPARE(reloaded->nodeValue(u"Label Text"_s, names.at(B)), u"<tag>"_s);
        QCOMPARE(reloaded->nodeValue(u"Label Text"_s, names.at(C)), u"q\"q"_s);
        QCOMPARE(reloaded->nodeValue(u"Label Text"_s, names.at(E)), u"plain"_s);

        QCOMPARE(reloaded->nodeValue(u"Count"_s, names.at(B)), u"-2"_s);
        QCOMPARE(reloaded->nodeValue(u"Count"_s, names.at(D)), u"2000000001"_s);

        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, names.at(A)), 0.123456789));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, names.at(B)), 1e-7));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, names.at(C)), 12345678.0));

        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, names.at(A), names.at(B)), 1.23456789));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, names.at(D), names.at(A)), 7.25));
    }

    void graphMLSharedNames()
    {
        TestGraphModel model({u"a-b"_s, u"b-c"_s});
        model.graphModel().setNodeName(model.nodeId(u"b"_s), u"a"_s);
        model.applyTransforms({});

        auto reloaded = reload<GraphMLParser>(save<GraphMLSaver>(model));
        QVERIFY(reloaded != nullptr);

        // The nodes are given ids of their own, with their names as descriptions
        QCOMPARE(reloaded->graph().numNodes(), 3);
        QCOMPARE(reloaded->graph().numEdges(), 2);

        QStringList descriptions;
        descriptions.reserve(static_cast<qsizetype>(reloaded->graph().numNodes()));
        auto description = reloaded->graphModel().attributeValueByName(u"Description"_s);
        for(auto nodeId : reloaded->graph().nodeIds())
            descriptions.append(description.stringValueOf(nodeId));

        descriptions.sort();
        QCOMPARE(descriptions, (QStringList{u"a"_s, u"a"_s, u"c"_s}));
    }

    void jsonGraph()
    {
        auto model = createModel();
        auto reloaded = reload<JsonGraphParser>(save<JSONGraphSaver>(*model));
        QVERIFY(reloaded != nullptr);

        QCOMPARE(reloaded->nodeNames(), _nodes);
        QCOMPARE(reloaded->edgeNames(), _edges);

        QCOMPARE(reloaded->nodeValue(u"Label Text"_s, A), u"A & B"_s);
        QCOMPARE(reloaded->nodeValue(u"Label Text"_s, B), u"<tag>"_s);
        QCOMPARE(reloaded->nodeValue(u"Label Text"_s, C), u"q\"q"_s);
        QCOMPARE(reloaded->nodeValue(u"Label Text"_s, E), u"plain"_s);

        QCOMPARE(reloaded->nodeValue(u"Count"_s, B), u"-2"_s);
        QCOMPARE(reloaded->nodeValue(u"Count"_s, D), u"2000000001"_s);

        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, A), 0.123456789));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, B), 1e-7));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, C), 12345678.0));

        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, A, B), 1.23456789));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, D, A), 7.25));
    }

    void structuralAttributesAreNotExported()
    {
        auto model = createModel();

        // Outside QCOMPARE, as MSVC wrongly sees escape sequences in raw strings in macro arguments
        const auto transform =
            uR"(AttributeSynthesis using $"Label Text" with Name = "Synthesised" RegularExpression = "^(.*)$" AttributeValue = "\1")"_s;
        QCOMPARE(model->applyTransforms({transform}), QStringList{});

        const std::map<QString, QByteArray> contents
        {
            {u"GML"_s, save<GMLSaver>(*model)},
            {u"GraphML"_s, save<GraphMLSaver>(*model)},
            {u"JSON Graph"_s, save<JSONGraphSaver>(*model)},
        };

        for(const auto& [format, content] : contents)
        {
            QVERIFY2(!content.isEmpty(), qPrintable(format));

            // Importing the file computes these again
            QVERIFY2(!content.contains("Degree"), qPrintable(format));
            QVERIFY2(!content.contains("Multiplicity"), qPrintable(format));
            QVERIFY2(!content.contains("Component"), qPrintable(format));

            // Attributes from the user's data or from transforms still are
            QVERIFY2(content.contains("Count"), qPrintable(format));
            QVERIFY2(content.contains("Synthesised"), qPrintable(format));
        }
    }

    void pairwise()
    {
        auto model = createModel();
        auto content = save<PairwiseSaver>(*model);

        auto reloaded = TestGraphModel::parse(content, [](auto* userNodeData, auto* userEdgeData)
        {
            auto parser = std::make_unique<PairwiseTXTFileParser>(userNodeData, userEdgeData);
            parser->setColumnsConfiguration(
            {
                {0, {._type = PairwiseColumnType::SourceNode, ._name = {}}},
                {1, {._type = PairwiseColumnType::TargetNode, ._name = {}}},
                {2, {._type = PairwiseColumnType::EdgeAttribute, ._name = u"Weight"_s}},
            });

            return parser;
        });

        QVERIFY(reloaded != nullptr);

        // Nodes without edges are saved as rows with one name
        QCOMPARE(reloaded->nodeNames(), _nodes);
        QCOMPARE(reloaded->edgeNames(), _edges);

        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, A, B), 1.23456789));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, B, C), 2.0));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, D, A), 7.25));
    }

    // Which edge attribute, if any, is saved as the edges' weights
    void pairwiseWeights()
    {
        auto contentWith = [](const std::function<void(TestGraphModel&)>& addAttributes)
        {
            TestGraphModel model({u"a-b"_s, u"b-c"_s, u"d"_s});
            addAttributes(model);
            model.applyTransforms({});
            // The file is written as text, so on Windows its lines end with \r\n
            return QString::fromUtf8(save<PairwiseSaver>(model)).remove(u'\r').split(u'\n', Qt::SkipEmptyParts);
        };

        // An attribute named as a weight, before one named as a value, then by name
        auto content = contentWith([](TestGraphModel& model)
        {
            model.addEdgeAttribute<double>(u"Score"_s, {{{u"a"_s, u"b"_s}, 1.0}, {{u"b"_s, u"c"_s}, 2.0}});
            model.addEdgeAttribute<double>(u"Value"_s, {{{u"a"_s, u"b"_s}, 3.0}, {{u"b"_s, u"c"_s}, 4.0}});
            model.addEdgeAttribute<double>(u"edge weight"_s, {{{u"a"_s, u"b"_s}, 5.0}, {{u"b"_s, u"c"_s}, 6.0}});
            model.addEdgeAttribute<double>(u"Weight B"_s, {{{u"a"_s, u"b"_s}, 7.5}, {{u"b"_s, u"c"_s}, 8.0}});
            model.addEdgeAttribute<QString>(u"Weight Name"_s, {{{u"a"_s, u"b"_s}, u"x"_s}, {{u"b"_s, u"c"_s}, u"y"_s}});
        });
        QCOMPARE(content, (QStringList{u"\"a\" \"b\" 7.5"_s, u"\"b\" \"c\" 8"_s, u"\"d\""_s}));

        content = contentWith([](TestGraphModel& model)
        {
            model.addEdgeAttribute<double>(u"Score"_s, {{{u"a"_s, u"b"_s}, 1.0}, {{u"b"_s, u"c"_s}, 2.0}});
            model.addEdgeAttribute<int>(u"P Value"_s, {{{u"a"_s, u"b"_s}, 3}, {{u"b"_s, u"c"_s}, 4}});
            model.addEdgeAttribute<double>(u"Value"_s, {{{u"a"_s, u"b"_s}, 0.25}, {{u"b"_s, u"c"_s}, 0.5}});
        });
        QCOMPARE(content, (QStringList{u"\"a\" \"b\" 3"_s, u"\"b\" \"c\" 4"_s, u"\"d\""_s}));

        // Without one, only the names are saved
        content = contentWith([](TestGraphModel& model)
        {
            model.addEdgeAttribute<double>(u"Score"_s, {{{u"a"_s, u"b"_s}, 1.0}, {{u"b"_s, u"c"_s}, 2.0}});
        });
        QCOMPARE(content, (QStringList{u"\"a\" \"b\""_s, u"\"b\" \"c\""_s, u"\"d\""_s}));
    }

    // Numbers that are missing stay missing, rather than becoming 0, in every format
    void missingNumbers()
    {
        auto model = reload<GmlFileParser>("graph [ "
            "node [ id 0 label \"a\" count 3 score 1.5 ] node [ id 1 label \"b\" ] node [ id 2 label \"c\" ] "
            "edge [ source 0 target 1 weight 0.5 kind \"x\" ] edge [ source 1 target 2 kind \"y\" ] ]");
        QVERIFY(model != nullptr);
        QVERIFY(!model->nodeHasValue(u"count"_s, u"b"_s));
        QVERIFY(!model->edgeHasValue(u"weight"_s, u"b"_s, u"c"_s));

        auto check = [](const QString& format, const TestGraphModel& reloaded,
            const std::map<QString, QString>& names, bool withNodeAttributes)
        {
            if(withNodeAttributes)
            {
                QVERIFY2(reloaded.nodeHasValue(u"count"_s, names.at(u"a"_s)), qPrintable(format));
                QVERIFY2(!reloaded.nodeHasValue(u"count"_s, names.at(u"b"_s)), qPrintable(format));
                QVERIFY2(reloaded.nodeHasValue(u"score"_s, names.at(u"a"_s)), qPrintable(format));
                QVERIFY2(!reloaded.nodeHasValue(u"score"_s, names.at(u"b"_s)), qPrintable(format));
            }

            QVERIFY2(exactlyEqual(reloaded.edgeNumericValue(u"weight"_s, names.at(u"a"_s), names.at(u"b"_s)), 0.5),
                qPrintable(format));
            QVERIFY2(!reloaded.edgeHasValue(u"weight"_s, names.at(u"b"_s), names.at(u"c"_s)), qPrintable(format));
        };

        const std::map<QString, QString> sameNames{{u"a"_s, u"a"_s}, {u"b"_s, u"b"_s}, {u"c"_s, u"c"_s}};

        auto gml = reload<GmlFileParser>(save<GMLSaver>(*model));
        QVERIFY(gml != nullptr);
        check(u"GML"_s, *gml, sameNames, true);

        auto graphML = reload<GraphMLParser>(save<GraphMLSaver>(*model));
        QVERIFY(graphML != nullptr);
        check(u"GraphML"_s, *graphML, namesByAttribute(*graphML, u"Description"_s), true);

        auto jsonGraph = reload<JsonGraphParser>(save<JSONGraphSaver>(*model));
        QVERIFY(jsonGraph != nullptr);
        check(u"JSON Graph"_s, *jsonGraph, sameNames, true);

        // Pairwise files only have the edges' weights
        auto pairwise = TestGraphModel::parse(save<PairwiseSaver>(*model), [](auto* userNodeData, auto* userEdgeData)
        {
            auto parser = std::make_unique<PairwiseTXTFileParser>(userNodeData, userEdgeData);
            parser->setColumnsConfiguration(
            {
                {0, {._type = PairwiseColumnType::SourceNode, ._name = {}}},
                {1, {._type = PairwiseColumnType::TargetNode, ._name = {}}},
                {2, {._type = PairwiseColumnType::EdgeAttribute, ._name = u"weight"_s}},
            });

            return parser;
        });
        QVERIFY(pairwise != nullptr);
        check(u"Pairwise"_s, *pairwise, sameNames, false);
    }
};

QTEST_MAIN(TestSavers)
#include "tst_savers.moc"
