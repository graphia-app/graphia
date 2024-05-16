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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import app.graphia.Controls

Item
{
    id: root

    width: row.width
    height: row.height

    Action
    {
        id: restartAction
        text: qsTr("Restart")
        onTriggered: function(source)
        {
            root.visible = false;
            root.restartClicked();
        }
    }

    Action
    {
        id: closeAction
        text: qsTr("Not Now")
        onTriggered: function(source)
        {
            root.visible = false;
        }
    }

    RowLayout
    {
        id: row

        Item
        {
            Layout.preferredWidth: icon.width + Math.abs(icon.endX - icon.startX)

            NamedIcon
            {
                id: icon
                anchors.verticalCenter: parent.verticalCenter

                property double startX: 16.0
                property double endX: 0.0

                iconName: "software-update-available"

                SequentialAnimation on x
                {
                    loops: Animation.Infinite

                    NumberAnimation
                    {
                        from: icon.startX; to: icon.endX
                        easing.type: Easing.OutExpo; duration: 500
                    }

                    NumberAnimation
                    {
                        from: icon.endX; to: icon.startX
                        easing.amplitude: 2.0
                        easing.type: Easing.OutBounce; duration: 500
                    }

                    PauseAnimation { duration: 500 }
                }
            }
        }

        Text { text: qsTr("An update is available!") }
        Button { action: restartAction }
        Button { action: closeAction }
    }

    signal restartClicked()
}

