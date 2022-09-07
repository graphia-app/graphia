/*
 * Copyright © 2013-2022 Graphia Technologies Ltd.
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

.import QtQuick.Controls as QtQuickControls
.import app.graphia.Shared.Controls as SharedControls
.import "Utils.js" as Utils

function createQmlItem(itemName, parent)
{
    return Qt.createQmlObject(
        "import QtQuick.Controls\n" +
        "import app.graphia.Shared.Controls\n" +
        itemName + " {}", parent);
}

function addSeparatorTo(menu)
{
    menu.addItem(createQmlItem("PlatformMenuSeparator", menu));
}

function addItemTo(menu, text)
{
    let menuItem = createQmlItem("PlatformMenuItem", menu);

    if(text)
        menuItem.text = text;

    menu.addItem(menuItem);
    return menuItem;
}

function addActionTo(menu, action)
{
    let menuItem = addItemTo(menu, undefined);
    menuItem.action = action;
    return menuItem;
}

function addSubMenuTo(menu, title)
{
    let subMenu = createQmlItem("PlatformMenu", menu);
    subMenu.title = title;
    menu.addMenu(subMenu);
    return subMenu;
}

function clear(menu)
{
    while(menu.count > 0)
        menu.takeItem(0);
}

// Clone one menu into another, such that to is a "proxy" for from that looks
// identical to from, and uses from's behaviour
function clone(from, to)
{
    clear(to);

    to.title = Qt.binding(function(from) { return () => from.title; }(from));
    to.enabled = Qt.binding(function(from) { return () => from.enabled; }(from));

    let buttonGroups = {};

    for(let i = 0; i < from.count; i++)
    {
        let fromItem = from.itemAt(i);

        if(fromItem.subMenu)
        {
            let toSubMenu = createQmlItem("PlatformMenu", to);
            clone(fromItem.subMenu, toSubMenu);
            to.addMenu(toSubMenu);

            Utils.proxyProperties(fromItem, toSubMenu.parent, ["enabled", "height"]);
        }
        else if(fromItem instanceof SharedControls.PlatformMenuItem)
        {
            let toItem = createQmlItem("PlatformMenuItem", to);
            to.addItem(toItem);

            // Note "action" is specifcally skipped because
            //   a) the properties it proxies are bound anyway
            //   b) binding it will cause loops
            Utils.proxyProperties(fromItem, toItem, ["checkable",
                "checked", "enabled", "icon", "text", "hidden"]);

            // Store a list of ButtonGroups so that we can recreate them
            // in the target menu, later
            if(fromItem.QtQuickControls.ButtonGroup.group !== null)
            {
                let key = fromItem.QtQuickControls.ButtonGroup.group.toString();

                if(buttonGroups[key] === undefined)
                    buttonGroups[key] = [];

                buttonGroups[key].push(toItem);
            }

            if(toItem.triggered !== undefined)
            {
                toItem.triggered.connect(function(fromItem)
                {
                    if(fromItem.action !== null)
                        return function() { fromItem.action.trigger(); }

                    return function() { fromItem.triggered(); };
                }(fromItem));
            }
        }
        else if(fromItem instanceof SharedControls.PlatformMenuSeparator)
            addSeparatorTo(to);
    }

    // Create new ButtonGroups which correspond to the source menu's ButtonGroups
    for(let key in buttonGroups)
    {
        let fromButtonGroup = buttonGroups[key];
        let toButtonGroup = createQmlItem("ButtonGroup", to)

        fromButtonGroup.forEach(function(menuItem)
        {
            toButtonGroup.addButton(menuItem);
        });
    }
}
