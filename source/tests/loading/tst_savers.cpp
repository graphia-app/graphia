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
const QString C = u"c"_s;
const QString D = u"d"_s;
const QString E = u"e"_s;

// The graph that is saved in each format
std::unique_ptr<TestGraphModel> createModel()
{
    auto model = std::make_unique<TestGraphModel>(QStringList{A + u"-"_s + B, B + u"-"_s + C,
        C + u"-"_s + D, D + u"-"_s + A, E});

    model->addNodeAttribute<QString>(u"Label Text"_s,
        {{A, u"A & B"_s}, {B, u"<tag>"_s}, {C, u"q\"q"_s}, {D, u""_s}, {E, u"plain"_s}});
    model->addNodeAttribute<int>(u"Count"_s,
        {{A, 1}, {B, -2}, {C, 3}, {D, 4}, {E, 0}});
    model->addNodeAttribute<double>(u"Score"_s,
        {{A, 0.5}, {B, 1e-7}, {C, 1234.0}, {D, -0.5}, {E, 3.0}});
    model->addEdgeAttribute<double>(u"Weight"_s,
        {{{A, B}, 1.25}, {{B, C}, 2.0}, {{C, D}, 0.1}, {{D, A}, 7.25}});

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
        QCOMPARE(reloaded->nodeValue(u"Node Count"_s, D), u"4"_s);

        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Node Score"_s, A), 0.5));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, B), 1e-7));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Node Score"_s, C), 1234.0));

        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Edge Weight"_s, A, B), 1.25));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, D, A), 7.25));
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
        QCOMPARE(reloaded->nodeValue(u"Count"_s, names.at(D)), u"4"_s);

        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, names.at(A)), 0.5));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, names.at(B)), 1e-7));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, names.at(C)), 1234.0));

        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, names.at(A), names.at(B)), 1.25));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, names.at(D), names.at(A)), 7.25));
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
        QCOMPARE(reloaded->nodeValue(u"Count"_s, D), u"4"_s);

        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, A), 0.5));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, B), 1e-7));
        QVERIFY(exactlyEqual(reloaded->nodeNumericValue(u"Score"_s, C), 1234.0));

        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, A, B), 1.25));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, D, A), 7.25));
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

        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, A, B), 1.25));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, B, C), 2.0));
        QVERIFY(exactlyEqual(reloaded->edgeNumericValue(u"Weight"_s, D, A), 7.25));
    }
};

QTEST_MAIN(TestSavers)
#include "tst_savers.moc"
