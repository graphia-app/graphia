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

function sanitiseAttribute(text)
{
    // Remove the leading $, if present
    text = text.replace(/^\$/, "");

    // Replace edge node syntax with human readable text
    text = text.replace(/^source\./, "Source ");
    text = text.replace(/^target\./, "Target ");

    return text;
}

function sanitiseOp(text)
{
    var replacements =
    [
        "==",                       "=",
        "!=",                       "≠",
        "<=",                       "≤",
        ">=",                       "≥",
        "&&",                       "and",
        "||",                       "or",
        "includes",                 "Includes",
        "excludes",                 "Excludes",
        "starts",                   "Starts With",
        "ends",                     "Ends With",
        "matches",                  "Matches Regex",
        "matchesCaseInsensitive",   "Matches Case Insensitive Regex",
        "hasValue",                 "Has Value",
    ];

    for(var i = 0; i < replacements.length; i += 2)
    {
        var regexLiteral = replacements[i].replace(/[-\/\\^$*+?.()|[\]{}]/g, "\\$&");
        text = text.replace(new RegExp("^" + regexLiteral + "$"), replacements[i + 1]);
    }

    return text;
}

function Create(transform)
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
        function templated(text)
        {
            if(!text || text.length === 0)
                return "";

            if(text[0] === '$')
            {
                text = text.substring(1);
                return "$\"" + text + "\"";
            }

            return "%";
        }

        var lhs = "";
        var rhs = "";

        if(typeof condition.lhs === "object")
            lhs = "(" + conditionToTemplate(condition.lhs) + ")";
        else
            lhs = templated(condition.lhs);

        if(typeof condition.rhs === "object")
            rhs = "(" + conditionToTemplate(condition.rhs) + ")";
        else
            rhs = templated(condition.rhs);

        return lhs + " " + condition.op + " " + rhs;
    }

    // Action
    appendToElements(this._elements, transform.action);

    // Attribute Parameters
    if(transform.attributes.length > 0)
    {
        this.template += " using";
        appendToElements(this._elements, " using ");

        for(var i = 0; i < transform.attributes.length; i++)
        {
            var attribute = transform.attributes[i];
            this.template += " $%";
            appendToElements(this._elements, {attribute: attribute});
        }
    }

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
            appendConditionToElements(this._elements, {lhs: "$" + parameterName, op:"=", rhs: parameterValue});
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

                var that = this;

                if(parameter.lhs !== undefined)
                {
                    // The element is a parameter or a condition

                    function addOperand(operand, opposite)
                    {
                        if(!operand || operand.length === 0)
                            return;

                        if(operand[0] === '$')
                            labelText += sanitiseAttribute(operand);
                        else
                        {
                            var parameterData = {};

                            if(opposite.length > 0 && opposite[0] === '$')
                            {
                                var parameterName = opposite.substring(1);
                                parameterData = document.findTransformParameter(that.action, parameterName);
                            }
                            else
                            {
                                // We known nothing of the type, so just treat it as a string
                                parameterData.valueType = ValueType.String;

                                parameterData.hasRange = false;
                                parameterData.hasMinimumValue = false;
                                parameterData.hasMaximumValue = false;
                            }

                            parameterData.initialValue = operand;

                            if(locked)
                            {
                                if(parameterData.valueType === ValueType.String)
                                    labelText += " \\\"" + operand + "\\\"";
                                else
                                    labelText += " " + Utils.formatForDisplay(operand);
                            }
                            else
                                addLabel();

                            var parameterObject = createTransformParameter(document,
                                locked ? null : parent, // If locked, still create the object, but don't display it
                                parameterData, onParameterChanged);

                            that.parameters.push(parameterObject);
                        }
                    }

                    addOperand(parameter.lhs, parameter.rhs);
                    labelText += " " + sanitiseOp(parameter.op) + " ";
                    addOperand(parameter.rhs, parameter.lhs);
                }
                else if(parameter.attribute !== undefined)
                {
                    // The element is an attribute

                    if(locked)
                        labelText += sanitiseAttribute(parameter.attribute);

                    addLabel();

                    var parameterData = {};
                    parameterData.valueType = ValueType.StringList;
                    parameterData.initialValue = document.attribute(attribute).similar;
                    parameterData.initialIndex = parameterData.initialValue.indexOf(attribute);

                    var parameterObject = createTransformParameter(document,
                        locked ? null : parent, // If locked, still create the object, but don't display it
                        parameterData, onParameterChanged);

                    that.parameters.push(parameterObject);
                }
            }
        }

        if(labelText.length > 0)
            addLabel();
    }
}
