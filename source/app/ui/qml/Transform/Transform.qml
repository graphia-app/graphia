import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import com.kajeka 1.0

import ".."
import "TransformConfig.js" as TransformConfig
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

    RowLayout
    {
        id: row

        Label
        {
            text: "📌"
            color: textColor
            visible: root.pinned
        }

        RowLayout
        {
            id: expression
            enabled: enabledMenuItem.checked

            // Hack to force the row to always be at least the height of a TextField
            // (so that when the Transform is locked, it doesn't change height)
            TextField { id: dummyField; visible: false }
            Rectangle { height: dummyField.height }
        }

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
                    id: lockedMenuItem

                    text: qsTr("Locked")
                    checkable: true

                    onCheckedChanged:
                    {
                        setMetaAttribute("locked", checked);
                        updateExpression();
                    }
                }

                MenuItem
                {
                    id: pinnedMenuItem

                    text: qsTr("Pinned To Bottom")
                    checkable: true

                    onCheckedChanged:
                    {
                        setMetaAttribute("pinned", checked);
                        updateExpression();
                    }
                }

                MenuItem
                {
                    text: qsTr("Delete")
                    iconName: "edit-delete"

                    onTriggered:
                    {
                        document.removeGraphTransform(index);
                        document.updateGraphTransforms();
                    }
                }
            }
        }
    }

    property bool ready: false

    property var metaAttributes: []
    property bool pinned: { return isMetaAttributeSet("pinned"); }

    function toggle()
    {
        setMetaAttribute("disabled", !isMetaAttributeSet("disabled"));
        updateExpression();
    }

    function toggleLock()
    {
        setMetaAttribute("locked", !isMetaAttributeSet("locked"));
        updateExpression();
    }

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

    property string template
    property var parameters

    function updateExpression()
    {
        if(!ready)
            return;

        var numTemplateArguments = (template.match(/%/g) || []).length;
        if(numTemplateArguments !== parameters.length)
        {
            console.log("Number of template arguments (" + numTemplateArguments + ") doesn't " +
                        "match number of parameters (" + parameters.length + ")");
            return;
        }

        var metaAttributesString = "";
        if(metaAttributes.length > 0)
            metaAttributesString = "[" + metaAttributes.toString() + "] ";

        var newExpression = metaAttributesString + template;

        for(var i = 0; i < parameters.length; i++)
        {
            var parameter = parameters[i].value;
            parameter = parameter.replace("\$", "$$$$");
            newExpression = newExpression.replace("%", parameter);
        }

        modelData = newExpression;
        document.updateGraphTransforms();
    }

    Component.onCompleted:
    {
        var transformConfig = new TransformConfig.create(document.parseGraphTransform(modelData));
        transformConfig.toComponents(document, expression);

        metaAttributes = transformConfig.metaAttributes;
        template = transformConfig.template;
        parameters = transformConfig.parameters;

        for(var i = 0; i < parameters.length; i++)
        {
            var parameter = parameters[i];
            parameter.valueChanged.connect(updateExpression);
        }

        enabledMenuItem.checked = !isMetaAttributeSet("disabled");
        lockedMenuItem.checked = isMetaAttributeSet("locked");
        pinnedMenuItem.checked = isMetaAttributeSet("pinned");
        ready = true;
    }
}
