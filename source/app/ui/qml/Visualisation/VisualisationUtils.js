function expressionFor(document, name, flags, type, channelName)
{
    var expression = "";

    if(flags & AttributeFlag.VisualiseByComponent)
        expression += "[component] ";

    expression += "\"" + name + "\" \"" + channelName +"\"";

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
