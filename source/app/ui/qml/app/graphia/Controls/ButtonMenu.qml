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

import QtQml
import QtQml.Models
import QtQuick
import QtQuick.Controls

import app.graphia
import app.graphia.Controls

Item
{
    id: root

    implicitWidth: button.implicitWidth
    implicitHeight: button.implicitHeight

    property string text: ""
    property string selectedValue: ""
    property color hoverColor: ControlColors.mid
    property color textColor: palette.buttonText
    property alias font: label.font
    property alias textAlignment: label.horizontalAlignment

    property color _contrastingColor:
    {
        if(mouseArea.containsMouse || root.menuDropped)
            return QmlUtils.contrastingColor(hoverColor);

        return textColor;
    }

    property bool propogatePresses: false

    property alias model: instantiator.model

    property bool _modelIsUnset: instantiator.model === 1 // wtf?

    property bool menuDropped: false
    PlatformMenu
    {
        id: menu

        onAboutToShow: root.menuDropped = true;
        onAboutToHide: root.menuDropped = false;

        Instantiator
        {
            id: instantiator
            delegate: PlatformMenuItem
            {
                text: index >= 0 && !_modelIsUnset ? instantiator.model[index] : ""

                onTriggered: { root.selectedValue = text; }
            }

            onObjectAdded: function(index, object) { menu.insertItem(index, object); }
            onObjectRemoved: function(index, object) { menu.removeItem(object); }
        }
    }

    Rectangle
    {
        id: button
        anchors.fill: parent
        radius: 2
        color: (mouseArea.containsMouse || root.menuDropped) ? root.hoverColor : "transparent"

        implicitWidth: label.contentWidth + 8
        implicitHeight: label.contentHeight + 8

        property bool atNaturalWidth: false
        onWidthChanged: { atNaturalWidth = (width === implicitWidth); }
        onImplicitWidthChanged: { atNaturalWidth = (width === implicitWidth); }

        Label
        {
            id: label
            anchors
            {
                fill: parent
                margins: 4
            }

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            elide: button.atNaturalWidth ? Text.ElideNone : Text.ElideRight
            text: root.selectedValue !== "" ? root.selectedValue : root.text
            color: root._contrastingColor
        }
    }

    MouseArea
    {
        id: mouseArea

        hoverEnabled: true
        anchors.fill: parent
        onClicked: function(mouse)
        {
            if(mouse.button === Qt.LeftButton && menu && !_modelIsUnset)
                menu.popup(parent, 0, parent.height + 8/*padding*/);

            root.clicked(mouse);
        }

        onPressed: function(mouse) { mouse.accepted = !propogatePresses; }

        onPressAndHold: function(mouse) { root.held(mouse); }
    }

    signal clicked(var mouse)
    signal held(var mouse)
}
