function createTransformParameter(document, parent, parameterData, onParameterChanged)
{
    var component = Qt.createComponent("TransformParameter.qml");
    if(component === null)
    {
        console.log("Failed to create parameterComponent");
        return null;
    }

    var object = component.createObject(parent);
    if(object === null)
    {
        console.log("Failed to create parameterObject");
        return null;
    }

    object.configure(parameterData);

    if(onParameterChanged !== null)
        object.valueChanged.connect(onParameterChanged);

    return object;
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
        "hasValue", "Has Value",
    ];

    for(var i = 0; i < replacements.length; i += 2)
        text = text.replace(replacements[i], replacements[i + 1]);

    return text;
}

function create(transform)
{
    this.action = transform.action;
    this.flags = transform.flags;
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
        else if(!document.opIsUnary(condition.op))
            rhs = "%";

        return lhs + " " + condition.op + " " + rhs;
    }

    // Action
    appendToElements(this._elements, transform.action);

    // Parameters
    if(Object.keys(transform.parameters).length > 0)
    {
        this.template += " with";
        appendToElements(this._elements, " with ");

        var firstParam = true;

        for(var parameterName in transform.parameters)
        {
            // Put whitespace between the parameters
            if(!firstParam)
                appendToElements(this._elements, " ");
            firstParam = false;

            var parameterValue = transform.parameters[parameterName];
            this.template += " \"" + parameterName + "\" = %";
            appendConditionToElements(this._elements, {lhs: parameterName, op:"=", rhs: parameterValue});
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
    function(document, parent, locked, onParameterChanged)
    {
        var labelText = "";

        function addLabel()
        {
            labelText = labelText.trim();
            Qt.createQmlObject(qsTr("import QtQuick 2.7\n\
                                    import QtQuick.Controls 1.5\n\
                                    Label { text: \"%1\"; color: root.textColor }")
                                    .arg(Utils.normaliseWhitespace(labelText)), parent);

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

                labelText += parameter.lhs + " " + sanitiseOp(parameter.op);

                // No parameter needed
                if(document.opIsUnary(parameter.op))
                    continue;

                var parameterData = document.findTransformParameter(this.action, parameter.lhs);
                parameterData.initialValue = parameter.rhs;

                if(locked)
                {
                    if(parameterData.valueType === ValueType.String)
                        labelText += " \\\"" + parameter.rhs + "\\\"";
                    else
                        labelText += " " + Utils.formatForDisplay(parameter.rhs);
                }
                else
                    addLabel();

                var parameterObject = createTransformParameter(document,
                    locked ? null : parent, // If locked, still create the object, but don't display it
                    parameterData, onParameterChanged);

                this.parameters.push(parameterObject);
            }
        }

        if(labelText.length > 0)
            addLabel();
    }
}
