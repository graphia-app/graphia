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
import QtTest

import Graphia.Utils

// The proxy model QML lists use to sort and filter with JavaScript expressions
TestCase
{
    name: "SortFilterProxyModel"

    Component
    {
        id: listModelComponent

        ListModel
        {
            ListElement { name: "b"; score: 2; kind: "x" }
            ListElement { name: "a"; score: 3; kind: "y" }
            ListElement { name: "c"; score: 1; kind: "x" }
            ListElement { name: "d"; score: 2; kind: "y" }
        }
    }

    Component { id: proxyComponent; SortFilterProxyModel {} }

    property var source: null
    property var proxy: null

    function init()
    {
        source = createTemporaryObject(listModelComponent, this);
        // A ListModel's roles are numbered from 0, so one of them is Qt::DisplayRole, which the
        // proxy otherwise sorts by; sort by name unless an expression is given
        proxy = createTemporaryObject(proxyComponent, this, {sourceModel: source, sortRoleName: "name"});
    }

    function names()
    {
        let list = [];
        for(let i = 0; i < proxy.count; i++)
            list.push(proxy.get(i, "name"));

        return list;
    }

    function byScore(left, right)
    {
        return source.data(left, proxy.role("score")) < source.data(right, proxy.role("score"));
    }

    function test_sortExpression()
    {
        compare(names(), ["a", "b", "c", "d"]);

        proxy.sortExpression = byScore;
        compare(proxy.sortExpression, byScore);

        // Rows that compare equal stay in the order they're in, in either order
        compare(names(), ["c", "b", "d", "a"]);
        proxy.ascendingSortOrder = false;
        compare(names(), ["a", "b", "d", "c"]);
        proxy.ascendingSortOrder = true;

        // Rows added to the source are sorted in
        source.append({name: "e", score: 0, kind: "y"});
        compare(names(), ["e", "c", "b", "d", "a"]);

        proxy.sortExpression = undefined;
        compare(names(), ["a", "b", "c", "d", "e"]);
    }

    function test_filterExpression()
    {
        let isX = function(row) { return source.get(row).kind === "x"; };
        proxy.filterExpression = isX;
        compare(proxy.filterExpression, isX);

        compare(names(), ["b", "c"]);
        compare(proxy.count, 2);

        // Rows added to the source are filtered too
        source.append({name: "e", score: 0, kind: "x"});
        source.append({name: "f", score: 0, kind: "y"});
        compare(names(), ["b", "c", "e"]);

        source.setProperty(1, "kind", "x");
        compare(names(), ["a", "b", "c", "e"]);

        proxy.filterExpression = null;
        compare(proxy.count, 6);
    }

    function test_sortedAndFiltered()
    {
        proxy.sortExpression = byScore;
        proxy.filterExpression = function(row) { return source.get(row).score >= 2; };
        compare(names(), ["b", "d", "a"]);
    }

    // An expression that throws rejects every row, or compares nothing as less than anything
    // else, which leaves the rows in the order of the source
    function test_throwingExpressions()
    {
        proxy.filterExpression = function() { throw new Error("filter"); };
        compare(proxy.count, 0);
        proxy.filterExpression = null;

        proxy.sortExpression = function() { throw new Error("sort"); };
        compare(names(), ["b", "a", "c", "d"]);
    }

    function test_notAnExpression()
    {
        proxy.sortExpression = byScore;

        ignoreWarning(/setSortExpression called on non-expression/);
        proxy.sortExpression = 1;
        compare(proxy.sortExpression, byScore);

        ignoreWarning(/filterExpression called on non-expression/);
        proxy.filterExpression = "x";
        compare(proxy.count, 4);
    }
}
