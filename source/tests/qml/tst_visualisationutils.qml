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
import Graphia.Utils

TestCase
{
    name: "VisualisationUtils"

    // Stands in for Document, providing what VisualisationUtils uses
    QtObject
    {
        id: document

        property var defaultParameters: ({})

        function writtenString(string) { return testHelper.writtenString(string); }

        function visualisationDefaultParameters(valueType, channelName)
        {
            return defaultParameters;
        }
    }

    function test_expressionFor_data()
    {
        return [
            {tag: "no parameters", attributeName: "NodeDegree", flags: AttributeFlag.None,
                channelName: "Size", parameters: {}},
            {tag: "quoted attribute name", attributeName: "\"Node Degree\"", flags: AttributeFlag.None,
                channelName: "Size", parameters: {}},
            {tag: "by component", attributeName: "ComponentSize",
                flags: AttributeFlag.VisualiseByComponent | AttributeFlag.AutoRange,
                channelName: "Colour", parameters: {}},
            {tag: "JSON parameter", attributeName: "NodeDegree", flags: AttributeFlag.None,
                channelName: "Colour", parameters: {gradient: "{ \"0\": \"#FF0000\", \"1\": \"#0000FF\" }"}},
            {tag: "several parameters", attributeName: "NodeName", flags: AttributeFlag.None,
                channelName: "Colour", parameters: {palette: "{\"colors\":[\"#FF0000\"]}", other: "x"}},
            {tag: "quotes in parameter", attributeName: "NodeName", flags: AttributeFlag.None,
                channelName: "Text", parameters: {text: "say \"hi\""}},
            {tag: "channel name needing quotes", attributeName: "NodeName", flags: AttributeFlag.None,
                channelName: "Text Colour", parameters: {}}
        ];
    }

    function test_expressionFor(data)
    {
        document.defaultParameters = data.parameters;

        let expression = VisualisationUtils.expressionFor(document,
            data.attributeName, data.flags, ValueType.Int, data.channelName);

        let visualisation = testHelper.parseVisualisation(expression);
        verify(Object.keys(visualisation).length > 0, "Failed to parse " + expression);

        compare(visualisation.attribute, data.attributeName.replace(/"/g, ""));
        compare(visualisation.channel, data.channelName);
        compare(visualisation.flags,
            (data.flags & AttributeFlag.VisualiseByComponent) ? ["component"] : []);

        compare(Object.keys(visualisation.parameters).sort(), Object.keys(data.parameters).sort());

        for(let name in data.parameters)
        {
            let value = Utils.unescapeQuotes(visualisation.parameters[name]);
            compare(value, Utils.sanitiseJson(data.parameters[name]), name);
        }
    }
}
