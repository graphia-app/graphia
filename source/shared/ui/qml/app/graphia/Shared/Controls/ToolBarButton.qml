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

import QtQuick
import QtQuick.Controls

import app.graphia.Shared

ToolButton
{
    icon.width: 24
    icon.height: 24
    icon.color: enabled ? "transparent" : "lightgrey"

    display: icon.name.length > 0 || icon.source.length > 0 ?
        AbstractButton.IconOnly : AbstractButton.TextOnly

    // Remove menu hotkey decorations
    property string _cleansedText: { return text.replace("&", ""); }

    ToolTip.visible: display === AbstractButton.IconOnly && _cleansedText.length > 0 && hovered
    ToolTip.delay: Constants.toolTipDelay
    ToolTip.text: display === AbstractButton.IconOnly ? _cleansedText : ""
}
