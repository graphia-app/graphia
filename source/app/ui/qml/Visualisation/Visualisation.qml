import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import ".."
import "VisualisationConfig.js" as VisualisationConfig
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
    property color textColor: enabledMenuItem.checked ? enabledTextColor : disabledTextColor

    function onDoubleClicked()
    {
        root.toggle();
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
                        var fieldType = document.dataFieldType(dataField);
                        return fieldType === FieldType.Float || fieldType === FieldType.Int;
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

        RowLayout
        {
            id: expression
            enabled: enabledMenuItem.checked
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

    property string dataField
    property string channel

    function updateExpression()
    {
        if(!ready)
            return;

        var flagsString = "";
        if(flags.length > 0)
            flagsString = "[" + flags.toString() + "] ";

        var newExpression = flagsString + "\"" + dataField + "\" \"" + channel + "\"";

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
            var visualisationConfig = new VisualisationConfig.create(document.parseVisualisation(value));
            visualisationConfig.toComponents(document, expression);

            flags = visualisationConfig.flags;
            dataField = visualisationConfig.dataField;
            channel = visualisationConfig.channel;

            enabledMenuItem.checked = !isFlagSet("disabled");
            invertMenuItem.checked = isFlagSet("invert");

            var visualisationAlert = document.visualisationAlertAtIndex(index);
            setVisualisationAlert(visualisationAlert);

            ready = true;
        }
    }
}
