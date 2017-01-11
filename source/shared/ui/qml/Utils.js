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
