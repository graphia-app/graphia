.pragma library

/*
 * Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

.import Graphia.SharedTypes as GraphiaSharedTypes
.import Graphia.Utils as GraphiaUtils

function createTransformParameter(document, parent, parameterData, onParameterChanged)
{
    let component = Qt.createComponent("TransformParameter.qml");
    if(component === null)
    {
        console.log("Failed to create parameterComponent");
        return null;
    }

    let object = component.createObject(parent, {"document": document});
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

function addLabelTo(text, parent, transformItem)
{
    text = text.trim();

    let label = Qt.createQmlObject("import QtQuick\n" +
        "import QtQuick.Controls\n" +
        "Label {}", parent);

    label.text = GraphiaUtils.Utils.normaliseWhitespace(text);
    label.color = Qt.binding(function() { return transformItem.textColor; });
}

function appendToElements(elements, value)
{
    let last = elements.length - 1;
    if(last >= 0 && typeof value === "string" && typeof elements[last] === typeof value)
        elements[last] += value;
    else
        elements.push(value);
}

function substituted(phrase, args)
{
    let elements = [];

    for(const part of phrase.split(/(%\d+)/))
    {
        let match = part.match(/^%(\d+)$/);

        if(match !== null)
            args[match[1] - 1].forEach(element => appendToElements(elements, element));
        else if(part.length > 0)
            appendToElements(elements, part);
    }

    return elements;
}

function Create(transformIndex, document, transformString)
{
    let parameterIndex = 1;
    let transform = document.parseGraphTransform(transformString);

    this.action = transform.action;
    this.flags = transform.flags;

    // Escape '%' to '%!' so that when the template is actually employed, a literal
    // % isn't potentially misinterpreted as a template substitution parameter
    this.template = document.writtenString(transform.action).replace(/%/g, "%!");

    let displayText = document.transformDisplayText();

    function conditionElements(condition, type)
    {
        if(typeof condition.rhs !== "object")
            return [Object.assign({type: type}, condition)];

        let bracketed = operand => substituted("(%1)", [conditionElements(operand, type)]);

        return substituted(document.opDisplayText(condition.op).phrase,
            [bracketed(condition.lhs), bracketed(condition.rhs)]);
    }

    function conditionToTemplate(condition)
    {
        function templated(text, isName)
        {
            // As with the action, escape '%' in attribute names
            if(isName)
                return "$" + document.writtenAttributeName(text).replace(/%/g, "%!");

            return "%" + parameterIndex++;
        }

        let template = "";

        if(typeof condition.lhs === "object")
            template += "(" + conditionToTemplate(condition.lhs) + ")";
        else
            template += templated(condition.lhs, condition.lhsIsName);

        template += " " + condition.op;

        if(condition.rhs !== undefined)
        {
            template += " ";

            if(typeof condition.rhs === "object")
                template += "(" + conditionToTemplate(condition.rhs) + ")";
            else
                template += templated(condition.rhs, condition.rhsIsName);
        }

        return template;
    }

    // Each part wraps those before it, so a translation can put it wherever its language needs

    // Action
    this._elements = [transform.displayName];

    // Attribute Parameters
    if(transform.attributes.length > 0)
    {
        this.template += " using";

        let attributeElements = [];

        for(let i = 0; i < transform.attributes.length; i++)
        {
            let attributeName = transform.attributes[i];
            let attribute = document.attribute(attributeName);
            this.template += " $%" + parameterIndex++;

            attributeElements.push(
                {
                    attributeName: attributeName,
                    elementType: attribute.elementType,
                    valueType: attribute.valueType
                });
        }

        this._elements = substituted(displayText.usingAttributes, [this._elements, attributeElements]);
    }

    // Parameters
    if(transform.parameters.length > 0)
    {
        this.template += " with";

        let parameterElements = [];

        for(let index in transform.parameters)
        {
            // Put whitespace between the parameters
            if(parameterElements.length > 0)
                appendToElements(parameterElements, " ");

            let parameter = transform.parameters[index];
            this.template += " " + document.writtenString(parameter.name) + " = %" + parameterIndex++;
            parameterElements.push({type: "parameter", lhs: parameter.name, lhsIsName: true, rhs: parameter.value});
        }

        this._elements = substituted(displayText.withParameters, [this._elements, parameterElements]);
    }

    // Condition
    if(transform.condition !== undefined)
    {
        this.template += " where " + conditionToTemplate(transform.condition);

        this._elements = substituted(displayText.whereCondition,
            [this._elements, conditionElements(transform.condition, "condition")]);
    }

    this.toComponents =
    function(document, parent, transformItem, locked, onParameterChanged)
    {
        let labelText = "";
        let parameters = [];
        this.parameters = parameters;

        function addLabel()
        {
            addLabelTo(labelText, parent, transformItem);
            labelText = "";
        }

        function addParameter(parameterData)
        {
            let parameterObject = createTransformParameter(document,
                locked ? null : parent, // If locked, still create the object, but don't display it
                parameterData, onParameterChanged);

            parameters.push(parameterObject);
        }

        function nameText(element, name)
        {
            if(element.type !== "parameter")
                return document.attributeDisplayName(name);

            let displayName = document.transformParameter(transform.action, name).displayName;
            return displayName !== undefined ? displayName : name;
        }

        // A value's type comes from the name opposite it, if there is one
        function valueParameterData(element, oppositeName)
        {
            if(oppositeName === undefined)
            {
                // We know nothing of the type, so just treat it as a string
                return {valueType: GraphiaSharedTypes.ValueType.String,
                    hasRange: false, hasMinimumValue: false, hasMaximumValue: false};
            }

            if(element.type === "parameter")
                return document.transformParameter(transform.action, oppositeName);

            if(element.type !== "condition")
            {
                console.log("Transform element parameter type unknown: " + element.type);
                return {};
            }

            let parameterData = document.attribute(oppositeName);
            parameterData.displayValueOf = value => document.attributeDisplayValue(oppositeName, value);
            parameterData.storedValueOf = text => document.attributeStoredValue(oppositeName, text);
            return parameterData;
        }

        function lockedValueText(parameterData, value)
        {
            switch(parameterData.valueType)
            {
            default:
            case GraphiaSharedTypes.ValueType.String:
            {
                let displayValueOf = parameterData.displayValueOf;
                return "\"" + (displayValueOf !== undefined ? displayValueOf(value) : value) + "\"";
            }
            case GraphiaSharedTypes.ValueType.StringList:
            {
                // A choice from a list, shown by its display text
                let index = parameterData.initialIndex;
                let displayValues = parameterData.displayValues;
                return displayValues !== undefined &&
                    index >= 0 && index < displayValues.length ? displayValues[index] : value;
            }
            case GraphiaSharedTypes.ValueType.Int:
            case GraphiaSharedTypes.ValueType.Float:
                return GraphiaUtils.NativeUtils.formatNumberScientific(value);
            }
        }

        // An operand is either a name of a parameter or an attribute, or a value
        function addOperand(element, operand, isName, opposite, oppositeIsName)
        {
            if(isName)
            {
                labelText += nameText(element, operand);
                return;
            }

            let parameterData = valueParameterData(element, oppositeIsName ? opposite : undefined);

            if(parameterData.valueType === GraphiaSharedTypes.ValueType.StringList)
                parameterData.initialIndex = parameterData.initialValue.indexOf(operand);
            else
                parameterData.initialValue = operand;

            if(locked)
                labelText += lockedValueText(parameterData, operand);
            else
                addLabel();

            addParameter(parameterData);
        }

        // The element is a parameter or a condition
        function addOperation(element)
        {
            let phrase = element.type === "parameter" ?
                displayText.parameter : document.opDisplayText(element.op).phrase;

            for(const part of phrase.split(/(%\d+)/))
            {
                if(part === "%1")
                    addOperand(element, element.lhs, element.lhsIsName, element.rhs, element.rhsIsName);
                else if(part === "%2" && element.rhs !== undefined)
                    addOperand(element, element.rhs, element.rhsIsName, element.lhs, element.lhsIsName);
                else if(!/^%\d+$/.test(part))
                    labelText += part;
            }
        }

        function addAttribute(element)
        {
            if(locked)
                labelText += document.attributeDisplayName(element.attributeName);

            addLabel();

            // Don't allow the user to choose attributes that don't exist at the point
            // in time when the transform is executed
            let unavailableAttributeNames =
                document.addedOrChangedAttributeNamesAtTransformIndexOrLater(transformIndex);

            addParameter(
            {
                valueType: GraphiaSharedTypes.ValueType.Attribute,
                initialValue: document.availableAttributesModel(
                    element.elementType, element.valueType,
                    GraphiaSharedTypes.AttributeFlag.DisableDuringTransform,
                    unavailableAttributeNames),
                initialAttributeName: element.attributeName
            });
        }

        for(const element of this._elements)
        {
            if(typeof element === "string")
                labelText += element;
            else if(element.lhs !== undefined)
                addOperation(element);
            else if(element.attributeName !== undefined)
                addAttribute(element);
        }

        if(labelText.length > 0)
            addLabel();
    }
}
