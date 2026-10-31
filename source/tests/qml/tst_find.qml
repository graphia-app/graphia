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

import Graphia

TestCase
{
    name: "Find"
    when: windowShown

    // A layout gives the multiple value list no size while it's invisible
    Window
    {
        id: window
        width: 800
        height: 400
        visible: true
    }

    ListModel
    {
        id: attributesModel
        ListElement { name: "Cluster"; display: "Cluster"; searchable: true; hasSharedValues: true }
    }

    // Stands in for Document, providing what Find uses; the attribute's values
    // are shown differently to how they're stored, as when translated
    QtObject
    {
        id: document

        property int foundIndex: -1
        property int numNodesFound: 0
        property bool nodesMaskActive: false

        signal commandsFinished()
        signal graphChanged(var graph, bool changeOccurred)

        property string findTerm: ""

        function resetFind() {}
        function find(term, options, attributeNames, selectStyle) { findTerm = term; }
        function selectByAttributeValue(attributeName, term) {}

        function availableAttributesModel(elementType) { return attributesModel; }
        function attribute(attributeName) { return {sharedValues: ["Cluster 1", "Cluster 2"]}; }
        function attributeDisplayValue(attributeName, value) { return value.replace("Cluster", "Grappe"); }
    }

    Component { id: findComponent; Find {} }

    property var find: null

    function findAll(item, predicate, found = [])
    {
        if(predicate(item))
            found.push(item);

        for(let child of item.children)
            findAll(child, predicate, found);

        return found;
    }

    function selectMultipleAction()
    {
        let actions = findAll(find, item => item instanceof AbstractButton && item.action !== null &&
            item.action.checkable && item.action.text === qsTr("Select Multiple"));
        verify(actions.length === 1);

        return actions[0].action;
    }

    function valueComboBox()
    {
        // The other ComboBoxes list attributes, by name
        let comboBoxes = findAll(find, item => item instanceof ComboBox && item.valueRole !== "name");
        verify(comboBoxes.length === 1);

        return comboBoxes[0];
    }

    function init()
    {
        document.findTerm = "";
        find = findComponent.createObject(window.contentItem, {document: document});
        selectMultipleAction().checked = false;
        find.show(Find.ByAttribute);
    }

    function cleanup()
    {
        selectMultipleAction().checked = false;
        find.destroy();
        find = null;
        wait(0);
    }

    function test_valuesAreDisplayed()
    {
        let comboBox = valueComboBox();

        compare(comboBox.count, 2);
        compare(comboBox.textAt(0), "Grappe 1");
        compare(comboBox.textAt(1), "Grappe 2");
        compare(comboBox.displayText, "Grappe 1");
    }

    function test_findsDisplayedValue()
    {
        compare(document.findTerm, "^Grappe 1$");

        valueComboBox().currentIndex = 1;
        document.commandsFinished();
        compare(document.findTerm, "^Grappe 2$");
    }

    function test_multipleValuesAreDisplayed()
    {
        selectMultipleAction().checked = true;

        let visibleCheckBoxes = () => findAll(find, item => item instanceof CheckBox && item.visible);
        tryVerify(() => visibleCheckBoxes().length === 2);

        let texts = visibleCheckBoxes().map(checkBox => checkBox.text);
        compare(texts.sort(), ["Grappe 1", "Grappe 2"]);
    }
}
