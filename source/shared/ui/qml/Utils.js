function watchPropertyChanges(object, handler)
{
    for(var prop in object)
    {
        if(prop.match(".*Changed$"))
            object[prop].connect(handler);
    }
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
    text = text.replace(/\\s+/g, " ");
    return text;
}

function unescapeQuotes(text)
{
    // [\s\S] is like . except it matches \n
    var re = /^\s*"([\s\S]*)"\s*$/;

    if(!text.match(re))
        return text;

    text = text.replace(re, "$1");
    text = text.replace(/\\"/g, "\"");
    return text;
}

function decimalPointsForRange(min, max)
{
    var range = max - min;

    if(range <= 100.0)
        return 3;
    else if(range <= 1000.0)
        return 2;
    else if(range <= 10000.0)
        return 1;

    return 0;
}

function roundToDp(value, dp)
{
    if(!isNumeric(value))
        return value;

    return parseFloat(parseFloat(value).toFixed(dp)).toString();
}
