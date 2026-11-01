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

import QtQuick
import QtTest

import Graphia
import Graphia.SharedTypes

TestCase
{
    name: "TransformConfig"

    // Stands in for Document, providing what TransformConfig uses
    QtObject
    {
        id: document

        property var attributes:
        ({
            "ComponentSize":    {valueType: ValueType.Int},
            "NodeDegree":       {valueType: ValueType.Int},
            "NodeName":         {valueType: ValueType.String},
            "Weight":           {valueType: ValueType.Float},
            "Score":            {valueType: ValueType.Float},
            "EdgeWeight":       {valueType: ValueType.Float, elementType: ElementType.Edge},
            "source.Score":     {valueType: ValueType.Float, elementType: ElementType.Edge},
            "Rank %1":          {valueType: ValueType.Float},
            "A\\B":             {valueType: ValueType.String}
        })

        property var parameters:
        ({
            "k":                {valueType: ValueType.Int, displayName: "k"},
            "Granularity":      {valueType: ValueType.Float, displayName: "Granularity"},
            "RankOrder":        {valueType: ValueType.StringList, displayName: "Rank Order",
                                    initialValue: ["Descending", "Ascending"],
                                    displayValues: ["Highest First", "Lowest First"]},
            "Name":             {valueType: ValueType.String, displayName: "Name"}
        })

        function parseGraphTransform(transform) { return testHelper.parseGraphTransform(transform); }
        function stringForExpression(string) { return testHelper.stringForExpression(string); }
        function attributeNameForExpression(attributeName) { return testHelper.attributeNameForExpression(attributeName); }
        function transformDisplayText() { return testHelper.transformDisplayText(); }
        function opDisplayText(op) { return testHelper.opDisplayText(op); }
        function attributeDisplayName(attributeName) { return testHelper.attributeDisplayName(attributeName); }

        // Values are shown as they're stored
        function attributeDisplayValue(attributeName, value) { return value; }
        function attributeStoredValue(attributeName, displayValue) { return displayValue; }

        function attribute(attributeName)
        {
            attributeName = attributeName.replace(/"/g, "");

            let a = Object.assign({elementType: ElementType.Node, isValid: true,
                hasRange: false, hasMinimumValue: false, hasMaximumValue: false},
                attributes[attributeName]);

            return a;
        }

        function transformParameter(transformName, parameterName)
        {
            let p = Object.assign({hasRange: false, hasMinimumValue: false,
                hasMaximumValue: false}, parameters[parameterName]);

            // Copy any lists, so changes aren't shared between tests
            if(Array.isArray(p.initialValue))
                p.initialValue = p.initialValue.slice();

            return p;
        }

        function addedOrChangedAttributeNamesAtTransformIndexOrLater(index) { return []; }

        function availableAttributesModel(elementTypes, valueTypes, skipFlags, skipAttributeNames)
        {
            return testHelper.availableAttributesModel(elementTypes, valueTypes,
                skipFlags, skipAttributeNames);
        }
    }

    Item { id: container }

    // The controls created for a transform; those for a locked transform have no
    // parent, so are destroyed here rather than left to the garbage collector,
    // which can destroy them in any order, causing spurious warnings
    property var parameters: []

    function toComponents(config, locked)
    {
        config.toComponents(document, container, {textColor: "black"}, locked, null);
        parameters = config.parameters;
    }

    function cleanup()
    {
        for(let parameter of parameters)
            parameter.destroy();

        parameters = [];

        for(let i = container.children.length - 1; i >= 0; i--)
            container.children[i].destroy();

        wait(0);
    }

    function test_create()
    {
        let config = new TransformConfig.Create(0, document,
            "[locked, pinned] MCLCluster with Granularity = 2.2 where $NodeDegree > 3");

        compare(config.action, "MCLCluster");
        compare(config.flags, ["locked", "pinned"]);
        compare(config.template, "MCLCluster with Granularity = %1 where $NodeDegree > %2");
    }

    // As Transform.updateExpression
    function expand(template, flags, parameterComponents)
    {
        let expanded = template.replace(/%(\d+)/g, (match, index) =>
        {
            verify(index <= parameterComponents.length, "Template argument index out of range");
            return parameterComponents[index - 1].value;
        });

        expanded = expanded.replace(/%\!/g, "%");

        if(flags.length > 0)
            expanded = "[" + flags.toString() + "] " + expanded;

        return expanded;
    }

    // Building the parameter components for a transform, then building the
    // expression from them without changing anything, should give the same
    // transform; if it doesn't, merely opening the transform in the UI and
    // editing one of its values changes or breaks the rest of it
    function test_unchangedExpansion_data()
    {
        return [
            {tag: "terminal condition",
                transform: "RemoveComponents where $ComponentSize <= 1"},
            {tag: "flags",
                transform: "[pinned, disabled] RemoveComponents where $ComponentSize <= 1"},
            {tag: "quoted names",
                transform: "\"RemoveComponents\" where $\"ComponentSize\" <= 1"},
            {tag: "int parameter",
                transform: "KNN with k = 5"},
            {tag: "attribute parameter",
                transform: "KNN using $EdgeWeight with k = 5"},
            {tag: "node attribute parameters",
                transform: "\"Some Transform\" using $Score $NodeName"},
            {tag: "source attribute parameter",
                transform: "\"Some Transform\" using $source.Score"},
            {tag: "float parameter",
                transform: "MCLCluster with Granularity = 2.2"},
            {tag: "string list parameter",
                transform: "KNN with RankOrder = Ascending"},
            {tag: "string parameter with quotes",
                transform: "\"Some Transform\" with Name = \"a \\\"b\\\"\""},
            {tag: "several parameters",
                transform: "MCLCluster with Granularity = 2.2 k = 3"},
            {tag: "compound condition",
                transform: "RemoveNodes where ($NodeDegree > 3 and " +
                    "$NodeName includes \"abc\") or $Weight hasValue"},
            {tag: "attribute on both sides",
                transform: "RemoveNodes where $NodeDegree > $ComponentSize"},
            {tag: "condition value with quotes",
                transform: "RemoveNodes where $NodeName == \"say \\\"hi\\\"\""},
            {tag: "numeric string",
                transform: "RemoveNodes where $NodeName == \"123\""},
            {tag: "percent in action",
                transform: "\"%-NN\" with k = 5"},
            {tag: "percent and digit in attribute name",
                transform: "RemoveNodes where $\"Rank %1\" > 2.5"}
        ];
    }

    function test_unchangedExpansion(data)
    {
        verify(testHelper.graphTransformIsValid(data.transform));

        let config = new TransformConfig.Create(0, document, data.transform);
        toComponents(config, false);

        let expanded = expand(config.template, config.flags, config.parameters);
        verify(testHelper.graphTransformsAreEquivalent(expanded, data.transform),
            "Expanded to " + expanded);
    }

    function labelText()
    {
        return Array.from(container.children, child => child.text).join("|");
    }

    // A locked transform is displayed as text, rather than as controls
    function test_lockedDisplay_data()
    {
        // Built as the transform's text is, from the translations of its parts, with a | between
        // the labels it's shown in, each of which is trimmed and has its whitespace collapsed
        let usingAttributes = (text, attributes) => qsTr("%1 using %2").arg(text).arg(attributes);
        let withParameters = (text, parameters) => qsTr("%1 with %2").arg(text).arg(parameters.join(" "));
        let parameter = (name, value) => qsTr("%1 = %2", "a transform parameter's value").arg(name).arg(value);
        let whereCondition = (text, condition) => qsTr("%1 where %2").arg(text).arg(condition);
        let op = (phrase, lhs, rhs) => phrase.arg(lhs).arg(rhs);
        let labels = text => text.split("|").map(label => label.trim().replace(/\s+/g, " ")).join("|");

        return [
            {tag: "terminal condition",
                transform: "RemoveComponents where $ComponentSize <= 1",
                expected: labels(whereCondition(qsTr("Remove Components"),
                    op(qsTr("%1 ≤ %2"), qsTr("Component Size"), "1")))},
            {tag: "parameters",
                transform: "MCLCluster with Granularity = 2.2 k = 3",
                expected: labels(withParameters(qsTr("MCL Cluster"),
                    [parameter("Granularity", "2" + Qt.locale().decimalPoint + "2"), parameter("k", "3")]))},
            {tag: "string list parameter",
                transform: "KNN with RankOrder = Ascending",
                expected: labels(withParameters(qsTr("k-NN"), [parameter("Rank Order", "Lowest First")]))},
            {tag: "attribute parameters",
                transform: "\"Some Transform\" using $Score $source.Score with k = 3",
                expected: labels(withParameters(usingAttributes("Some Transform",
                    "Score|" + qsTr("Source › %1").arg("Score") + "|"), [parameter("k", "3")]))},
            {tag: "string",
                transform: "RemoveNodes where $NodeName includes \"abc\"",
                expected: labels(whereCondition(qsTr("Remove Nodes"),
                    op(qsTr("%1 Includes %2"), "Node Name", "\"abc\"")))},
            {tag: "string with quotes and backslashes",
                transform: "RemoveNodes where $NodeName == \"a \\\"b\\\" \\\\ c\"",
                expected: labels(whereCondition(qsTr("Remove Nodes"),
                    op(qsTr("%1 = %2", "a condition"), "Node Name", "\"a \"b\" \\ c\"")))},
            {tag: "string starting with $",
                transform: "RemoveNodes where $NodeName == \"$HOME\"",
                expected: labels(whereCondition(qsTr("Remove Nodes"),
                    op(qsTr("%1 = %2", "a condition"), "Node Name", "\"$HOME\"")))},
            {tag: "backslash in attribute name",
                transform: "RemoveNodes where $\"A\\B\" == \"x\"",
                expected: labels(whereCondition(qsTr("Remove Nodes"),
                    op(qsTr("%1 = %2", "a condition"), "A\\B", "\"x\"")))}
        ];
    }

    function test_lockedDisplay(data)
    {
        verify(testHelper.graphTransformIsValid(data.transform));

        let config = new TransformConfig.Create(0, document, data.transform);
        toComponents(config, true);

        compare(labelText(), data.expected);
    }

    function test_addLabelTo_data()
    {
        return [
            {tag: "plain", text: "Invalid Expression", expected: "Invalid Expression"},
            {tag: "whitespace", text: " a \n b ", expected: "a b"},
            {tag: "quotes", text: "Attribute \"x\" not found", expected: "Attribute \"x\" not found"},
            {tag: "backslashes", text: "a \\u b \\", expected: "a \\u b \\"}
        ];
    }

    function test_addLabelTo(data)
    {
        TransformConfig.addLabelTo(data.text, container, {textColor: "black"});
        compare(labelText(), data.expected);
    }
}
