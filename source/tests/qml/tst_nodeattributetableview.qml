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
    property var tableView: null
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
        tableView = findAll(view, item => item instanceof TableView && item.syncView === headerView)[0];
        horizontalScrollBar = findAll(view, item => item instanceof ScrollBar && item.orientation === Qt.Horizontal)[0];
        verify(headerView !== undefined);
        verify(tableView !== undefined);
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

    function columnNames()
    {
        return view.columnSelectionMode ? view.columnOrder :
            view.columnOrder.filter(columnName => !view.hiddenColumns.includes(columnName));
    }

    function leftColumnName() { return columnNames()[headerView.leftColumn]; }
    function leftColumnOffset() { return headerView.contentX - headerItem(headerView.leftColumn).x; }

    function setColumnSelectionMode(columnSelectionMode)
    {
        view.columnSelectionMode = columnSelectionMode;
        wait(100);
    }

    function clickCheckBox(columnName)
    {
        let item = headerItem(columnNames().indexOf(columnName));
        verify(item !== null, columnName + " isn't loaded");

        let checkBox = findAll(item, child => child instanceof CheckBox)[0];
        verify(checkBox !== undefined);
        let wasChecked = checkBox.checked;
        mouseClick(checkBox);
        verify(checkBox.checked !== wasChecked, columnName + " wasn't toggled");
    }

    function verifyInView(columnName)
    {
        let item = headerItem(columnNames().indexOf(columnName));
        verify(item !== null, columnName + " isn't loaded");
        verify(item.x >= headerView.contentX - 0.5 &&
            item.x + item.width <= headerView.contentX + headerView.width + 0.5, columnName + " isn't in view");
    }

    function test_columnSelectionKeepsScrollPosition()
    {
        view.setHiddenColumns(model.columnNames.filter((name, index) => index % 2 === 0));
        wait(100);

        horizontalScrollBar.position = 0.4;
        wait(100);

        let columnName = leftColumnName();
        let offset = leftColumnOffset();
        verify(offset > 0);

        setColumnSelectionMode(true);
        compare(leftColumnName(), columnName);
        fuzzyCompare(leftColumnOffset(), offset, 0.5);

        setColumnSelectionMode(false);
        compare(leftColumnName(), columnName);
        fuzzyCompare(leftColumnOffset(), offset, 0.5);
    }

    function test_columnSelectionKeepsScrollPositionWhenEarlierColumnsAreHidden()
    {
        view.setHiddenColumns(model.columnNames.slice(0, 10));
        wait(100);

        dragScrollBarToRight();

        let columnName = leftColumnName();
        let offset = leftColumnOffset();

        setColumnSelectionMode(true);
        compare(leftColumnName(), columnName);
        fuzzyCompare(leftColumnOffset(), offset, 0.5);

        setColumnSelectionMode(false);
        compare(leftColumnName(), columnName);
        fuzzyCompare(leftColumnOffset(), offset, 0.5);
    }

    function test_columnSelectionKeepsNearestColumnWhenLeftmostIsHidden()
    {
        setColumnSelectionMode(true);
        horizontalScrollBar.position = 0.3;
        wait(100);

        let allColumnNames = columnNames();
        let column = headerView.leftColumn;
        view.setColumnVisibility(allColumnNames[column], false);

        setColumnSelectionMode(false);
        compare(leftColumnName(), allColumnNames[column + 1]);
        fuzzyCompare(leftColumnOffset(), 0, 0.5);
    }

    function test_columnSelectionKeepsLastShownColumnInView()
    {
        let lastColumnName = model.columnNames[model.columnNames.length - 1];
        view.setHiddenColumns([lastColumnName]);
        wait(100);

        setColumnSelectionMode(true);
        dragScrollBarToRight();
        clickCheckBox(lastColumnName);
        dragScrollBarToLeft();

        setColumnSelectionMode(false);
        verifyInView(lastColumnName);
        verifyAtRightEnd();
    }

    function test_columnSelectionKeepsShownColumnInViewWhenMostAreHidden()
    {
        view.setHiddenColumns(model.columnNames.slice(5));
        wait(100);

        setColumnSelectionMode(true);
        horizontalScrollBar.position = 0.85 * (1 - horizontalScrollBar.size);
        wait(100);

        // The rightmost column that can be clicked
        let column = headerView.rightColumn;
        let item = headerItem(column);
        if(item.x + item.width > headerView.contentX + headerView.width)
            column--;

        let columnName = columnNames()[column];
        clickCheckBox(columnName);

        setColumnSelectionMode(false);
        verifyInView(columnName);
    }

    function test_columnSelectionKeepsScrollPositionWhenLastShownColumnIsInView()
    {
        let columnName = model.columnNames[1];
        view.setHiddenColumns([columnName]);
        wait(100);

        setColumnSelectionMode(true);
        clickCheckBox(columnName);

        setColumnSelectionMode(false);
        verifyAtLeftEnd();
        verifyInView(columnName);
    }

    function test_columnSelectionLeavesNoSpaceAfterLastColumn()
    {
        setColumnSelectionMode(true);
        dragScrollBarToRight();

        let allColumnNames = columnNames();
        for(let column = headerView.leftColumn + 1; column < allColumnNames.length - 1; column++)
            view.setColumnVisibility(allColumnNames[column], false);

        setColumnSelectionMode(false);
        verifyAtRightEnd();
    }

    function test_columnSelectionKeepsScrollPositionWithRows()
    {
        testHelper.selectAllTableNodes(model);
        tryVerify(() => view.rowCount > 0);

        test_columnSelectionKeepsScrollPosition();
        fuzzyCompare(tableView.contentX, headerView.contentX, 0.5, "The rows aren't scrolled with the header");
    }
}
