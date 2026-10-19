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
            "Rank %1":          {valueType: ValueType.Float}
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
        function writtenString(string) { return testHelper.writtenString(string); }
        function writtenAttributeName(attributeName) { return testHelper.writtenAttributeName(attributeName); }
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
    }

    Item { id: container }

    function cleanup()
    {
        for(let i = container.children.length - 1; i >= 0; i--)
            container.children[i].destroy();
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
        let expanded = template.replace(/%(\d+)/g, function(match, index)
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
        config.toComponents(document, container, {textColor: "black"}, false, null);

        let expanded = expand(config.template, config.flags, config.parameters);
        verify(testHelper.graphTransformsAreEquivalent(expanded, data.transform),
            "Expanded to " + expanded);
    }

    function labelText()
    {
        let texts = [];
        for(let i = 0; i < container.children.length; i++)
            texts.push(container.children[i].text);

        return texts.join("|");
    }

    // A locked transform is displayed as text, rather than as controls
    function test_lockedDisplay_data()
    {
        return [
            {tag: "terminal condition",
                transform: "RemoveComponents where $ComponentSize <= 1",
                expected: "Remove Components where Component Size ≤ 1"},
            {tag: "parameters",
                transform: "MCLCluster with Granularity = 2.2 k = 3",
                expected: "MCL Cluster with Granularity = 2.2 k = 3"},
            {tag: "string list parameter",
                transform: "KNN with RankOrder = Ascending",
                expected: "k-NN with Rank Order = Lowest First"},
            {tag: "string",
                transform: "RemoveNodes where $NodeName includes \"abc\"",
                expected: "Remove Nodes where Node Name Includes \"abc\""},
            {tag: "string with quotes and backslashes",
                transform: "RemoveNodes where $NodeName == \"a \\\"b\\\" \\\\ c\"",
                expected: "Remove Nodes where Node Name = \"a \"b\" \\ c\""}
        ];
    }

    function test_lockedDisplay(data)
    {
        verify(testHelper.graphTransformIsValid(data.transform));

        let config = new TransformConfig.Create(0, document, data.transform);
        config.toComponents(document, container, {textColor: "black"}, true, null);

        compare(labelText(), data.expected);
    }
}
