/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

import app.graphia

MenuItem
{
    property bool hidden: false

    clip: true
    height: hidden ? 0 : implicitHeight

    Label
    {
        id: shortcutLabel

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        text: action && action.shortcut ? QmlUtils.nativeShortcutSequence(action.shortcut) : ""
        color: highlighted ? palette.highlightedText : palette.text
    }

    Component.onCompleted:
    {
        const shortcutLabelSpacing = 10;
        let defaultRightPadding = rightPadding;
        shortcutLabel.rightPadding = defaultRightPadding;

        // Make room for the shortcut label, if it exists
        rightPadding = Qt.binding(() =>
            (shortcutLabel.text.length > 0 ? shortcutLabelSpacing + shortcutLabel.implicitWidth : 0) +
            defaultRightPadding);
    }
}
