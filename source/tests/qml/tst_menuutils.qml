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


import QtQuick
import QtQuick.Controls
import QtTest

import Graphia.Controls
import Graphia.Utils

TestCase
{
    name: "MenuUtils"

    Component { id: menuComponent; PlatformMenu {} }
    Component { id: signalSpyComponent; SignalSpy {} }
    Component { id: buttonGroupComponent; ButtonGroup {} }

    // Each test gets its own menus, so nothing is left over from earlier tests
    property var from: null
    property var to: null

    Action { id: actionA; text: "A"; checkable: true }
    Action { id: actionB; text: "B"; checkable: true }

    SignalSpy { id: actionASpy; target: actionA; signalName: "triggered" }

    function init()
    {
        from = createTemporaryObject(menuComponent, this, {title: "From"});
        to = createTemporaryObject(menuComponent, this);
        actionASpy.clear();
        actionA.checked = false;
        actionB.checked = false;
    }

    function test_add()
    {
        let item = MenuUtils.addItemTo(from, "Item");
        MenuUtils.addSeparatorTo(from);
        let actionItem = MenuUtils.addActionTo(from, actionA);
        let subMenu = MenuUtils.addSubMenuTo(from, "Sub Menu");

        compare(from.count, 4);
        compare(from.itemAt(0), item);
        compare(item.text, "Item");
        verify(from.itemAt(1) instanceof PlatformMenuSeparator);
        compare(from.itemAt(2), actionItem);
        compare(actionItem.action, actionA);
        compare(actionItem.text, "A");
        compare(from.itemAt(3).subMenu, subMenu);
        compare(subMenu.title, "Sub Menu");

        MenuUtils.clear(from);
        compare(from.count, 0);
    }

    function test_clone()
    {
        let item = MenuUtils.addItemTo(from, "Item");
        let disabledItem = MenuUtils.addItemTo(from, "Disabled");
        disabledItem.enabled = false;
        MenuUtils.addSeparatorTo(from);
        MenuUtils.addActionTo(from, actionA);
        let subMenu = MenuUtils.addSubMenuTo(from, "Sub Menu");
        MenuUtils.addItemTo(subMenu, "Sub Item");

        // Leave something in the target, which should be replaced
        MenuUtils.addItemTo(to, "Stale");

        MenuUtils.clone(from, to);

        compare(to.title, "From");
        compare(to.count, 5);
        compare(to.itemAt(0).text, "Item");
        compare(to.itemAt(1).text, "Disabled");
        verify(!to.itemAt(1).enabled);
        verify(to.itemAt(2) instanceof PlatformMenuSeparator);
        compare(to.itemAt(3).text, "A");
        verify(to.itemAt(3).checkable);

        let toSubMenu = to.itemAt(4).subMenu;
        verify(toSubMenu !== null);
        compare(toSubMenu.title, "Sub Menu");
        compare(toSubMenu.count, 1);
        compare(toSubMenu.itemAt(0).text, "Sub Item");
    }

    function test_cloneFollowsChanges()
    {
        let item = MenuUtils.addItemTo(from, "Item");
        MenuUtils.addActionTo(from, actionA);

        MenuUtils.clone(from, to);

        from.title = "Changed Title";
        compare(to.title, "Changed Title");

        item.text = "Changed";
        compare(to.itemAt(0).text, "Changed");

        item.enabled = false;
        verify(!to.itemAt(0).enabled);

        from.enabled = false;
        verify(!to.enabled);

        actionA.checked = true;
        verify(to.itemAt(1).checked);
    }

    function test_cloneTriggers()
    {
        let item = MenuUtils.addItemTo(from, "Item");
        MenuUtils.addActionTo(from, actionA);

        let itemSpy = createTemporaryObject(signalSpyComponent, this,
            {target: item, signalName: "triggered"});

        MenuUtils.clone(from, to);

        to.itemAt(0).triggered();
        compare(itemSpy.count, 1);

        to.itemAt(1).triggered();
        compare(actionASpy.count, 1);
        verify(actionA.checked);

        // The clone's bindings survive it being triggered
        actionA.checked = false;
        verify(!to.itemAt(1).checked);
    }

    function test_cloneButtonGroups()
    {
        let a = MenuUtils.addActionTo(from, actionA);
        let b = MenuUtils.addActionTo(from, actionB);

        let group = createTemporaryObject(buttonGroupComponent, this);
        a.ButtonGroup.group = group;
        b.ButtonGroup.group = group;

        MenuUtils.clone(from, to);

        let toA = to.itemAt(0);
        let toB = to.itemAt(1);

        // The clones are put in a group of their own, so only one can be checked
        verify(toA.ButtonGroup.group !== null);
        verify(toA.ButtonGroup.group !== group);
        compare(toB.ButtonGroup.group, toA.ButtonGroup.group);

        toA.checked = true;
        toB.checked = true;
        verify(!toA.checked);

        toA.checked = true;
        verify(!toB.checked);
    }

    // Count the objects destroyed, without keeping references to them
    function destroyedAfterClear(populate)
    {
        let destroyed = {count: 0};
        let objects = populate();
        objects.forEach(o => o.Component.destruction.connect(() => destroyed.count++));
        objects = null;

        MenuUtils.clear(to);
        wait(0);

        return destroyed.count;
    }

    function test_clearDestroysItems()
    {
        compare(destroyedAfterClear(() =>
        {
            let item = MenuUtils.addItemTo(to, "Item");
            let separator = MenuUtils.addSeparatorTo(to) ?? to.itemAt(1);
            let actionItem = MenuUtils.addActionTo(to, actionA);
            let subMenu = MenuUtils.addSubMenuTo(to, "Sub Menu");
            let subItem = MenuUtils.addItemTo(subMenu, "Sub Item");

            return [item, separator, actionItem, subMenu, subItem];
        }), 5);

        compare(to.count, 0);
    }

    function test_clearDestroysClones()
    {
        let a = MenuUtils.addActionTo(from, actionA);
        let b = MenuUtils.addActionTo(from, actionB);
        let group = createTemporaryObject(buttonGroupComponent, this);
        a.ButtonGroup.group = group;
        b.ButtonGroup.group = group;
        MenuUtils.addSubMenuTo(from, "Sub Menu");

        compare(destroyedAfterClear(() =>
        {
            MenuUtils.clone(from, to);

            let objects = [];
            for(let i = 0; i < to.count; i++)
                objects.push(to.itemAt(i));

            return objects;
        }), 3);

        // Cloning again replaces the earlier clone, which is destroyed,
        // along with the group created for it
        MenuUtils.clone(from, to);
        let count = {value: 0};
        to.itemAt(0).Component.destruction.connect(() => count.value++);
        to.itemAt(0).ButtonGroup.group.Component.destruction.connect(() => count.value++);
        MenuUtils.clone(from, to);
        wait(0);
        compare(count.value, 2);
        compare(to.count, 3);
    }
}
