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

#include "app/graph/graphmodel.h"
#include "app/ui/visualisations/elementvisual.h"

#include "shared/utils/string.h"

#include <QTest>
#include <QColor>
#include <QString>
#include <QStringList>

#include <map>
#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
// Isolated nodes, whose scores have a mean of 5 and standard deviation of 2
std::unique_ptr<TestGraphModel> createModel()
{
    auto model = std::make_unique<TestGraphModel>(
        QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s, u"f"_s, u"g"_s, u"h"_s});

    model->addNodeAttribute<double>(u"score"_s, {{u"a"_s, 2.0}, {u"b"_s, 4.0}, {u"c"_s, 4.0},
        {u"d"_s, 4.0}, {u"e"_s, 5.0}, {u"f"_s, 5.0}, {u"g"_s, 7.0}, {u"h"_s, 9.0}});

    model->addNodeAttribute<QString>(u"kind"_s, {{u"a"_s, u"Thing 10"_s}, {u"b"_s, u"Thing 2"_s},
        {u"c"_s, u"Thing 2"_s}, {u"d"_s, u"Thing 1"_s}, {u"e"_s, u"Thing 1"_s}, {u"f"_s, u"Thing 1"_s},
        {u"g"_s, u"Thing 10"_s}, {u"h"_s, u"Thing 10"_s}});

    model->applyTransforms({});

    return model;
}

QString parameter(const QString& name, const QString& value)
{
    return u"%1 = \"%2\""_s.arg(name, u::escapeQuotes(value));
}

// Black for the lowest value, white for the highest
const QString BlackToWhite = parameter(u"gradient"_s, u"{\"0\": \"#000000\", \"1\": \"#ffffff\"}"_s);

const QString RedGreenBlue = parameter(u"palette"_s,
    u"{\"autoColors\": [\"#ff0000\", \"#00ff00\", \"#0000ff\"]}"_s);

const ElementVisual& visualOf(TestGraphModel& model, const QString& node)
{
    return model.graphModel().nodeVisual(model.nodeId(node));
}

// For the black to white gradient, the position of the node's colour along it
double gradientPositionOf(TestGraphModel& model, const QString& node)
{
    return visualOf(model, node)._outerColor.redF();
}

// Builds the visualisations, then applies them, as happens after a command
void visualise(TestGraphModel& model, const QStringList& visualisations)
{
    model.graphModel().enableVisualUpdates();
    model.graphModel().buildVisualisations(visualisations);
    model.graphModel().applyPendingVisualUpdates();
}
} // namespace

class TestVisualisations : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void gradient()
    {
        auto model = createModel();
        visualise(*model, {uR"("score" "Colour" with )"_s + BlackToWhite});

        QCOMPARE(gradientPositionOf(*model, u"a"_s), 0.0);
        QCOMPARE(gradientPositionOf(*model, u"h"_s), 1.0);
        QVERIFY(std::abs(gradientPositionOf(*model, u"e"_s) - (3.0 / 7.0)) < 0.01);
    }

    void invert()
    {
        auto model = createModel();
        visualise(*model, {uR"([invert] "score" "Colour" with )"_s + BlackToWhite});

        QCOMPARE(gradientPositionOf(*model, u"a"_s), 1.0);
        QCOMPARE(gradientPositionOf(*model, u"h"_s), 0.0);
        QVERIFY(std::abs(gradientPositionOf(*model, u"e"_s) - (4.0 / 7.0)) < 0.01);
    }

    void standardDeviationMapping()
    {
        auto model = createModel();
        visualise(*model, {uR"("score" "Colour" with )"_s + BlackToWhite + u" "_s +
            parameter(u"mapping"_s, u"{\"type\": \"stddev\"}"_s)});

        // The range is 3 to 7
        QCOMPARE(gradientPositionOf(*model, u"a"_s), 0.0);
        QVERIFY(std::abs(gradientPositionOf(*model, u"b"_s) - 0.25) < 0.01);
        QVERIFY(std::abs(gradientPositionOf(*model, u"e"_s) - 0.5) < 0.01);
        QCOMPARE(gradientPositionOf(*model, u"g"_s), 1.0);
        QCOMPARE(gradientPositionOf(*model, u"h"_s), 1.0);
    }

    void size()
    {
        auto model = createModel();
        visualise(*model, {uR"("score" "Size")"_s});

        auto sizeOf = [&](const QString& node) { return visualOf(*model, node)._size; };

        QVERIFY(sizeOf(u"a"_s) < sizeOf(u"b"_s));
        QCOMPARE(sizeOf(u"b"_s), sizeOf(u"c"_s));
        QVERIFY(sizeOf(u"b"_s) < sizeOf(u"e"_s));
        QVERIFY(sizeOf(u"e"_s) < sizeOf(u"h"_s));
    }

    // String values are given colours in natural order
    void palette()
    {
        auto model = createModel();
        visualise(*model, {uR"("kind" "Colour" with )"_s + RedGreenBlue});

        QCOMPARE(visualOf(*model, u"d"_s)._outerColor.name(), u"#ff0000"_s);
        QCOMPARE(visualOf(*model, u"b"_s)._outerColor.name(), u"#00ff00"_s);
        QCOMPARE(visualOf(*model, u"a"_s)._outerColor.name(), u"#0000ff"_s);
    }

    // Or, most common first
    void paletteByQuantity()
    {
        auto model = createModel();
        visualise(*model, {uR"([assignByQuantity] "kind" "Colour" with )"_s + RedGreenBlue});

        // Thing 1 and Thing 10 each have 3 nodes, and Thing 2 has 2
        QCOMPARE(visualOf(*model, u"d"_s)._outerColor.name(), u"#ff0000"_s);
        QCOMPARE(visualOf(*model, u"a"_s)._outerColor.name(), u"#00ff00"_s);
        QCOMPARE(visualOf(*model, u"b"_s)._outerColor.name(), u"#0000ff"_s);
    }

    void text()
    {
        auto model = createModel();
        visualise(*model, {uR"("kind" "Text")"_s});

        QCOMPARE(visualOf(*model, u"a"_s)._text, u"Thing 10"_s);
        QCOMPARE(visualOf(*model, u"d"_s)._text, u"Thing 1"_s);

        visualise(*model, {uR"("score" "Text")"_s});
        QCOMPARE(visualOf(*model, u"e"_s)._text, u"5"_s);

        // Without a visualisation, a node's text is its name
        visualise(*model, {});
        QCOMPARE(visualOf(*model, u"e"_s)._text, u"e"_s);
    }

    void disabled()
    {
        auto model = createModel();
        visualise(*model, {uR"([disabled] "score" "Colour" with )"_s + BlackToWhite});

        const auto defaultColor = visualOf(*model, u"a"_s)._outerColor;
        QCOMPARE(visualOf(*model, u"h"_s)._outerColor, defaultColor);
    }

    // A later visualisation of the same channel takes precedence
    void overridden()
    {
        auto model = createModel();
        visualise(*model, {uR"("kind" "Colour" with )"_s + RedGreenBlue,
            uR"("score" "Colour" with )"_s + BlackToWhite});

        QCOMPARE(gradientPositionOf(*model, u"h"_s), 1.0);
    }
};

QTEST_MAIN(TestVisualisations)
#include "tst_visualisations.moc"
