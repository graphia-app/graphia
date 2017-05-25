import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
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

        AlertIcon
        {
            id: alertIcon
            visible: false
        }

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
                    id: lockedMenuItem

                    text: qsTr("Locked")
                    checkable: true

                    onCheckedChanged:
                    {
                        setFlag("locked", checked);
                        updateExpression();
                    }
                }

                MenuItem
                {
                    id: repeatingMenuItem

                    text: qsTr("Apply Repeatedly")
                    checkable: true

                    onCheckedChanged:
                    {
                        setFlag("repeating", checked);
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
                        setFlag("pinned", checked);
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
                        document.update();
                    }
                }
            }
        }
    }

    property bool ready: false

    property bool pinned: { return isFlagSet("pinned"); }

    function toggle()
    {
        if(!enabledMenuItem.enabled)
            return;

        setFlag("disabled", !isFlagSet("disabled"));
        updateExpression();
    }

    function toggleLock()
    {
        if(!lockedMenuItem.enabled)
            return;

        setFlag("locked", !isFlagSet("locked"));
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

        var flagsString = "";
        if(flags.length > 0)
            flagsString = "[" + flags.toString() + "] ";

        var newExpression = flagsString + template;

        for(var i = 0; i < parameters.length; i++)
        {
            var parameter = parameters[i].value;
            parameter = parameter.replace("\$", "$$$$");
            newExpression = newExpression.replace("%", parameter);
        }

        value = newExpression;
        document.update();
    }

    function setAlertIcon(transformInfo)
    {
        switch(transformInfo.alertType)
        {
        case AlertType.Error:
            alertIcon.type = "error";
            alertIcon.text = transformInfo.alertText;
            alertIcon.visible = true;
            break;

        case AlertType.Warning:
            alertIcon.type = "warning";
            alertIcon.text = transformInfo.alertText;
            alertIcon.visible = true;
            break;

        default:
        case AlertType.None:
            alertIcon.visible = false;
        }
    }

    property int index: -1
    property string value
    onValueChanged:
    {
        if(!ready)
        {
            var error = false;
            if(document.hasTransformInfo() && index >= 0)
            {
                var transformInfo = document.transformInfoAtIndex(index);
                setAlertIcon(transformInfo);
                error = transformInfo.alertType === AlertType.Error;
            }

            var transformConfig = new TransformConfig.create(document.parseGraphTransform(value));
            flags = transformConfig.flags;
            template = transformConfig.template;

            transformConfig.toComponents(document, expression, isFlagSet("locked") || error, updateExpression);
            parameters = transformConfig.parameters;

            enabledMenuItem.enabled =
                    lockedMenuItem.enabled =
                    repeatingMenuItem.enabled =
                    pinnedMenuItem.enabled =
                    !error;

            enabledMenuItem.checked = !isFlagSet("disabled") && !error;
            lockedMenuItem.checked = isFlagSet("locked") || error;
            repeatingMenuItem.checked = isFlagSet("repeating");
            pinnedMenuItem.checked = isFlagSet("pinned");
            ready = true;
        }
    }
}
