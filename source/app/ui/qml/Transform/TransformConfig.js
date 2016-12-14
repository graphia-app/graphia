function createTransformParameter(document, component, parent, parameterData)
{
    if(component === null)
    {
        component = Qt.createComponent("TransformParameter.qml");
        if(component === null)
        {
            console.log("Failed to create parameterComponent");
            return null;
        }
    }

    var object = component.createObject(parent);
    if(object === null)
    {
        console.log("Failed to create parameterObject");
        return null;
    }

    object.configure(parameterData);

    return object;
}

function roundTo3dp(text)
{
    if(!Utils.isNumeric(text))
        return text;

    return parseFloat(parseFloat(text).toFixed(3)).toString();
}

function sanitiseOp(text)
{
    var replacements =
    [
        "==",       "=",
        "!=",       "≠",
        "<=",       "≤",
        ">=",       "≥",
        "&&",       "and",
        "||",       "or",
        "includes", "Includes",
        "excludes", "Excludes",
        "starts",   "Starts With",
        "ends",     "Ends With",
        "matches",  "Matches Regex",
    ];

    for(var i = 0; i < replacements.length; i += 2)
        text = text.replace(replacements[i], replacements[i + 1]);

    return text;
}

function normaliseWhitespace(text)
{
    text = text.replace(/\\s+/g, " ");
    return text;
}

function create(transform)
{
    this.action = transform.action;
    this.metaAttributes = transform.metaAttributes;
    this.template = "\"" + transform.action + "\"";

    this._elements = [];

    function appendToElements(elements, value)
    {
        var last = elements.length - 1;
        if(last >= 0 && typeof value === "string" && typeof elements[last] === typeof value)
            elements[last] += value;
        else
            elements.push(value);
    }

    function appendConditionToElements(elements, condition)
    {
        if(typeof condition.rhs !== "object")
        {
            appendToElements(elements, condition);
            return;
        }

        appendToElements(elements, "(");
        appendConditionToElements(elements, condition.lhs);
        appendToElements(elements, ") " + sanitiseOp(condition.op) + " (");
        appendConditionToElements(elements, condition.rhs);
        appendToElements(elements, ")");
    }

    function conditionToTemplate(condition)
    {
        var lhs = "";
        var rhs = "";

        if(typeof condition.lhs === "object")
            lhs = "(" + conditionToTemplate(condition.lhs) + ")";
        else
            lhs = "\"" + condition.lhs + "\"";

        if(typeof condition.rhs === "object")
            rhs = "(" + conditionToTemplate(condition.rhs) + ")";
        else
            rhs = "%";

        return lhs + " " + condition.op + " " + rhs;
    }

    // Action
    appendToElements(this._elements, transform.action);

    // Parameters
    if(transform.parameters.length > 0)
    {
        this.template += " with";
        appendToElements(this._elements, " with ");

        for(var i = 0; i < transform.parameters.length; i++)
        {
            var parameter = transform.parameters[i];
            this.template += " \"" + parameter.name + "\" = %";
            appendConditionToElements(this._elements, {lhs: parameter.name, op:"=", rhs: parameter.value});
        }
    }

    // Condition
    if(transform.condition !== undefined)
    {
        this.template += " where " + conditionToTemplate(transform.condition);

        appendToElements(this._elements, " where ");
        appendConditionToElements(this._elements, transform.condition);
    }

    this.toComponents =
    function(document, parent)
    {
        var labelText = "";

        function addLabel()
        {
            labelText = labelText.trim();
            Qt.createQmlObject(qsTr("import QtQuick 2.5;" +
                                    "import QtQuick.Controls 1.4;" +
                                    "Label { text: \"%1\"; color: root.textColor }")
                                    .arg(normaliseWhitespace(labelText)), parent);

            labelText = "";
        }

        this.parameters = [];

        for(var i = 0; i < this._elements.length; i++)
        {
            if(typeof this._elements[i] === "string")
                labelText += this._elements[i];
            else if(typeof this._elements[i] === 'object')
            {
                var parameter = this._elements[i];
                var locked = this.metaAttributes.indexOf("locked") >= 0;

                labelText += parameter.lhs + " " + sanitiseOp(parameter.op);

                var parameterData = document.findTransformParameter(this.action, parameter.lhs);
                parameterData.initialValue = parameter.rhs;

                if(locked)
                {
                    if(parameterData.type === FieldType.String)
                        labelText += "\\\"" + parameter.rhs + "\\\"";
                    else
                        labelText += roundTo3dp(parameter.rhs);
                }
                else
                    addLabel();

                var parameterObject = createTransformParameter(document, null,
                    locked ? null : parent, // If locked, still create the object, but don't display it
                    parameterData);

                this.parameters.push(parameterObject);
            }
        }

        if(labelText.length > 0)
            addLabel();
    }
}
