import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import ".."
import "../Constants.js" as Constants
import "../Utils.js" as Utils

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

    property var gradientList
    Connections
    {
        target: gradientList

        onConfigurationChanged:
        {
            if(gradientList.visualisationIndex !== index)
                return;

            parameters["gradient"] = Utils.escapeQuotes(gradientList.configuration);
            root.updateExpression();
        }
    }

    MouseArea
    {
        anchors.fill: row
        onDoubleClicked: { root.toggle(); }

        // Pass presses on to parent (DraggableList)
        onPressed: { mouse.accepted = false; }
    }

    RowLayout
    {
        id: row

        Hamburger
        {
            width: 20
            height: 15
            color: disabledTextColor
            hoverColor: enabledTextColor
            propogatePresses: true

            menu: Menu
            {
                MenuItem
                {
                    id: enabledMenuItem

                    text: qsTr("Enabled")
                    checkable: true

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
                    enabled:
                    {
                        var valueType = document.attribute(attribute).valueType;
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

        ButtonMenu
        {
            id: attributeList
            selectedValue: attribute
            model: document.attribute(attribute).similar;
            enabled: enabledMenuItem.checked
            propogatePresses: true

            textColor: root.textColor
            hoverColor: root.hoverColor

            onSelectedValueChanged: { updateExpression(); }
        }

        Label
        {
            id: channelLabel
            visible: !gradientKey.visible
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
            propogatePresses: true

            onClicked:
            {
                gradientList.visualisationIndex = index;
                gradientList.selected = gradientKey.configuration;
                gradientList.show();
            }
        }

        AlertIcon
        {
            id: alertIcon
            visible: false
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
    property string channel
    property var parameters

    function updateExpression()
    {
        if(!ready)
            return;

        var flagsString = "";
        if(flags.length > 0)
            flagsString = "[" + flags.toString() + "] ";

        var newExpression = flagsString + "\"" + attributeList.selectedValue + "\" \"" + channel + "\"";

        if(Object.keys(parameters).length !== 0)
            newExpression += " with ";

        for(var key in parameters)
            newExpression += " " + key + " = " + parameters[key];

        value = newExpression;
        document.update();
    }

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

        gradientKey.minimum = visualisationInfo.minimumNumericValue;
        gradientKey.maximum = visualisationInfo.maximumNumericValue;
    }

    function parseParameters(valid)
    {
        gradientKey.visible = false;

        for(var key in parameters)
        {
            var value = parameters[key];

            switch(key)
            {
            case "gradient":
                if(valid)
                {
                    var unescaped = Utils.unescapeQuotes(value);
                    gradientKey.configuration = unescaped;
                    gradientKey.visible = true;
                    gradientKey.showLabels = !isFlagSet("disabled");
                }
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
                var visualisationInfo = document.visualisationInfoAtIndex(index);
                setVisualisationInfo(visualisationInfo);

                error = visualisationInfo.alertType === AlertType.Error;
                parseParameters(!error);
            }

            enabledMenuItem.enabled = invertMenuItem.enabled = !error
            enabledMenuItem.checked = !isFlagSet("disabled") && !error;
            invertMenuItem.checked = isFlagSet("invert");

            ready = true;
        }
    }
}
