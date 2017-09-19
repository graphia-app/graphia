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
    text = text.replace(/\s+/g, " ");
    return text;
}

function escapeQuotes(text)
{
    text = text.replace(/\"/g, "\\\"");
    text = "\"" + text + "\"";
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
        return 0.0001;
    else if(range <= 0.01)
        return 0.001;
    else if(range <= 1.0)
        return 0.01;
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

function numDecimalsAfterPoint(value)
{
    if(Math.floor(value) === value)
        return 0;

    return value.toString().split(".")[1].length || 0;
}

function superScriptValue(value)
{
    if(!isNumeric(value))
        return value;

    var superScriptDigits = "⁰¹²³⁴⁵⁶⁷⁸⁹";

    // Make sure it's a string
    value = value.toString();

    // Give up if not an integer
    if(!value.match(/[0-9]*/))
        return value;

    var superScriptString = "";

    for(var i = 0; i < value.length; i++)
    {
        var index = parseInt(value[i]);
        superScriptString += superScriptDigits[index];
    }

    return superScriptString;
}

function formatForDisplay(value, minDecimalPlaces, maxDecimalPlaces, scientificNotationDigitsThreshold)
{
    if(!isNumeric(value))
        return value;

    minDecimalPlaces = (typeof minDecimalPlaces !== "undefined") ?
        minDecimalPlaces : 0;

    maxDecimalPlaces = (typeof maxDecimalPlaces !== "undefined") ?
        maxDecimalPlaces : decimalPointsForValue(value);

    if(maxDecimalPlaces < minDecimalPlaces)
        maxDecimalPlaces = minDecimalPlaces;

    scientificNotationDigitsThreshold = (typeof scientificNotationDigitsThreshold !== "undefined") ?
        scientificNotationDigitsThreshold : 5;

    // String to float
    value = parseFloat(value);

    var threshold = Math.pow(10, scientificNotationDigitsThreshold);
    if(value >= threshold || value <= -threshold)
    {
        var exponential = value.toExponential(2);
        var mantissaAndExponent = exponential.split("e");

        // 1.20 -> 1.2
        var mantissa = parseFloat(mantissaAndExponent[0]);

        // +123 -> 123
        var exponent = parseInt(mantissaAndExponent[1]);

        return mantissa + "×10" + superScriptValue(exponent);
    }

    // 1.234567... -> 1.234
    var truncated = value.toFixed(maxDecimalPlaces);

    // 1.100 -> 1.1
    var simplified = parseFloat(truncated);

    // 1 -> 1.1
    if(minDecimalPlaces > 0 && numDecimalsAfterPoint(simplified) < minDecimalPlaces)
        simplified = simplified.toFixed(minDecimalPlaces);

    return simplified.toString();
}

// http://stackoverflow.com/questions/9461621
function formatUsingSIPostfix(num)
{
    var si =
        [
            { value: 1E9,  symbol: "G" },
            { value: 1E6,  symbol: "M" },
            { value: 1E3,  symbol: "k" }
        ], i;

    for(i = 0; i < si.length; i++)
    {
        if(num >= si[i].value * 100)
            return (num / si[i].value).toFixed(1).replace(/\.?0+$/, "") + si[i].symbol;
    }

    return num;
}

function desaturate(colorString, factor)
{
    var c = Qt.darker(colorString, 1.0);
    return Qt.hsla(c.hslHue, c.hslSaturation * factor, c.hslLightness, c.a);
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

    for(var index = 0; index < from.items.length; index++)
    {
        var fromItem = from.items[index];
        var toItem = null;

        switch(fromItem.type)
        {
        case MenuItemType.Item:
            toItem = to.addItem(fromItem.text);
            break;

        case MenuItemType.Menu:
            toItem = to.addMenu(fromItem.title);
            cloneMenu(fromItem, toItem);
            break;

        case MenuItemType.Separator:
            to.addSeparator();
            break;
        }

        if(toItem === null)
            continue;

        var properties = [// Note "action" is specifcally skipped because
                          //   a) the properties it proxies are bound anyway
                          //   b) binding it will cause loops
                          "checkable", "checked", "enabled",
                          "exclusiveGroup", "iconName", "iconSource",
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

        toItem.triggered.connect(function(fromItem)
        {
            return function()
            {
                fromItem.trigger();
            };
        }(fromItem));
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
}
