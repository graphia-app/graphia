/*
 * Copyright © 2013-2025 Tim Angus
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

.import Graphia.SharedTypes as GraphiaSharedTypes
.import Graphia.Utils as GraphiaUtils

function expressionFor(document, attributeName, flags, type, channelName)
{
    let expression = "";

    if(flags & GraphiaSharedTypes.AttributeFlag.VisualiseByComponent)
        expression += "[component] ";

    expression += attributeName + " \"" + channelName +"\"";

    let parameters = document.visualisationDefaultParameters(
        type, channelName);

    if(Object.keys(parameters).length !== 0)
    {
        expression += " with";

        for(let key in parameters)
        {
            let parameter = parameters[key];
            parameter = GraphiaUtils.Utils.sanitiseJson(parameter);
            parameter = GraphiaUtils.Utils.escapeQuotes(parameter);

            expression += " " + key + " = \"" + parameter + "\"";
        }
    }

    return expression;
}
