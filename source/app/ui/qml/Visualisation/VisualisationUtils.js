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

    if(flags & AttributeFlag.VisualiseByComponent)
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
