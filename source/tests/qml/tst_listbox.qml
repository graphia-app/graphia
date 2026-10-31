/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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
import QtTest

import Graphia.Controls

TestCase
{
    name: "ListBox"
    when: windowShown

    Component { id: listBoxComponent; ListBox { allowMultipleSelection: true } }

    // A TestCase is invisible, and so would be a list parented to it, receiving no input
    Window
    {
        id: window
        width: 300
        height: 400
        visible: true
    }

    property var listBox: null

    function init()
    {
        listBox = listBoxComponent.createObject(window.contentItem,
            {model: ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l"], width: 200, height: 400});
        tryVerify(() => listBox.itemAt(listBox.count - 1) !== null);
    }

    function cleanup()
    {
        listBox.destroy();
        listBox = null;
        wait(0);
    }

    function clickRow(index, modifiers = Qt.NoModifier)
    {
        mouseClick(listBox.itemAt(index), undefined, undefined, Qt.LeftButton, modifiers);
    }

    function verifySelection(indices)
    {
        compare(listBox.selectedIndices.slice(), indices);

        for(let index = 0; index < listBox.count; index++)
        {
            compare(listBox.itemAt(index).background.visible, indices.includes(index),
                "Row " + index + " highlight");
        }
    }

    function test_controlClickTogglesRows()
    {
        clickRow(0);
        clickRow(2, Qt.ControlModifier);
        verifySelection([0, 2]);

        clickRow(0, Qt.ControlModifier);
        verifySelection([2]);
    }

    function test_shiftClickExtendsSelection()
    {
        clickRow(0);
        clickRow(2, Qt.ControlModifier);
        clickRow(4, Qt.ShiftModifier);
        verifySelection([0, 2, 3, 4]);
    }

    function test_selectionIsInNumericOrder()
    {
        clickRow(10);
        clickRow(2, Qt.ControlModifier);
        verifySelection([2, 10]);
        compare(listBox.selectedIndex, 10);
    }

    function test_clickReplacesSelection()
    {
        clickRow(0);
        clickRow(2, Qt.ControlModifier);
        clickRow(3);
        verifySelection([3]);
    }
}
