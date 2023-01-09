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
import QtQuick.Layouts

import app.graphia.Controls

FramedScrollView
{
    id: root

    property var model: null
    property var textProvider: (modelData) => modelData

    ColumnLayout
    {
        spacing: 0

        Repeater
        {
            id: repeater

            model: root.model ? root.model : []

            CheckBox
            {
                rightPadding: root.scrollBarWidth
                checked: true
                text: root.textProvider(modelData)

                property string value: modelData
            }
        }
    }

    property var selected:
    {
        let a = [];

        for(let i = 0; i < repeater.count; i++)
        {
            let item = repeater.itemAt(i);

            if(item.checked)
                a.push(item.value);
        }

        return a;
    }

    function checkAll()
    {
        for(let i = 0; i < repeater.count; i++)
        {
            let item = repeater.itemAt(i);
            item.checked = true;
        }
    }
}
