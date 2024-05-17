/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

import Graphia.Utils

Preferences
{
    section: "misc"

    property string templates

    function asArray()
    {
        let a = [];

        if(templates.length > 0)
        {
            try { a = JSON.parse(templates); }
            catch(e) { a = []; }
        }

        return a;
    }

    function namesAsArray()
    {
        return asArray().map(e => e.name).sort(NativeUtils.compareStrings);
    }

    function remove(names)
    {
        let a = asArray().filter(e => names.indexOf(e.name) < 0);
        templates = JSON.stringify(a);
    }

    function rename(from, to)
    {
        let a = asArray();
        let foundElement = a.find(e => e.name === from);

        if(!foundElement)
        {
            console.log("renameTemplate: could not find template named " + from);
            return;
        }

        foundElement.name = to;
        templates = JSON.stringify(a);
    }

    function add(name, method, transforms, visualisations)
    {
        let a = asArray();
        let element = a.find(e => e.name === name);
        if(!element)
            element = a[a.push({}) - 1];

        element.name = name;
        element.method = method;
        element.transforms = transforms;
        element.visualisations = visualisations;

        templates = JSON.stringify(a);
    }

    function templateFor(name)
    {
        return asArray().find(e => e.name === name);
    }
}
