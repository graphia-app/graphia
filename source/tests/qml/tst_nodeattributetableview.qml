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
import QtQuick.Controls
import QtQuick.Window
import QtTest

import Graphia.Controls

TestCase
{
    name: "NodeAttributeTableView"
    when: windowShown

    Component { id: tableViewComponent; NodeAttributeTableView {} }

    // A TestCase is invisible, and so would be a view parented to it, receiving no input
    Window
    {
        id: window
        width: 1600
        height: 400
        visible: true
    }

    // Nothing is selected, so the table has columns but no rows
    property var model: null
    property var view: null
    property var headerView: null
    property var horizontalScrollBar: null

    function findAll(item, predicate, found = [])
    {
        if(predicate(item))
            found.push(item);

        for(let child of item.children)
            findAll(child, predicate, found);

        return found;
    }

    function init()
    {
        model = testHelper.nodeAttributeTableModel();
        tryVerify(() => model.columnNames.length > 0);

        view = tableViewComponent.createObject(window.contentItem, {model: model, width: 400, height: 300});

        headerView = findAll(view, item => item instanceof TableView && !item.interactive)[0];
        horizontalScrollBar = findAll(view, item => item instanceof ScrollBar && item.orientation === Qt.Horizontal)[0];
        verify(headerView !== undefined);
        verify(horizontalScrollBar !== undefined);

        tryVerify(() => headerView.columns === model.columnNames.length && headerItem(0) !== null);
    }

    function cleanup()
    {
        view.destroy();
        view = null;
        wait(0);

        model.destroy();
        model = null;
    }

    function headerItem(column) { return headerView.itemAtCell(Qt.point(column, 0)); }

    function verifyAtLeftEnd()
    {
        let item = headerItem(0);
        verify(item !== null, "The first column isn't loaded");
        fuzzyCompare(item.x, headerView.contentX, 0.5, "Space before the first column");
    }

    function verifyAtRightEnd()
    {
        let item = headerItem(headerView.columns - 1);
        verify(item !== null, "The last column isn't loaded");
        fuzzyCompare(item.x + item.width, headerView.contentX + headerView.width, 0.5,
            "The last column doesn't end at the right edge");
    }

    // As holding the scroll bar against an end while the table finds its true width
    function dragScrollBarToLeft()
    {
        for(let i = 0; i < 30; i++)
        {
            horizontalScrollBar.position = 0;
            wait(20);
        }
    }

    function dragScrollBarToRight()
    {
        for(let i = 0; i < 30; i++)
        {
            horizontalScrollBar.position = 1 - horizontalScrollBar.size;
            wait(20);
        }
    }

    function scrollWheel(xDelta)
    {
        for(let i = 0; i < 200; i++)
        {
            mouseWheel(view, view.width * 0.5, view.height * 0.5, xDelta, 0);
            wait(10);
        }

        wait(500);
    }

    function test_scrollBarReachesBothEnds()
    {
        dragScrollBarToRight();
        verifyAtRightEnd();

        dragScrollBarToLeft();
        verifyAtLeftEnd();
    }

    function test_wheelReachesBothEnds()
    {
        scrollWheel(-120);
        verifyAtRightEnd();

        scrollWheel(120);
        verifyAtLeftEnd();
    }

    function test_noSpaceBeforeFirstColumnAfterResizing()
    {
        dragScrollBarToRight();

        // Widen until the last column shows, which moves the columns right
        let lastColumn = headerView.columns - 1;
        do
        {
            view.width += 20;
            wait(20);
        }
        while(headerItem(lastColumn) === null);

        // Then narrow enough to unload it again
        let narrowerWidth = view.width - (headerItem(lastColumn).width + 100);
        while(view.width > narrowerWidth)
        {
            view.width -= 20;
            wait(20);
        }

        // In steps, as when dragged by hand
        while(horizontalScrollBar.position > 0)
        {
            horizontalScrollBar.position = Math.max(horizontalScrollBar.position - 0.01, 0);
            wait(20);
        }

        dragScrollBarToLeft();
        verifyAtLeftEnd();
    }
}
