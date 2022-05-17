/* Copyright © 2013-2022 Graphia Technologies Ltd.
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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

import app.graphia 1.0
import app.graphia.Controls 1.0
import app.graphia.Shared 1.0
import app.graphia.Shared.Controls 1.0

import ".."
import "TransformConfig.js" as TransformConfig

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
            id: hamburger

            width: 20
            height: 15
            color: disabledTextColor
            hoverColor: enabledTextColor
            propogatePresses: true

            menu: PlatformMenu
            {
                PlatformMenuItem
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

                PlatformMenuItem
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

                PlatformMenuItem
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

                PlatformMenuItem
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

                PlatformMenuItem
                {
                    text: qsTr("Delete")
                    icon.name: "edit-delete"

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
    property var _parameterComponents

    function updateExpression()
    {
        if(!ready)
            return;

        let expandedTemplate = template;

        expandedTemplate = expandedTemplate.replace(/%(\d+)/g,
        function(match, index)
        {
            if(index > _parameterComponents.length)
            {
                console.log("Template argument index is out of range " + index + " > " +
                    _parameterComponents.length);
                return match;
            }

            let parameterComponent = _parameterComponents[index - 1];
            let value = parameterComponent.value;
            return value !== undefined ? value : match;
        });

        // Unescape literal % in the original template
        expandedTemplate = expandedTemplate.replace(/%\!/g, "%");

        let flagsString = "";
        if(flags.length > 0)
            flagsString = "[" + flags.toString() + "] ";

        value = flagsString + expandedTemplate;
        document.setGraphTransform(index, value);
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
            if(document.graphTransformIsValid(value))
            {
                if(document.hasTransformInfo() && index >= 0)
                {
                    let transformInfo = document.transformInfoAtIndex(index);
                    setAlertIcon(transformInfo);
                }

                let transformConfig = new TransformConfig.Create(index, document.parseGraphTransform(value));
                flags = transformConfig.flags;
                template = transformConfig.template;

                transformConfig.toComponents(document, expression, isFlagSet("locked"), updateExpression);
                _parameterComponents = transformConfig.parameters;
            }
            else
            {
                let detail = document.hasTransformInfo() && index >= 0 ?
                    document.transformInfoAtIndex(index).alertText : "";

                setAlertIcon(
                {
                    "alertType": AlertType.Error,
                    "alertText": qsTr("This transform expression is invalid\n" +
                        (detail.length > 0 ? "(Reason: " + detail + ")\n" : "") +
                        "Please send a screenshot to the developers")
                });

                flags = [];
                template = "";
                _parameterComponents = [];

                TransformConfig.addLabelTo(
                    (detail.length > 0 ? detail + ": " : "Invalid Expression: ") +
                    Utils.addSlashes(value), expression);
            }

            enabledMenuItem.checked = !isFlagSet("disabled");
            lockedMenuItem.checked = isFlagSet("locked");
            repeatingMenuItem.checked = isFlagSet("repeating");
            pinnedMenuItem.checked = isFlagSet("pinned");
            ready = true;
        }
    }
}
