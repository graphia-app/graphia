/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import app.graphia 1.0

import ".."
import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils
import "VisualisationUtils.js" as VisualisationUtils

import "../Controls"

Item
{
    id: root

    width: row.width
    height: row.height

    property color enabledTextColor
    property color disabledTextColor
    property color hoverColor
    property color textColor: enabledMenuItem.checked ? enabledTextColor : disabledTextColor

    property var gradientSelector
    Connections
    {
        target: gradientSelector

        function onConfigurationChanged()
        {
            if(gradientSelector.visualisationIndex !== index)
                return;

            parameters["gradient"] = "\"" + Utils.escapeQuotes(gradientSelector.configuration) + "\"";
            root.updateExpression();
        }
    }

    property var paletteSelector
    Connections
    {
        target: paletteSelector

        function onConfigurationChanged()
        {
            if(paletteSelector.visualisationIndex !== index)
                return;

            parameters["palette"] = "\"" + Utils.escapeQuotes(paletteSelector.configuration) + "\"";
            root.updateExpression();
        }
    }

    MouseArea
    {
        anchors.fill: row

        onClicked:
        {
            if(mouse.button === Qt.RightButton)
                hamburger.menu.popup();
        }

        onDoubleClicked: { root.toggle(); }

        // Pass presses on to parent (DraggableList)
        onPressed: { mouse.accepted = false; }
    }

    RowLayout
    {
        id: row

        AlertIcon
        {
            id: alertIcon
            visible: false
        }

        ComboBox
        {
            id: attributeList

            implicitWidth: 180

            model:
            {
                if(similarAttributes !== undefined)
                    return similarAttributes;

                if(attributeName.length > 0)
                    return [attributeName];

                return [];
            }

            enabled: enabledMenuItem.checked

            onCurrentIndexChanged: { updateExpression(); }
        }

        ComboBox
        {
            id: attributeParameterList

            implicitWidth: 180

            enabled: enabledMenuItem.checked
            visible: false

            onCurrentIndexChanged: { updateExpression(); }
        }

        Label
        {
            id: channelLabel
            visible: !gradientKey.visible && !paletteKey.visible
            text: channel
            enabled: enabledMenuItem.checked
            color: root.textColor
        }

        GradientKey
        {
            id: gradientKey
            visible: false
            enabled: enabledMenuItem.checked

            keyWidth: 100

            textColor: root.textColor
            hoverColor: root.hoverColor

            invert: isFlagSet("invert");

            minimum: root._visualisationInfo.minimumNumericValue !== undefined ?
                root._visualisationInfo.minimumNumericValue : 0.0
            maximum: root._visualisationInfo.maximumNumericValue !== undefined ?
                root._visualisationInfo.maximumNumericValue : 0.0

            showLabels:
            {
                return !root.isFlagSet("disabled") && !root._error &&
                    root._visualisationInfo.hasNumericRange !== undefined &&
                    root._visualisationInfo.hasNumericRange;
            }

            onClicked:
            {
                if(mouse.button === Qt.LeftButton)
                {
                    gradientSelector.visualisationIndex = index;
                    gradientSelector.configuration = gradientKey.configuration;
                    gradientSelector.show();
                }
                else
                    mouse.accepted = false;
            }
        }

        PaletteKey
        {
            id: paletteKey
            visible: false
            enabled: enabledMenuItem.checked

            textColor: root.textColor
            hoverColor: root.hoverColor

            stringValues: root._visualisationInfo.stringValues !== undefined ?
                root._visualisationInfo.stringValues : []

            onClicked:
            {
                if(mouse.button === Qt.LeftButton)
                {
                    paletteSelector.visualisationIndex = index;
                    paletteSelector.configuration = paletteKey.configuration;
                    paletteSelector.stringValues = root._visualisationInfo.stringValues;
                    paletteSelector.show();
                }
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

            menu: Menu
            {
                id: optionsMenu

                MenuItem
                {
                    id: enabledMenuItem

                    text: qsTr("Enabled")
                    checkable: true
                    enabled: alertIcon.type !== "error"

                    onCheckedChanged:
                    {
                        setFlag("disabled", !checked);
                        updateExpression();
                    }
                }

                MenuItem
                {
                    id: invertMenuItem

                    text: qsTr("Invert")
                    checkable: true
                    enabled: alertIcon.type !== "error"

                    visible:
                    {
                        // Inversion doesn't really make sense unless
                        // we're dealing with a gradient
                        if(!gradientKey.visible)
                            return false;

                        var valueType = document.attribute(attributeName).valueType;
                        return valueType === ValueType.Float || valueType === ValueType.Int;
                    }

                    onCheckedChanged:
                    {
                        setFlag("invert", checked);
                        updateExpression();
                    }
                }

                MenuItem
                {
                    id: perComponentMenuItem

                    text: qsTr("Apply Per Component")
                    checkable: true
                    enabled: alertIcon.type !== "error"

                    visible:
                    {
                        var valueType = document.attribute(attributeName).valueType;
                        return valueType === ValueType.Float || valueType === ValueType.Int;
                    }

                    onCheckedChanged:
                    {
                        setFlag("component", checked);
                        updateExpression();
                    }
                }

                property bool _showAssignByOptions:
                {
                    if(!paletteKey.visible)
                        return false;

                    return root.attributeType === ValueType.String;
                }

                ExclusiveGroup { id: sortByExclusiveGroup }

                MenuSeparator { visible: optionsMenu._showAssignByOptions }

                MenuItem
                {
                    id: sortByValueMenuItem

                    enabled: alertIcon.type !== "error"
                    visible: optionsMenu._showAssignByOptions

                    text: qsTr("By Value")
                    checkable: true
                    exclusiveGroup: sortByExclusiveGroup
                }

                MenuItem
                {
                    id: sortBySharedValuesMenuItem

                    enabled: alertIcon.type !== "error"
                    visible: optionsMenu._showAssignByOptions

                    text: qsTr("By Quantity")
                    checkable: true
                    exclusiveGroup: sortByExclusiveGroup

                    onCheckedChanged:
                    {
                        setFlag("assignByQuantity", checked);
                        updateExpression();
                    }
                }

                MenuSeparator { visible: optionsMenu._showAssignByOptions }

                MenuItem
                {
                    text: qsTr("Delete")
                    iconName: "edit-delete"

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
        updateExpression();
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
    readonly property var similarAttributes: attributeName.length > 0 ?
        document.attributesSimilarTo(attributeName) : []
    property var attributeType:
    {
        var valueType = document.attribute(attributeName).valueType;
        if(valueType === ValueType.Float || valueType === ValueType.Int)
            return ValueType.Numerical;

        return valueType;
    }

    property string channel
    property var parameters

    function updateExpression()
    {
        if(!ready)
            return;

        var flagsString = "";
        if(flags.length > 0)
            flagsString = "[" + flags.toString() + "] ";

        var attribute = document.attribute(attributeList.currentText);
        var parameterValue = "";
        if(attribute.hasParameter)
        {
            if(attributeParameterList.count === 0)
                parameterValue = attribute.validParameterValues[0];
            else
                parameterValue = attributeParameterList.currentText;
        }

        var attributeName = VisualisationUtils.decorateAttributeName(
            attributeList.currentText, parameterValue);

        var newExpression = flagsString + attributeName + " \"" + channel + "\"";

        if(Object.keys(parameters).length !== 0)
            newExpression += " with";

        for(var key in parameters)
            newExpression += " " + key + " = " + parameters[key];

        value = newExpression;
        document.update();
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

        for(var key in parameters)
        {
            var value = parameters[key];
            var unescaped = Utils.unescapeQuotes(value);

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
            }
        }
    }

    property bool _error: false
    property int index: -1
    property string value
    onValueChanged:
    {
        if(!ready)
        {
            var visualisationConfig = document.parseVisualisation(value);

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

            parseParameters();

            enabledMenuItem.checked = !isFlagSet("disabled") && !_error;
            invertMenuItem.checked = isFlagSet("invert");
            perComponentMenuItem.checked = isFlagSet("component");
            sortByValueMenuItem.checked = !isFlagSet("assignByQuantity");
            sortBySharedValuesMenuItem.checked = isFlagSet("assignByQuantity");

            var attribute = document.attribute(attributeName);
            attributeList.currentIndex = attributeList.find(
                attribute.name !== undefined ? attribute.name : attributeName);
            if(attribute.hasParameter)
            {
                attributeParameterList.model = attribute.validParameterValues;
                attributeParameterList.currentIndex =
                    attributeParameterList.find(attribute.parameterValue);

                if(attributeParameterList.currentIndex < 0 && attributeParameterList.count > 0)
                    attributeParameterList.currentIndex = 0;

                attributeParameterList.visible = true;
            }
            else
            {
                attributeParameterList.model = [];
                attributeParameterList.visible = false;
            }

            ready = true;
        }
    }
}
