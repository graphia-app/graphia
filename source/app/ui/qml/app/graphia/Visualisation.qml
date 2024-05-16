/* Copyright © 2013-2024 Graphia Technologies Ltd.
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
import QtQuick.Controls
import QtQuick.Layouts

import app.graphia
import app.graphia.Controls
import app.graphia.SharedTypes
import app.graphia.Utils

Item
{
    id: root

    width: row.width
    height: row.height

    property var document: null

    property color enabledTextColor
    property color disabledTextColor
    property color hoverColor
    property color textColor: enabledMenuItem.checked ? enabledTextColor : disabledTextColor

    property var gradientSelector
    Connections
    {
        target: gradientSelector

        function onAccepted()
        {
            if(gradientSelector.visualisationIndex !== index)
                return;

            if(gradientSelector.applied)
                return;

            parameters["gradient"] = "\"" + Utils.escapeQuotes(gradientSelector.configuration) + "\"";
            root.updateExpression();
        }

        function onRejected()
        {
            if(gradientSelector.visualisationIndex !== index)
                return;

            if(gradientSelector.applied)
                document.rollback();
        }

        function onApplyClicked(alreadyApplied)
        {
            if(gradientSelector.visualisationIndex !== index)
                return;

            parameters["gradient"] = "\"" + Utils.escapeQuotes(gradientSelector.configuration) + "\"";
            root.updateExpression(alreadyApplied);
        }
    }

    property var paletteSelector
    Connections
    {
        target: paletteSelector

        function onAccepted()
        {
            if(paletteSelector.visualisationIndex !== index)
                return;

            if(paletteSelector.applied)
                return;

            parameters["palette"] = "\"" + Utils.escapeQuotes(paletteSelector.configuration) + "\"";
            root.updateExpression();
        }

        function onRejected()
        {
            if(paletteSelector.visualisationIndex !== index)
                return;

            if(paletteSelector.applied)
                document.rollback();
        }

        function onApplyClicked(alreadyApplied)
        {
            if(paletteSelector.visualisationIndex !== index)
                return;

            parameters["palette"] = "\"" + Utils.escapeQuotes(paletteSelector.configuration) + "\"";
            root.updateExpression(alreadyApplied);
        }
    }

    property var mappingSelector
    Connections
    {
        target: mappingSelector

        function onAccepted()
        {
            if(mappingSelector.visualisationIndex !== index)
                return;

            if(mappingSelector.applied)
                return;

            parameters["mapping"] = "\"" + Utils.escapeQuotes(mappingSelector.configuration) + "\"";
            root.updateExpression();
        }

        function onRejected()
        {
            if(mappingSelector.visualisationIndex !== index)
                return;

            if(mappingSelector.applied)
                document.rollback();
            else
                optionsMenu.setupMappingMenuItems();
        }

        function onApplyClicked(alreadyApplied)
        {
            if(mappingSelector.visualisationIndex !== index)
                return;

            parameters["mapping"] = "\"" + Utils.escapeQuotes(mappingSelector.configuration) + "\"";
            root.updateExpression(alreadyApplied);
        }
    }

    MouseArea
    {
        anchors.fill: row

        onClicked: function(mouse)
        {
            if(mouse.button === Qt.RightButton)
                hamburger.menu.popup();
        }

        onDoubleClicked: function(mouse) { root.toggle(); }

        // Pass presses on to parent (DraggableList)
        onPressed: function(mouse) { mouse.accepted = false; }
    }

    RowLayout
    {
        id: row

        AlertIcon
        {
            id: alertIcon
            visible: false
        }

        TreeComboBox
        {
            id: attributeList

            implicitWidth: 180
            model: root.similarAttributes
            enabled: !isFlagSet("disabled") && root._valid
            placeholderText: root.attributeName

            onSelectedValueChanged:
            {
                if(selectedValue !== undefined)
                    root.updateExpression();
            }

            prettifyFunction: AttributeUtils.prettify

            Preferences
            {
                section: "misc"
                property alias visualisationAttributeSortOrder: attributeList.ascendingSortOrder
                property alias visualisationAttributeSortBy: attributeList.sortRoleName
            }
        }

        NamedIcon
        {
            visible: iconName.length > 0
            enabled: enabledMenuItem.checked
            iconName:
            {
                if(!root._valid || channel.length === 0)
                    return "applications-other";

                if(channel === "Shared Text")
                    return "shared-text";

                if(channel === "Text")
                    return "format-text-bold";

                if(channel === "Size")
                {
                    if(attributeElementType === ElementType.Node)
                        return "node-size";
                    else if(attributeElementType === ElementType.Edge)
                        return "edge-size";

                    // Element type not known (probably because the visualisation is erroring)
                    return "view-fullscreen";
                }

                if(channel === "Text Colour")
                    return "text-color";

                if(channel === "Text Size")
                    return "text-size";

                return "";
            }
        }

        function showGradientSelector()
        {
            gradientSelector.visualisationIndex = index;
            gradientSelector.open(gradientKey.configuration);
        }

        GradientKey
        {
            id: gradientKey
            visible: false
            enabled: enabledMenuItem.checked && !root._error

            keyWidth: 100

            textColor: root.textColor
            hoverColor: root.hoverColor

            invert: isFlagSet("invert");

            minimum: root._visualisationInfo.minimumNumericValue !== undefined ?
                root._visualisationInfo.minimumNumericValue : 0.0
            maximum: root._visualisationInfo.maximumNumericValue !== undefined ?
                root._visualisationInfo.maximumNumericValue : 0.0

            mappedMinimum: root._visualisationInfo.mappedMinimumNumericValue !== undefined ?
                root._visualisationInfo.mappedMinimumNumericValue : 0.0
            mappedMaximum: root._visualisationInfo.mappedMaximumNumericValue !== undefined ?
                root._visualisationInfo.mappedMaximumNumericValue : 0.0

            showLabels:
            {
                return !root.isFlagSet("disabled") && !root._error &&
                    (root._visualisationInfo.hasNumericRange !== undefined &&
                    root._visualisationInfo.hasNumericRange) &&
                    // If the visualisation has been applied to multiple components,
                    // then there are multiple ranges; don't show any labels
                    (root._visualisationInfo.numApplications !== undefined &&
                    root._visualisationInfo.numApplications === 1);
            }

            onClicked: function(mouse)
            {
                if(mouse.button === Qt.LeftButton)
                    row.showGradientSelector();
                else
                    mouse.accepted = false;
            }
        }

        function showPaletteSelector()
        {
            paletteSelector.visualisationIndex = index;
            paletteSelector.open(paletteKey.configuration,
                root._visualisationInfo.stringValues);
        }

        PaletteKey
        {
            id: paletteKey
            visible: false
            enabled: enabledMenuItem.checked && !root._error

            textColor: root.textColor
            hoverColor: root.hoverColor

            stringValues: root._visualisationInfo.stringValues !== undefined ?
                root._visualisationInfo.stringValues : []

            onClicked: function(mouse)
            {
                if(mouse.button === Qt.LeftButton)
                    row.showPaletteSelector();
                else
                    mouse.accepted = false;
            }
        }

        Hamburger
        {
            id: hamburger

            width: 20
            height: 15
            color: disabledTextColor
            hoverColor: enabledTextColor
            propogatePresses: true

            menu: PlatformMenu
            {
                id: optionsMenu

                PlatformMenuItem
                {
                    id: enabledMenuItem

                    text: qsTr("Enabled")
                    checkable: true

                    onCheckedChanged:
                    {
                        setFlag("disabled", !checked);
                        root.updateExpression();
                    }
                }

                PlatformMenuItem
                {
                    text: qsTr("Edit…")
                    hidden: !gradientKey.visible && !paletteKey.visible
                    enabled: visible && enabledMenuItem.checked && !root._error

                    onTriggered:
                    {
                        if(gradientKey.visible)
                            row.showGradientSelector();
                        else if(paletteKey.visible)
                            row.showPaletteSelector();
                    }
                }

                property bool _showMappingOptions:
                {
                    return document.visualisationChannelAllowsMapping(root.channel) &&
                        root.attributeValueType === ValueType.Numerical;
                }

                PlatformMenuSeparator { hidden: !optionsMenu._showMappingOptions }

                PlatformMenuItem
                {
                    id: invertMenuItem

                    text: qsTr("Invert")
                    checkable: true
                    enabled: visible && enabledMenuItem.checked && !root._error

                    hidden: !optionsMenu._showMappingOptions

                    onCheckedChanged:
                    {
                        setFlag("invert", checked);
                        root.updateExpression();
                    }
                }

                ButtonGroup { buttons: [minmaxMenuItem, stddevMenuItem, customMappingMenuItem] }

                function setupMappingMenuItems()
                {
                    if(parameters.mapping === undefined)
                    {
                        minmaxMenuItem.checked = true;
                        return;
                    }

                    let mappingString = Utils.unescapeQuotes(parameters["mapping"]);
                    let mapping = JSON.parse(mappingString);

                    if(mapping.exponent !== undefined && mapping.exponent !== 1.0)
                        customMappingMenuItem.checked = true;
                    else if(mapping.type !== undefined)
                    {
                        if(mapping.type === "minmax")
                            minmaxMenuItem.checked = true;
                        else if(mapping.type === "stddev")
                            stddevMenuItem.checked = true;
                    }
                    else
                        customMappingMenuItem.checked = true;
                }

                PlatformMenuItem
                {
                    id: minmaxMenuItem
                    text: qsTr("Min/Max")
                    enabled: visible && enabledMenuItem.checked && !root._error

                    // This is the default when there is no mapping
                    checked: parameters.mapping === undefined

                    checkable: true

                    hidden: !optionsMenu._showMappingOptions

                    onTriggered:
                    {
                        parameters["mapping"] = "\"{\\\"type\\\":\\\"minmax\\\",\\\"exponent\\\":1}\"";
                        root.updateExpression();
                    }
                }

                PlatformMenuItem
                {
                    id: stddevMenuItem
                    text: qsTr("Standard Deviation")
                    enabled: visible && enabledMenuItem.checked && !root._error

                    checkable: true

                    hidden: !optionsMenu._showMappingOptions

                    onTriggered:
                    {
                        parameters["mapping"] = "\"{\\\"type\\\":\\\"stddev\\\",\\\"exponent\\\":1}\"";
                        root.updateExpression();
                    }
                }

                PlatformMenuItem
                {
                    id: customMappingMenuItem

                    text: qsTr("Custom Mapping…")

                    checkable: true

                    enabled: visible && enabledMenuItem.checked && !root._error
                    hidden: !optionsMenu._showMappingOptions

                    onTriggered:
                    {
                        mappingSelector.visualisationIndex = index;
                        mappingSelector.values = root._visualisationInfo.numericValues;
                        mappingSelector.invert = isFlagSet("invert");

                        if(parameters.mapping !== undefined)
                            mappingSelector.initialise(Utils.unescapeQuotes(parameters["mapping"]));
                        else
                            mappingSelector.reset();

                        mappingSelector.show();
                    }
                }

                PlatformMenuSeparator { hidden: !optionsMenu._showMappingOptions }

                PlatformMenuItem
                {
                    id: perComponentMenuItem

                    text: qsTr("Apply Per Component")
                    checkable: true
                    enabled: visible && enabledMenuItem.checked && !root._error

                    hidden: !optionsMenu._showMappingOptions

                    onCheckedChanged:
                    {
                        setFlag("component", checked);
                        root.updateExpression();
                    }
                }

                property bool _showAssignByOptions:
                {
                    if(!paletteKey.visible)
                        return false;

                    return root.attributeValueType === ValueType.String;
                }

                ButtonGroup { buttons: [sortByValueMenuItem, sortBySharedValuesMenuItem] }

                PlatformMenuSeparator { hidden: !optionsMenu._showAssignByOptions }

                PlatformMenuItem
                {
                    id: sortByValueMenuItem

                    enabled: visible && enabledMenuItem.checked && !root._error
                    hidden: !optionsMenu._showAssignByOptions

                    text: qsTr("By Value")
                    checkable: true
                }

                PlatformMenuItem
                {
                    id: sortBySharedValuesMenuItem

                    enabled: visible && enabledMenuItem.checked && !root._error
                    hidden: !optionsMenu._showAssignByOptions

                    text: qsTr("By Quantity")
                    checkable: true

                    onCheckedChanged:
                    {
                        setFlag("assignByQuantity", checked);
                        root.updateExpression();
                    }
                }

                PlatformMenuSeparator { hidden: !optionsMenu._showAssignByOptions }

                PlatformMenuItem
                {
                    text: qsTr("Delete")
                    icon.name: "edit-delete"

                    onTriggered:
                    {
                        document.removeVisualisation(index);
                        document.update();
                    }
                }
            }
        }
    }

    property bool ready: false

    function toggle()
    {
        if(!enabledMenuItem.enabled)
            return;

        setFlag("disabled", !isFlagSet("disabled"));
        root.updateExpression();
    }

    property var flags: []
    function setFlag(flag, value)
    {
        if(!ready)
            return;

        if(value)
        {
            flags.push(flag);
            flags = flags.filter(function(e, i)
            {
                return flags.lastIndexOf(e) === i;
            });
        }
        else
        {
            flags = flags.filter(function(e)
            {
                return e !== flag;
            });
        }
    }

    function isFlagSet(flag)
    {
        return flags.indexOf(flag) >= 0;
    }

    property string attributeName

    property var similarAttributes: null
    function updateSimilarAttributes()
    {
        if(document.attributeExists(attributeName))
            similarAttributes = document.attributesSimilarTo(attributeName);

        // The attribute doesn't exist, so we work backwards to provide some sensible options
        else if(root.channel === "Size")
            similarAttributes = document.availableAttributesModel(ElementType.NodeAndEdge, ValueType.Numerical);
        else if(root.parameters !== undefined && root.parameters.hasOwnProperty("gradient"))
            similarAttributes = document.availableAttributesModel(ElementType.NodeAndEdge, ValueType.Numerical);
        else if(root.parameters !== undefined && root.parameters.hasOwnProperty("palette"))
            similarAttributes = document.availableAttributesModel(ElementType.NodeAndEdge, ValueType.String);
        else
            similarAttributes = document.availableAttributesModel();
    }

    onAttributeNameChanged: { updateSimilarAttributes(); }
    Connections
    {
        target: document
        function onAttributesChanged() { updateSimilarAttributes(); }
    }

    property var attributeValueType:
    {
        let attribute = document.attribute(attributeName);
        if(!attribute.isValid)
            return ValueType.Unknown;

        if(attribute.valueType === ValueType.Float || attribute.valueType === ValueType.Int)
            return ValueType.Numerical;

        return attribute.valueType;
    }

    property var attributeElementType:
    {
        let attribute = document.attribute(attributeName);
        if(!attribute.isValid)
            return ElementType.None;

        return attribute.elementType;
    }

    property string channel
    property var parameters

    function updateExpression(replaceLastCommand = false)
    {
        if(!ready)
            return;

        let flagsString = "";
        if(flags.length > 0)
            flagsString = "[" + flags.toString() + "] ";

        let newAttributeName = attributeList.selectedValue ?
            attributeList.selectedValue :
            " \"" + root.attributeName + "\"";
        let newExpression = flagsString + newAttributeName + " \"" + channel + "\"";

        if(Object.keys(parameters).length !== 0)
            newExpression += " with";

        for(let key in parameters)
            newExpression += " " + key + " = " + parameters[key];

        value = newExpression;
        document.setVisualisation(index, value);
        document.update([], [], replaceLastCommand);
    }

    property var _visualisationInfo: ({})

    function setVisualisationInfo(visualisationInfo)
    {
        switch(visualisationInfo.alertType)
        {
        case AlertType.Error:
            alertIcon.type = "error";
            alertIcon.text = visualisationInfo.alertText;
            alertIcon.visible = true;
            break;

        case AlertType.Warning:
            alertIcon.type = "warning";
            alertIcon.text = visualisationInfo.alertText;
            alertIcon.visible = true;
            break;

        default:
        case AlertType.None:
            alertIcon.visible = false;
        }
    }

    function parseParameters()
    {
        gradientKey.visible = false;
        paletteKey.visible = false;

        for(let key in parameters)
        {
            let value = parameters[key];
            let unescaped = Utils.unescapeQuotes(value);

            switch(key)
            {
            case "gradient":
                gradientKey.configuration = unescaped;
                gradientKey.visible = true;
                break;

            case "palette":
                paletteKey.configuration = unescaped;
                paletteKey.visible = true;
                break;

            case "mapping":
                optionsMenu.setupMappingMenuItems();
                break;
            }
        }
    }

    property bool _error: false
    property bool _valid: true
    property int index: -1
    property string value
    onValueChanged:
    {
        if(!ready)
        {
            let visualisationConfig = document.parseVisualisation(value);

            flags = visualisationConfig.flags;
            attributeName = visualisationConfig.attribute;
            channel = visualisationConfig.channel;
            parameters = visualisationConfig.parameters;

            _error = false;
            if(document.hasVisualisationInfo() && index >= 0)
            {
                root._visualisationInfo = document.visualisationInfoAtIndex(index);
                setVisualisationInfo(root._visualisationInfo);

                _error = root._visualisationInfo.alertType === AlertType.Error;
            }

            _valid = document.visualisationIsValid(value);

            parseParameters();

            enabledMenuItem.checked = !isFlagSet("disabled");
            invertMenuItem.checked = isFlagSet("invert");
            perComponentMenuItem.checked = isFlagSet("component");
            sortByValueMenuItem.checked = !isFlagSet("assignByQuantity");
            sortBySharedValuesMenuItem.checked = isFlagSet("assignByQuantity");

            if(attributeList.model !== null)
                attributeList.select(attributeList.model.find(attributeName));

            ready = true;
        }
    }
}
