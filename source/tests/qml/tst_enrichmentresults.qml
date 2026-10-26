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
import QtTest

import Graphia
import Graphia.Controls

TestCase
{
    name: "EnrichmentResults"

    // Stands in for Document, providing what EnrichmentResults uses
    QtObject
    {
        id: document

        property string title: "Test.graphia"
        function attributeDisplayName(attributeName) { return attributeName; }
    }

    Component { id: enrichmentResultsComponent; EnrichmentResults {} }

    property var models: []
    property var window: null

    function init()
    {
        models = [testHelper.enrichmentTableModel(), testHelper.enrichmentTableModel()];
        window = enrichmentResultsComponent.createObject(null, {models: models, document: document});
    }

    function cleanup()
    {
        window.destroy();
        window = null;

        for(let model of models)
            model.destroy();

        models = [];
        wait(0);
    }

    function findItem(item, type)
    {
        if(item instanceof type)
            return item;

        for(let child of item.children)
        {
            let found = findItem(child, type);
            if(found !== null)
                return found;
        }

        return null;
    }

    function test_sortIndicatorFollowsTabs()
    {
        let tabBar = findItem(window.contentItem, TabBar);
        let table = findItem(window.contentItem, DataTable);
        verify(tabBar !== null);
        verify(table !== null);

        compare(tabBar.currentIndex, 0);
        table.sortIndicatorOrder = Qt.DescendingOrder;
        verify(!models[0].ascendingSortOrder);

        tabBar.currentIndex = 1;
        compare(table.sortIndicatorOrder, Qt.AscendingOrder);
        verify(models[1].ascendingSortOrder);

        tabBar.currentIndex = 0;
        compare(table.sortIndicatorOrder, Qt.DescendingOrder);
        verify(!models[0].ascendingSortOrder);
    }
}
