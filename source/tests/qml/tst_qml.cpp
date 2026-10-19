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

#include "app/attributes/attribute.h"
#include "app/configgrammar.h"
#include "app/transform/graphtransformconfigparser.h"
#include "app/transform/transformdisplaytext.h"
#include "app/ui/qml/Graphia/availableattributesmodel.h"
#include "app/ui/visualisations/visualisationconfigparser.h"

#include "tests/common/testenvironment.h"
#include "tests/common/testgraphmodel.h"

#include <QtQuickTest/quicktest.h>
#include <QQmlContext>
#include <QQmlEngine>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include <memory>

using namespace Qt::Literals::StringLiterals;

// Runs the QML tests in this directory, which test the JavaScript helpers the
// UI uses; anything they need from C++ that would otherwise come from a
// Document is provided here, as the context property "testHelper"
class Setup : public QObject
{
    Q_OBJECT

    // A graph with a few attributes, for availableAttributesModel
    std::unique_ptr<TestGraphModel> _graphModel;

public:
    Setup() = default;

    // As Document::parseGraphTransform
    Q_INVOKABLE QVariantMap parseGraphTransform(const QString& transform) const
    {
        GraphTransformConfigParser p;
        if(p.parse(transform))
            return p.result().asVariantMap(TestGraphModel::application()->transformFactory(p.result()._action));

        return {};
    }

    Q_INVOKABLE bool graphTransformIsValid(const QString& transform) const
    {
        GraphTransformConfigParser p;
        return p.parse(transform);
    }

    Q_INVOKABLE bool graphTransformsAreEquivalent(const QString& a, const QString& b) const
    {
        GraphTransformConfigParser pa;
        GraphTransformConfigParser pb;
        if(!pa.parse(a) || !pb.parse(b))
            return false;

        return pa.result().equals(pb.result(), "ignoreInertFlags"_no);
    }

    // As the Document functions of the same names
    Q_INVOKABLE QString writtenString(const QString& string) const
    {
        return LexyConfigGrammar::writtenString(string);
    }

    Q_INVOKABLE QString writtenAttributeName(const QString& attributeName) const
    {
        return Attribute::writtenAttributeName(attributeName);
    }

    Q_INVOKABLE QVariantMap transformDisplayText() const
    {
        return
        {
            {u"usingAttributes"_s, TransformDisplayText::usingAttributes()},
            {u"withParameters"_s, TransformDisplayText::withParameters()},
            {u"whereCondition"_s, TransformDisplayText::whereCondition()},
            {u"parameter"_s, TransformDisplayText::parameter()}
        };
    }

    Q_INVOKABLE QVariantMap opDisplayText(const QString& op) const
    {
        auto text = GraphTransformConfigParser::opDisplayText(op);
        return {{u"name"_s, text._name}, {u"phrase"_s, text._phrase}};
    }

    // Of the attributes of _graphModel
    Q_INVOKABLE QString attributeDisplayName(const QString& attributeName) const
    {
        return _graphModel->graphModel().attributeDisplayName(attributeName);
    }

    // As Document::parseVisualisation
    Q_INVOKABLE QVariantMap parseVisualisation(const QString& visualisation) const
    {
        VisualisationConfigParser p;
        if(p.parse(visualisation))
            return p.result().asVariantMap();

        return {};
    }

    // As Document::availableAttributesModel, but with the attributes of _graphModel, which
    // are those it declares, such as NodeName, along with those added in applicationAvailable
    Q_INVOKABLE AvailableAttributesModel* availableAttributesModel(int elementTypes, int valueTypes,
        int skipFlags, const QStringList& skipAttributeNames) const
    {
        // The caller takes ownership
        return new AvailableAttributesModel(_graphModel->graphModel(), nullptr,
            static_cast<ElementType>(elementTypes), static_cast<ValueType>(valueTypes),
            static_cast<AttributeFlag>(skipFlags), skipAttributeNames);
    }

public slots:
    void applicationAvailable()
    {
        TestEnvironment::initialise();

        _graphModel = std::make_unique<TestGraphModel>(QStringList{u"a-b"_s});
        // As loaded by a plugin, which declares the attribute's display name
        _graphModel->addNodeAttribute<QString>(u"NodeName"_s, {{u"a"_s, u"a"_s}, {u"b"_s, u"b"_s}});
        _graphModel->graphModel().attributeByName(u"NodeName"_s)->setDisplayName(u"Node Name"_s);
        _graphModel->addNodeAttribute<double>(u"Score"_s, {{u"a"_s, 1.0}, {u"b"_s, 2.0}});
        _graphModel->addNodeAttribute<double>(u"Weight"_s, {{u"a"_s, 1.0}, {u"b"_s, 2.0}});
        _graphModel->addNodeAttribute<double>(u"Rank %1"_s, {{u"a"_s, 1.0}, {u"b"_s, 2.0}});
        _graphModel->addNodeAttribute<QString>(u"A\\B"_s, {{u"a"_s, u"x"_s}, {u"b"_s, u"y"_s}});
        _graphModel->addEdgeAttribute<double>(u"EdgeWeight"_s, {{{u"a"_s, u"b"_s}, 1.0}});
    }

    void qmlEngineAvailable(QQmlEngine* engine)
    {
        engine->rootContext()->setContextProperty(u"testHelper"_s, this);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(graphia, Setup)
#include "tst_qml.moc"
