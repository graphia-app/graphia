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

.import QtQuick.Controls 2.12 as QtQuickControls2

function watchPropertyChanges(object, handler)
{
    for(let prop in object)
    {
        if(prop.match(".*Changed$"))
            object[prop].connect(handler);
    }
}

function objectsEquivalent(a, b)
{
    let aProps = Object.getOwnPropertyNames(a);
    let bProps = Object.getOwnPropertyNames(b);

    if(aProps.length !== bProps.length)
        return false;

    for(let i = 0; i < aProps.length; i++)
    {
        let propName = aProps[i];

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

    let x = parseFloat(value);
    return (x | 0) === x;
}

function clamp(value, min, max)
{
    return Math.min(Math.max(value, min), max);
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
    let err = new Error();
    let elements = err.stack.split("\n");
    elements.splice(0, 1);
    elements = elements.map(function(e) { return "    " + e; });
    let trace = "Stack trace:\n" + elements.join("\n");
    console.log(trace);
}

function callingFunction()
{
    let err = new Error();
    let elements = err.stack.split("\n");
    return elements[2];
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
    let re = /^\s*"([\s\S]*)"\s*$/;

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
        let o = JSON.parse(text);
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
    value = Math.abs(value);

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
    if(!isNumeric(min) || !isNumeric(max) || min > max)
        return 0;

    return decimalPointsForValue(max - min);
}

function incrementForRange(min, max)
{
    let range = Math.abs(max - min);
    let largeFractionOfRange = range * 0.999;
    let nextPowerOfTenSmallerThanRange =
        Math.pow(10.0, Math.floor(Math.log10(largeFractionOfRange)));

    return nextPowerOfTenSmallerThanRange * 0.01;
}

function desaturate(colorString, factor)
{
    let c = Qt.darker(colorString, 1.0);
    return Qt.hsla(c.hslHue, c.hslSaturation * factor, c.hslLightness, c.a);
}

function generateColorFrom(color)
{
    if(typeof(color) === "string")
        color = Qt.lighter(color, 1.0);

    let goldenAngle = 137.5 / 360.0;
    let hue = color.hsvHue;

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

    let found = setContains(set, value);

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

    let index = set.indexOf(value);

    if(index > -1)
        set.splice(index, 1);

    return set;
}

function setIntersection(a, b)
{
    let result = [];

    a.forEach(function(value)
    {
        if(setContains(b, value))
            result.push(value);
    });

    return result;
}

function readFile(file)
{
    let allText = "";

    let rawFile = new XMLHttpRequest();
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

// This is only a shallow value comparison
function arrayCompare(a, b)
{
    if(!Array.isArray(a) || !Array.isArray(b))
        return false;

    if(a.length !== b.length)
        return false;

    let as = a.sort((x, y) => x - y);
    let bs = b.sort((x, y) => x - y);

    let i = a.length;
    while(i--)
    {
        if(as[i] !== bs[i])
            return false;
    }

    return true;
}

function floatCompare(a, b)
{
    // Note the relatively large epsilon
    if(Math.abs(a - b) < 1e-5)
        return 0;

    if(a < b)
        return -1;

    return 1;
}

function lerp(min, max, value)
{
    return min + (value * (max - min));
}

function normalise(min, max, value)
{
    return (value - min) / (max - min);
}