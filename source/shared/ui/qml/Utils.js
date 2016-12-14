function watchPropertyChanges(object, handler)
{
    for(var prop in object)
    {
        if(prop.match(".*Changed$"))
            object[prop].connect(handler);
    }
}

function isNumeric(text)
{
    return !isNaN(parseFloat(text)) && isFinite(text);
}

