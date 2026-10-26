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

#include "app/application.h"
#include "app/graph/graphmodel.h"
#include "app/transform/graphtransform.h"
#include "app/ui/visualisations/visualisationchannel.h"

#include "shared/loading/gmlfileparser.h"

#include <QString>
#include <QStringList>
#include <QTest>

#include <algorithm>
#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// Two components, a -> b -> c -> a and d -> e, with node and edge attributes of each type
const QByteArray Gml = R"(graph
[
    directed 1
    node [ id 0 label "a" score 0.5 count 1 kind "x" ]
    node [ id 1 label "b" score 1.5 count 2 kind "x" ]
    node [ id 2 label "c" score 2.5 count 3 kind "y" ]
    node [ id 3 label "d" score 3.5 count 4 kind "y" ]
    node [ id 4 label "e" score 4.5 count 5 kind "z" ]
    edge [ source 0 target 1 weight 0.5 rank 1 link "p" ]
    edge [ source 1 target 2 weight 1.5 rank 2 link "p" ]
    edge [ source 2 target 0 weight 2.5 rank 3 link "q" ]
    edge [ source 3 target 4 weight 3.5 rank 4 link "q" ]
])";

struct UserAttribute
{
    QString _name;
    ElementType _elementType;
    ValueType _valueType;
};

const std::vector<UserAttribute> userAttributes
{
    {._name = u"score"_s, ._elementType = ElementType::Node, ._valueType = ValueType::Float},
    {._name = u"count"_s, ._elementType = ElementType::Node, ._valueType = ValueType::Int},
    {._name = u"kind"_s, ._elementType = ElementType::Node, ._valueType = ValueType::String},
    {._name = u"weight"_s, ._elementType = ElementType::Edge, ._valueType = ValueType::Float},
    {._name = u"rank"_s, ._elementType = ElementType::Edge, ._valueType = ValueType::Int},
    {._name = u"link"_s, ._elementType = ElementType::Edge, ._valueType = ValueType::String},
};

template<typename T>
bool overlaps(T a, T b) { return (static_cast<int>(a) & static_cast<int>(b)) != 0; }

const GraphTransformFactory* factoryFor(const QString& name)
{
    const auto& factories = TestGraphModel::application()->transformFactories();
    auto it = factories.find(name);
    return it != factories.end() ? it->second.get() : nullptr;
}

// A config for the transform, with the first of the user's attributes that fits each attribute
// parameter, a condition that's true for some elements if it needs one, and default parameters
QString configFor(const QString& name, const GraphTransformFactory& factory)
{
    auto config = u"\"%1\""_s.arg(name);

    const auto attributeParameters = factory.attributeParameters();
    if(!attributeParameters.empty())
    {
        config += u" using"_s;

        for(const auto& attributeParameter : attributeParameters)
        {
            auto it = std::ranges::find_if(userAttributes, [&](const auto& userAttribute)
            {
                return overlaps(userAttribute._elementType, attributeParameter.elementType()) &&
                    overlaps(userAttribute._valueType, attributeParameter.valueType());
            });

            if(it != userAttributes.end())
                config += u" $\"%1\""_s.arg(it->_name);
        }
    }

    if(factory.requiresCondition())
    {
        switch(factory.elementType())
        {
        case ElementType::Edge:         config += uR"( where $"weight" > 1)"_s; break;
        case ElementType::Component:    config += uR"( where $"ComponentSize" > 2)"_s; break;
        default:                        config += uR"( where $"count" > 1)"_s; break;
        }
    }

    return config;
}
} // namespace

// What every transform says of itself: what the UI shows, the parameters it takes,
// and the attributes it creates and visualises by default
class TestTransformFactories : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        QVERIFY(!TestGraphModel::application()->transformFactories().empty());
    }

    void factories_data()
    {
        QTest::addColumn<QString>("name");

        for(const auto& [name, factory] : TestGraphModel::application()->transformFactories())
            QTest::newRow(qPrintable(name)) << name;
    }

    // What the UI shows of each transform and its parameters
    void factories()
    {
        QFETCH(const QString, name);
        const auto* factory = factoryFor(name);
        if(factory == nullptr)
            QFAIL(qPrintable(u"There's no %1 transform"_s.arg(name)));

        QVERIFY(!factory->displayName().isEmpty());
        QVERIFY(!factory->description().isEmpty());
        QVERIFY(!factory->category().isEmpty());

        for(const auto& parameter : factory->parameters())
        {
            QVERIFY(!parameter.name().isEmpty());
            QVERIFY2(!parameter.description().isEmpty(), qPrintable(parameter.name()));
            QVERIFY2(parameter.initialValue().isValid(), qPrintable(parameter.name()));

            const auto initialValue = parameter.initialValue();

            switch(parameter.type())
            {
            case ValueType::Int:
            case ValueType::Float:
            {
                bool isNumber = false;
                auto value = initialValue.toDouble(&isNumber);
                QVERIFY2(isNumber, qPrintable(parameter.name()));

                if(parameter.hasMin())
                    QVERIFY2(value >= parameter.min(), qPrintable(parameter.name()));

                if(parameter.hasMax())
                    QVERIFY2(value <= parameter.max(), qPrintable(parameter.name()));

                break;
            }

            case ValueType::StringList:
            {
                // The choices, and as many names to show them by, if there are any
                const auto choices = initialValue.toStringList();
                QVERIFY2(!choices.isEmpty(), qPrintable(parameter.name()));

                if(!parameter.displayValues().isEmpty())
                    QCOMPARE(parameter.displayValues().size(), choices.size());

                break;
            }

            case ValueType::String:
                break;

            default:
                QFAIL(qPrintable(u"%1 has no type"_s.arg(parameter.name())));
            }
        }

        for(const auto& attributeParameter : factory->attributeParameters())
        {
            QVERIFY(!attributeParameter.name().isEmpty());
            QVERIFY2(!attributeParameter.description().isEmpty(), qPrintable(attributeParameter.name()));
        }

        // Each default visualisation is of a channel that can show its attribute's type
        for(const auto& defaultVisualisation : factory->defaultVisualisations())
        {
            const auto* channel = TestGraphModel::application()->visualisationChannelFactory(defaultVisualisation._channel);
            if(channel == nullptr)
                QFAIL(qPrintable(u"There's no %1 channel"_s.arg(defaultVisualisation._channel)));
            QVERIFY2(channel->supports(defaultVisualisation._attributeValueType), qPrintable(defaultVisualisation._channel));
            QVERIFY(channel->appliesTo(defaultVisualisation._elementType));
        }
    }

    void applied_data() { factories_data(); }

    // Applied with its parameters' defaults, each transform creates the attributes it declares,
    // and those it visualises by default, of the element and value types it says they are
    void applied()
    {
        QFETCH(const QString, name);
        const auto* factory = factoryFor(name);
        if(factory == nullptr)
            QFAIL(qPrintable(u"There's no %1 transform"_s.arg(name)));

        auto model = TestGraphModel::parse(Gml, [](auto* userNodeData, auto* userEdgeData)
            { return std::make_unique<GmlFileParser>(userNodeData, userEdgeData); });
        QVERIFY(model != nullptr);

        const auto config = configFor(name, *factory);
        const auto alerts = model->applyTransforms({config});
        QVERIFY2(alerts.isEmpty(), qPrintable(config + u": "_s + alerts.join(u"; "_s)));

        const auto& graphModel = model->graphModel();

        for(const auto& declaration : factory->attributeDeclarations())
        {
            QVERIFY2(graphModel.attributeByName(declaration._name) != nullptr,
                qPrintable(u"%1 doesn't exist"_s.arg(declaration._name)));
        }

        for(const auto& defaultVisualisation : factory->defaultVisualisations())
        {
            // The attribute is either named, or given by a parameter
            auto attributeName = defaultVisualisation._attributeName;
            const auto parameter = factory->parameter(attributeName);
            if(!parameter.name().isEmpty())
                attributeName = parameter.initialValue().toString();

            const auto* attribute = graphModel.attributeByName(attributeName);
            if(attribute == nullptr)
                QFAIL(qPrintable(u"%1 doesn't exist"_s.arg(attributeName)));
            QCOMPARE(attribute->elementType(), defaultVisualisation._elementType);
            QCOMPARE(attribute->valueType(), defaultVisualisation._attributeValueType);
        }
    }
};

QTEST_MAIN(TestTransformFactories)
#include "tst_transformfactories.moc"
