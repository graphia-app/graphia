import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import ".."
import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils

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

        onConfigurationChanged:
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

        onConfigurationChanged:
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

            model: similarAttributes !== undefined ? similarAttributes : [attribute]
            enabled: enabledMenuItem.checked

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

                        var valueType = document.attribute(attribute).valueType;
                        return valueType === ValueType.Float || valueType === ValueType.Int;
                    }

                    onCheckedChanged:
                    {
                        setFlag("invert", checked);
                        updateExpression();
                    }
                }

                property bool _showAssignByOptions:
                {
                    if(!paletteKey.visible)
                        return false;

                    var valueType = document.attribute(attribute).valueType;
                    return valueType === ValueType.String;
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

    property string attribute
    readonly property var similarAttributes: attribute.length > 0 ?
        document.attributesSimilarTo(attribute) : []

    property string channel
    property var parameters

    function updateExpression()
    {
        if(!ready)
            return;

        var flagsString = "";
        if(flags.length > 0)
            flagsString = "[" + flags.toString() + "] ";

        var newExpression = flagsString + "\"" + attributeList.currentText + "\" \"" + channel + "\"";

        if(Object.keys(parameters).length !== 0)
            newExpression += " with ";

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

    function parseParameters(valid)
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
                gradientKey.showLabels = !isFlagSet("disabled") && valid;
                break;

            case "palette":
                paletteKey.configuration = unescaped;
                paletteKey.visible = true;
                break;
            }
        }
    }

    property int index: -1
    property string value
    onValueChanged:
    {
        if(!ready)
        {
            var visualisationConfig = document.parseVisualisation(value);

            flags = visualisationConfig.flags;
            attribute = visualisationConfig.attribute;
            channel = visualisationConfig.channel;
            parameters = visualisationConfig.parameters;

            var error = false;
            if(document.hasVisualisationInfo() && index >= 0)
            {
                root._visualisationInfo = document.visualisationInfoAtIndex(index);
                setVisualisationInfo(root._visualisationInfo);

                error = root._visualisationInfo.alertType === AlertType.Error;
            }

            parseParameters(!error);

            enabledMenuItem.checked = !isFlagSet("disabled") && !error;
            invertMenuItem.checked = isFlagSet("invert");
            sortByValueMenuItem.checked = !isFlagSet("assignByQuantity");
            sortBySharedValuesMenuItem.checked = isFlagSet("assignByQuantity");
            attributeList.currentIndex = attributeList.find(attribute);

            ready = true;
        }
    }
}
