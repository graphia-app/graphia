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

.import "../../../../shared/ui/qml/Utils.js" as Utils
.import app.graphia 1.0 as Graphia

function decorateAttributeName(name, parameter)
{
    name = name.replace(/([^\.]+\.)?(.*)/, "$1\"$2\"");
    name += (parameter !== undefined && parameter.length > 0) ?
        (".\"" + parameter + "\"") : "";

    return name;
}

function expressionFor(document, attributeName, flags, type, channelName)
{
    var expression = "";

    if(flags & Graphia.AttributeFlag.VisualiseByComponent)
        expression += "[component] ";

    expression += attributeName + " \"" + channelName +"\"";

    var parameters = document.visualisationDefaultParameters(
        type, channelName);

    if(Object.keys(parameters).length !== 0)
    {
        expression += " with";

        for(var key in parameters)
        {
            var parameter = parameters[key];
            parameter = Utils.sanitiseJson(parameter);
            parameter = Utils.escapeQuotes(parameter);

            expression += " " + key + " = \"" + parameter + "\"";
        }
    }

    return expression;
}
