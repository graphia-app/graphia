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
    property color pressedColor
    property color textColor: enabledMenuItem.checked ? enabledTextColor : disabledTextColor

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
                        document.updateVisualisations();
                    }
                }
            }
        }

        ButtonMenu
        {
            id: attributeList
            selectedValue: attribute
            model: document.availableAttributesSimilarTo(attribute);
            enabled: enabledMenuItem.checked

            textColor: root.textColor
            pressedColor: root.pressedColor

            onSelectedValueChanged: { updateExpression(); }
        }

        Label
        {
            id: channelLabel
            text: channel
            enabled: enabledMenuItem.checked
            color: root.textColor
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

    function updateExpression()
    {
        if(!ready)
            return;

        var flagsString = "";
        if(flags.length > 0)
            flagsString = "[" + flags.toString() + "] ";

        var newExpression = flagsString + "\"" + attributeList.selectedValue + "\" \"" + channel + "\"";

        value = newExpression;
        document.updateVisualisations();
    }

    function setVisualisationAlert(visualisationAlert)
    {
        switch(visualisationAlert.type)
        {
        case VisualisationAlertType.Error:
            alertIcon.type = "error";
            alertIcon.text = visualisationAlert.text;
            alertIcon.visible = true;
            break;

        case VisualisationAlertType.Warning:
            alertIcon.type = "warning";
            alertIcon.text = visualisationAlert.text;
            alertIcon.visible = true;
            break;

        default:
        case VisualisationAlertType.None:
            alertIcon.visible = false;
        }


    }

    property int index
    property string value
    onValueChanged:
    {
        if(!ready)
        {
            var visualisationConfig = document.parseVisualisation(value);

            flags = visualisationConfig.flags;
            attribute = visualisationConfig.attribute;
            channel = visualisationConfig.channel;

            enabledMenuItem.checked = !isFlagSet("disabled");
            invertMenuItem.checked = isFlagSet("invert");

            var visualisationAlert = document.visualisationAlertAtIndex(index);
            setVisualisationAlert(visualisationAlert);

            ready = true;
        }
    }
}
