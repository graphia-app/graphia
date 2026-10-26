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
#include "app/graph/graphmodel.h"
#include "app/preferences.h"
#include "app/ui/visualisations/colorvisualisationchannel.h"
#include "app/ui/visualisations/elementvisual.h"
#include "app/ui/visualisations/textcolorvisualisationchannel.h"
#include "app/ui/visualisations/textvisual.h"

#include "shared/utils/string.h"

#include <QTest>
#include <QColor>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <array>
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
// Each group of nodes labelled with shared text, as its text and the sorted names of its nodes
QStringList sharedTextOf(TestGraphModel& model)
{
    QStringList groups;

    model.graphModel().forEachTextVisual([&model, &groups](const TextVisual& textVisual)
    {
        QStringList names;
        names.reserve(static_cast<qsizetype>(textVisual._nodeIds.size()));
        for(auto nodeId : textVisual._nodeIds)
            names.append(model.graphModel().nodeName(nodeId));

        names.sort();
        groups.append(u"%1: %2"_s.arg(textVisual._text, names.join(u' ')));
    });

    groups.sort();
    return groups;
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

    // Inverting reverses the mapping, about the middle of its range
    void invertedStandardDeviationMapping()
    {
        auto model = createModel();
        visualise(*model, {uR"([invert] "score" "Colour" with )"_s + BlackToWhite + u" "_s +
            parameter(u"mapping"_s, u"{\"type\": \"stddev\"}"_s)});

        QCOMPARE(gradientPositionOf(*model, u"a"_s), 1.0);
        QVERIFY(std::abs(gradientPositionOf(*model, u"b"_s) - 0.75) < 0.01);
        QVERIFY(std::abs(gradientPositionOf(*model, u"e"_s) - 0.5) < 0.01);
        QCOMPARE(gradientPositionOf(*model, u"g"_s), 0.0);
        QCOMPARE(gradientPositionOf(*model, u"h"_s), 0.0);
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

    // The colour of the text, but not of the node
    void textColour()
    {
        auto model = createModel();
        visualise(*model, {});
        const auto defaultNodeColour = visualOf(*model, u"a"_s)._outerColor;
        const auto defaultTextColour = visualOf(*model, u"a"_s)._textColor;

        visualise(*model, {uR"("score" "TextColour" with )"_s + BlackToWhite});

        QCOMPARE(visualOf(*model, u"a"_s)._textColor.name(), u"#000000"_s);
        QCOMPARE(visualOf(*model, u"h"_s)._textColor.name(), u"#ffffff"_s);
        QVERIFY(std::abs(static_cast<double>(visualOf(*model, u"e"_s)._textColor.redF()) - (3.0 / 7.0)) < 0.01);
        QCOMPARE(visualOf(*model, u"a"_s)._outerColor, defaultNodeColour);

        visualise(*model, {uR"("kind" "TextColour" with )"_s + RedGreenBlue});

        // Natural order: Thing 1, Thing 2, Thing 10
        QCOMPARE(visualOf(*model, u"d"_s)._textColor.name(), u"#ff0000"_s);
        QCOMPARE(visualOf(*model, u"b"_s)._textColor.name(), u"#00ff00"_s);
        QCOMPARE(visualOf(*model, u"a"_s)._textColor.name(), u"#0000ff"_s);
        QCOMPARE(visualOf(*model, u"a"_s)._outerColor, defaultNodeColour);

        visualise(*model, {});
        QCOMPARE(visualOf(*model, u"a"_s)._textColor, defaultTextColour);
    }

    void textSize()
    {
        auto model = createModel();
        visualise(*model, {});
        const auto defaultTextSize = visualOf(*model, u"a"_s)._textSize;
        QVERIFY(defaultTextSize > 0.0f);

        visualise(*model, {uR"("score" "TextSize")"_s});

        auto textSizeOf = [&](const QString& node) { return visualOf(*model, node)._textSize; };

        QVERIFY(textSizeOf(u"a"_s) < textSizeOf(u"b"_s));
        QCOMPARE(textSizeOf(u"b"_s), textSizeOf(u"c"_s));
        QVERIFY(textSizeOf(u"b"_s) < textSizeOf(u"e"_s));
        QVERIFY(textSizeOf(u"e"_s) < textSizeOf(u"h"_s));

        // The default is between the smallest and the largest
        QVERIFY(textSizeOf(u"a"_s) < defaultTextSize);
        QVERIFY(textSizeOf(u"h"_s) > defaultTextSize);

        // The node's size is unaffected
        QCOMPARE(visualOf(*model, u"a"_s)._size, visualOf(*model, u"h"_s)._size);
    }

    // What the visualisations panel is given for a new colour visualisation
    void colourDefaults()
    {
        const ColorVisualisationChannelFactory colour;
        const TextColorVisualisationChannelFactory textColour;

        const std::array<const VisualisationChannelFactory*, 2> factories{&colour, &textColour};

        for(const auto* factory : factories)
        {
            for(auto valueType : {ValueType::Int, ValueType::Float})
            {
                QCOMPARE(factory->defaultParameters(valueType),
                    (QVariantMap{{u"gradient"_s, u::pref(u"visuals/defaultGradient"_s).toString()}}));
                QVERIFY(factory->description(ElementType::Node, valueType).contains(u"gradient"_s));
            }

            QCOMPARE(factory->defaultParameters(ValueType::String),
                (QVariantMap{{u"palette"_s, u::pref(u"visuals/defaultPalette"_s).toString()}}));
            QVERIFY(factory->description(ElementType::Edge, ValueType::String).contains(u"edge"_s));

            QVERIFY(factory->defaultParameters(ValueType::Unknown).isEmpty());
            QVERIFY(factory->description(ElementType::Node, ValueType::Unknown).isEmpty());
        }

        QVERIFY(textColour.description(ElementType::Node, ValueType::Float).contains(u"text"_s));
        QVERIFY(!colour.description(ElementType::Node, ValueType::Float).contains(u"text"_s));
    }

    // What the dialog for creating a visualisation asks of each channel
    void channelDescriptions()
    {
        auto model = createModel();
        const auto& graphModel = model->graphModel();

        // Text and Shared Text show values as they are, so they can't be mapped
        QVERIFY(graphModel.visualisationChannelAllowsMapping(u"Colour"_s));
        QVERIFY(graphModel.visualisationChannelAllowsMapping(u"Size"_s));
        QVERIFY(graphModel.visualisationChannelAllowsMapping(u"TextColour"_s));
        QVERIFY(graphModel.visualisationChannelAllowsMapping(u"TextSize"_s));
        QVERIFY(!graphModel.visualisationChannelAllowsMapping(u"Text"_s));
        QVERIFY(!graphModel.visualisationChannelAllowsMapping(u"SharedText"_s));
        QVERIFY(!graphModel.visualisationChannelAllowsMapping(u"Unknown"_s));
        QVERIFY(!graphModel.visualisationChannelAllowsMapping({}));

        // A description for each channel, which says when it can't show the attribute
        auto descriptions = graphModel.visualisationDescription(u"kind"_s,
            {u"Colour"_s, u"Size"_s, u"Text"_s, u"SharedText"_s});
        QCOMPARE(descriptions.size(), 4);
        QCOMPARE(descriptions.at(0), u"This visualisation will be applied by assigning a colour "
            "to the node which represents unique values of the attribute."_s);
        QCOMPARE(descriptions.at(1), u"This visualisation channel is not supported for the attribute type."_s);
        QCOMPARE(descriptions.at(2), u"The attribute will be visualised as text."_s);
        QCOMPARE(descriptions.at(3), u"Groups of nodes that share an attribute value will be labelled as such."_s);

        descriptions = graphModel.visualisationDescription(u"score"_s,
            {u"Colour"_s, u"Size"_s, u"TextColour"_s, u"TextSize"_s, u"SharedText"_s});
        QCOMPARE(descriptions.size(), 5);
        QCOMPARE(descriptions.at(0), u"The attribute will be visualised by continuously varying "
            "the colour of the node using a gradient."_s);
        QCOMPARE(descriptions.at(1), u"The attribute will be visualised by varying the size of the node."_s);
        QCOMPARE(descriptions.at(2), u"The attribute will be visualised by continuously varying "
            "the colour of the node text using a gradient."_s);
        QCOMPARE(descriptions.at(3), u"The attribute will be visualised by varying the size of the node text."_s);
        QCOMPARE(descriptions.at(4), u"This visualisation channel is not supported for the attribute type."_s);

        QCOMPARE(graphModel.visualisationDescription(u"missing"_s, {u"Colour"_s}), QStringList{});
        QCOMPARE(graphModel.visualisationDescription(u"score"_s, {u"Unknown"_s}), QStringList{});
    }

    void sharedText_data()
    {
        QTest::addColumn<QString>("attributeName");

        QTest::newRow("plain") << u"kind"_s;
        QTest::newRow("space") << u"Gene Type"_s;
        QTest::newRow("quotes") << u"Say \"Hi\""_s;
    }

    // Each group of adjacent nodes with the same value is labelled once
    void sharedText()
    {
        QFETCH(const QString, attributeName);

        auto model = std::make_unique<TestGraphModel>(
            QStringList{u"a-b"_s, u"b-c"_s, u"c-d"_s, u"e-f"_s, u"g"_s});

        model->addNodeAttribute<QString>(attributeName, {{u"a"_s, u"x"_s}, {u"b"_s, u"x"_s},
            {u"c"_s, u"y"_s}, {u"d"_s, u"x"_s}, {u"e"_s, u"x"_s}, {u"f"_s, u"x"_s}, {u"g"_s, u"x"_s}});
        model->applyTransforms({});

        visualise(*model, {u"%1 SharedText"_s.arg(Attribute::writtenAttributeName(attributeName))});

        QCOMPARE(sharedTextOf(*model), (QStringList{u"x: a b"_s, u"x: d"_s, u"x: e f"_s, u"x: g"_s, u"y: c"_s}));

        // The groups are in the components of their nodes
        int numGroups = 0;
        model->graphModel().forEachTextVisual(model->graph().componentIdOfNode(model->nodeId(u"e"_s)),
            [&numGroups](const TextVisual&) { numGroups++; });
        QCOMPARE(numGroups, 1);
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
