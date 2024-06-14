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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import Graphia.Controls
import Graphia.Utils

Window
{
    id: root

    flags: Constants.defaultWindowFlags
    color: palette.window
    title: qsTr("Palette")

    minimumWidth: 600
    minimumHeight: 600

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        FramedScrollView
        {
            id: textArea

            Layout.fillWidth: true
            Layout.fillHeight: true

            Column
            {
                padding: Constants.padding
                spacing: 4

                Repeater
                {
                    model: ["alternateBase", "base", "brightText", "button", "buttonText",
                        "dark", "highlight", "highlightedText", "light", "link", "mid",
                        "midlight", "placeholderText", "shadow", "text", "toolTipBase",
                        "toolTipText", "window", "windowText"]

                    RowLayout
                    {
                        property string colorName: modelData

                        Repeater
                        {
                            model: ["active", "disabled", "inactive"]

                            Rectangle
                            {
                                id: colorRectangle
                                width: 128
                                height: 28
                                color: palette[modelData][colorName]
                                border.color: NativeUtils.contrastingColor(color)
                                border.width: 1

                                Label
                                {
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left
                                    anchors.leftMargin: 4
                                    text: colorRectangle.color
                                    color: NativeUtils.contrastingColor(colorRectangle.color)
                                }
                            }
                        }

                        Label
                        {
                            text: modelData
                            color: palette.windowText
                        }
                    }
                }
            }
        }

        RowLayout
        {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("Close")
                onClicked: function(mouse) { root.close(); }
            }
        }
    }
}
