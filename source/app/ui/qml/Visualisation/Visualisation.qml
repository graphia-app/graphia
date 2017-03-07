import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
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
                        setMetaAttribute("disabled", !checked);
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
                        setMetaAttribute("invert", checked);
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
        setMetaAttribute("disabled", !isMetaAttributeSet("disabled"));
        updateExpression();
    }

    property var metaAttributes: []
    function setMetaAttribute(attribute, value)
    {
        if(!ready)
            return;

        if(value)
        {
            metaAttributes.push(attribute);
            metaAttributes = metaAttributes.filter(function(e, i)
            {
                return metaAttributes.lastIndexOf(e) === i;
            });
        }
        else
        {
            metaAttributes = metaAttributes.filter(function(e)
            {
                return e !== attribute;
            });
        }
    }

    function isMetaAttributeSet(attribute)
    {
        return metaAttributes.indexOf(attribute) >= 0;
    }

    property string dataField
    property string channel

    function updateExpression()
    {
        if(!ready)
            return;

        var metaAttributesString = "";
        if(metaAttributes.length > 0)
            metaAttributesString = "[" + metaAttributes.toString() + "] ";

        var newExpression = metaAttributesString + "\"" + dataField + "\" \"" + channel + "\"";

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

            metaAttributes = visualisationConfig.metaAttributes;
            dataField = visualisationConfig.dataField;
            channel = visualisationConfig.channel;

            enabledMenuItem.checked = !isMetaAttributeSet("disabled");
            invertMenuItem.checked = isMetaAttributeSet("invert");

            var visualisationAlert = document.visualisationAlertAtIndex(index);
            setVisualisationAlert(visualisationAlert);

            ready = true;
        }
    }
}
