/*
 * Copyright © 2013-2020 Graphia Technologies Ltd.
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

.import QtQuick.Controls 1.5 as QtQuickControls

function watchPropertyChanges(object, handler)
{
    for(var prop in object)
    {
        if(prop.match(".*Changed$"))
            object[prop].connect(handler);
    }
}

function objectsEquivalent(a, b)
{
    var aProps = Object.getOwnPropertyNames(a);
    var bProps = Object.getOwnPropertyNames(b);

    if(aProps.length !== bProps.length)
        return false;

    for(var i = 0; i < aProps.length; i++)
    {
        var propName = aProps[i];

        if(a[propName] !== b[propName])
            return false;
    }

    return true;
}

function isNumeric(value)
{
    return !isNaN(parseFloat(value)) && isFinite(value);
}

function isInt(value)
{
    if(isNaN(value))
        return false;

    var x = parseFloat(value);
    return (x | 0) === x;
}

function castToBool(value)
{
    switch(typeof value)
    {
        case 'boolean':
            return value;
        case 'string':
            return value.toLowerCase() === 'true';
        case 'number':
            return value !== 0;
        default:
            return false;
    }
}

function printStackTrace()
{
    var err = new Error();
    var elements = err.stack.split("\n");
    elements.splice(0, 1);
    elements = elements.map(function(e) { return "    " + e; });
    var trace = "Stack trace:\n" + elements.join("\n");
    console.log(trace);
}

function normaliseWhitespace(text)
{
    text = text.replace(/\s+/g, " ");
    return text;
}

function escapeQuotes(text)
{
    text = text.replace(/\"/g, "\\\"");
    return text;
}

function unescapeQuotes(text)
{
    // [\s\S] is like . except it matches \n
    var re = /^\s*"([\s\S]*)"\s*$/;

    // Strip off enclosing (non-escaped) quotes, if present
    if(text.match(re))
        text = text.replace(re, "$1");

    text = text.replace(/\\"/g, "\"");
    return text;
}

function addSlashes(text)
{
    text = JSON.stringify(String(text));
    text = text.substring(1, text.length - 1);

    return text;
}

function sanitiseJson(text)
{
    try
    {
        var o = JSON.parse(text);
        text = JSON.stringify(o);
    }
    catch(e)
    {
        // It's not JSON
    }

    return text;
}

function decimalPointsForValue(value)
{
    if(value <= 0.001)
        return 5;
    else if(value <= 0.01)
        return 4;
    else if(value <= 1.0)
        return 3;
    else if(value <= 100.0)
        return 2;
    else if(value <= 1000.0)
        return 1;

    return 0;
}

function decimalPointsForRange(min, max)
{
    return decimalPointsForValue(max - min);
}

function incrementForRange(min, max)
{
    var range = max - min;

    if(range <= 0.001)
        return 0.00001;
    else if(range <= 0.01)
        return 0.0001;
    else if(range <= 1.0)
        return 0.001;
    else if(range <= 100.0)
        return 0.1;
    else if(range <= 1000.0)
        return 10.0;
    else if(range <= 10000.0)
        return 100.0;
    else if(range <= 100000.0)
        return 1000.0;

    return 100000.0;
}

function desaturate(colorString, factor)
{
    var c = Qt.darker(colorString, 1.0);
    return Qt.hsla(c.hslHue, c.hslSaturation * factor, c.hslLightness, c.a);
}

function generateColorFrom(color)
{
    if(typeof(color) === "string")
        color = Qt.lighter(color, 1.0);

    var goldenAngle = 137.5 / 360.0;
    var hue = color.hsvHue;

    if(hue >= 0.0)
    {
        hue += goldenAngle;
        if(hue > 1.0)
            hue -= 1.0;

        color.hsvHue = hue;

        return color.toString();
    }

    return "#FF0000";
}

function pluralise(count, singular, plural)
{
    if(count === 1)
        return "1 " + singular;

    return count + " " + plural;
}

// Clone one menu into another, such that to is a "proxy" for from that looks
// identical to from, and uses from's behaviour
function cloneMenu(from, to)
{
    // Clear out any existing items
    while(to.items.length > 0)
        to.removeItem(to.items[0]);

    var exclusiveGroups = {};

    for(var index = 0; index < from.items.length; index++)
    {
        var fromItem = from.items[index];
        var toItem = null;

        switch(fromItem.type)
        {
        case QtQuickControls.MenuItemType.Item:
            toItem = to.addItem(fromItem.text);
            break;

        case QtQuickControls.MenuItemType.Menu:
            toItem = to.addMenu(fromItem.title);
            cloneMenu(fromItem, toItem);
            break;

        case QtQuickControls.MenuItemType.Separator:
            to.addSeparator();
            break;
        }

        if(toItem === null)
            continue;

        var properties = [// Note "action" is specifcally skipped because
                          //   a) the properties it proxies are bound anyway
                          //   b) binding it will cause loops
                          "checkable", "checked", "enabled",
                          "iconName", "iconSource",
                          "shortcut", "text", "visible"];

        properties.forEach(function(prop)
        {
            if(fromItem[prop] !== undefined)
            {
                toItem[prop] = Qt.binding(function(fromItem, prop)
                {
                    return function()
                    {
                        return fromItem[prop];
                    };
                }(fromItem, prop));
            }
        });

        // Store a list of ExclusiveGroups so that we can recreate them
        // in the target menu, later
        if(fromItem.exclusiveGroup !== undefined && fromItem.exclusiveGroup !== null)
        {
            var key = fromItem.exclusiveGroup.toString();

            if(exclusiveGroups[key] === undefined)
                exclusiveGroups[key] = [];

            exclusiveGroups[key].push(toItem);
        }

        if(toItem.triggered !== undefined)
        {
            toItem.triggered.connect(function(fromItem)
            {
                return function()
                {
                    fromItem.trigger();
                };
            }(fromItem));
        }
    }

    // Create new ExclusiveGroups which correspond to the source menu's ExclusiveGroups
    for(key in exclusiveGroups)
    {
        var fromExclusiveGroup = exclusiveGroups[key];
        var toExclusiveGroup = Qt.createQmlObject('import QtQuick.Controls 1.5; ExclusiveGroup {}', to);

        fromExclusiveGroup.forEach(function(menuItem)
        {
            menuItem.exclusiveGroup = toExclusiveGroup;
        });
    }
}

function setContains(set, value)
{
    if(typeof(set) !== "array" && typeof(set) !== "object")
        return false;

    return set.indexOf(value) > -1;
}

function setAdd(set, value)
{
    if(typeof(set) !== "array" && typeof(set) !== "object")
    {
        console.log("Utils.setAdd passed non-array");
        return;
    }

    var found = setContains(set, value);

    if(!found)
        set.push(value);

    return set;
}

function setRemove(set, value)
{
    if(typeof(set) !== "array" && typeof(set) !== "object")
    {
        console.log("Utils.setAdd passed non-array");
        return;
    }

    var index = set.indexOf(value);

    if(index > -1)
        set.splice(index, 1);

    return set;
}

function setIntersection(a, b)
{
    var result = [];

    a.forEach(function(value)
    {
        if(setContains(b, value))
            result.push(value);
    });

    return result;
}

function readFile(file)
{
    var allText = "";

    var rawFile = new XMLHttpRequest();
    rawFile.open("GET", file, false);
    rawFile.onreadystatechange = function()
    {
        if(rawFile.readyState === 4 && rawFile.status === 200 || rawFile.status === 0)
            allText = rawFile.responseText;
    }
    rawFile.send(null);

    return allText;
}

// Escape a text value so it will match literally in a regex
function regexEscape(text)
{
    return text.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&');
}
