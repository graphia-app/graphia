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

            // Ideally we'd use Utils.sanitiseJson and Utils.escapeQuotes here,
            // but after Qt 5.13 (I think!) referring to the contents of a .js
            // file from another is proving unpredictably problematic
            try { parameter = JSON.stringify(JSON.parse(parameter)); } catch(e) {}
            parameter = parameter.replace(/\"/g, "\\\"");

            expression += " " + key + " = \"" + parameter + "\"";
        }
    }

    return expression;
}
