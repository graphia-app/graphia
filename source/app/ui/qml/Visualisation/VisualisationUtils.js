function expressionFor(document, attribute, channelName)
{
    var expression = "";

    if(attribute.flags & AttributeFlag.VisualiseByComponent)
        expression += "[component] ";

    expression += "\"" + attribute.name + "\" \"" + channelName +"\"";

    var parameters = document.visualisationDefaultParameters(
        attribute.valueType, channelName);

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
