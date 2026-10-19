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
        function templated(text)
        {
            if(text[0] === '$')
            {
                // As with the action, escape '%' in attribute names
                text = text.substring(1);
                return "$" + document.writtenAttributeName(text).replace(/%/g, "%!");
            }

            return "%" + parameterIndex++;
        }

        let template = "";

        if(typeof condition.lhs === "object")
            template += "(" + conditionToTemplate(condition.lhs) + ")";
        else
            template += templated(condition.lhs);

        template += " " + condition.op;

        if(condition.rhs !== undefined)
        {
            template += " ";

            if(typeof condition.rhs === "object")
                template += "(" + conditionToTemplate(condition.rhs) + ")";
            else
                template += templated(condition.rhs);
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
            parameterElements.push({type: "parameter", lhs: "$" + parameter.name, rhs: parameter.value});
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

        function addLabel()
        {
            addLabelTo(labelText, parent, transformItem);
            labelText = "";
        }

        this.parameters = [];

        for(let i = 0; i < this._elements.length; i++)
        {
            if(typeof this._elements[i] === "string")
                labelText += this._elements[i];
            else if(typeof this._elements[i] === 'object')
            {
                let parameter = this._elements[i];

                let that = this;

                if(parameter.lhs !== undefined)
                {
                    // The element is a parameter or a condition

                    function addOperand(operand, opposite)
                    {
                        if(operand[0] === '$')
                        {
                            if(parameter.type === "parameter")
                            {
                                let parameterName = operand.substring(1);
                                let displayName = document.transformParameter(that.action, parameterName).displayName;
                                labelText += displayName !== undefined ? displayName : parameterName;
                            }
                            else
                                labelText += document.attributeDisplayName(operand);
                        }
                        else
                        {
                            let parameterData = {};
                            let displayOperand = operand;

                            if(opposite !== undefined && opposite.length > 0 && opposite[0] === '$')
                            {
                                let parameterName = opposite.substring(1);

                                if(parameter.type === "parameter")
                                    parameterData = document.transformParameter(that.action, parameterName);
                                else if(parameter.type === "condition")
                                {
                                    parameterData = document.attribute(parameterName);
                                    displayOperand = document.attributeDisplayValue(parameterName, operand);
                                    parameterData.displayValueOf = value => document.attributeDisplayValue(parameterName, value);
                                    parameterData.storedValueOf = text => document.attributeStoredValue(parameterName, text);
                                }
                                else
                                    console.log("Transform element parameter type unknown: " + parameter.type);
                            }
                            else
                            {
                                // We known nothing of the type, so just treat it as a string
                                parameterData.valueType = GraphiaSharedTypes.ValueType.String;

                                parameterData.hasRange = false;
                                parameterData.hasMinimumValue = false;
                                parameterData.hasMaximumValue = false;
                            }

                            if(parameterData.valueType === GraphiaSharedTypes.ValueType.StringList)
                                parameterData.initialIndex = parameterData.initialValue.indexOf(operand);
                            else
                                parameterData.initialValue = operand;

                            if(locked)
                            {
                                switch(parameterData.valueType)
                                {
                                default:
                                case GraphiaSharedTypes.ValueType.String:
                                    labelText += "\"" + displayOperand + "\"";
                                    break;
                                case GraphiaSharedTypes.ValueType.StringList:
                                {
                                    // A choice from a list, shown by its display text
                                    let index = parameterData.initialValue.indexOf(operand);
                                    let displayValues = parameterData.displayValues;
                                    labelText += displayValues !== undefined &&
                                        index >= 0 && index < displayValues.length ? displayValues[index] : operand;
                                    break;
                                }
                                case GraphiaSharedTypes.ValueType.Int:
                                case GraphiaSharedTypes.ValueType.Float:
                                    labelText += GraphiaUtils.NativeUtils.formatNumberScientific(operand);
                                    break;
                                }
                            }
                            else
                                addLabel();

                            let parameterObject = createTransformParameter(document,
                                locked ? null : parent, // If locked, still create the object, but don't display it
                                parameterData, onParameterChanged);

                            that.parameters.push(parameterObject);
                        }
                    }

                    let phrase = parameter.type === "parameter" ?
                        displayText.parameter : document.opDisplayText(parameter.op).phrase;

                    for(const part of phrase.split(/(%\d+)/))
                    {
                        if(part === "%1")
                            addOperand(parameter.lhs, parameter.rhs);
                        else if(part === "%2" && parameter.rhs !== undefined)
                            addOperand(parameter.rhs, parameter.lhs);
                        else if(!/^%\d+$/.test(part))
                            labelText += part;
                    }
                }
                else if(parameter.attributeName !== undefined)
                {
                    // The element is an attribute

                    if(locked)
                        labelText += document.attributeDisplayName(parameter.attributeName);

                    addLabel();

                    let parameterData = {};
                    parameterData.valueType = GraphiaSharedTypes.ValueType.Attribute;

                    // Don't allow the user to choose attributes that don't exist at the point
                    // in time when the transform is executed
                    let unavailableAttributeNames =
                        document.addedOrChangedAttributeNamesAtTransformIndexOrLater(transformIndex);

                    parameterData.initialValue = document.availableAttributesModel(
                        parameter.elementType, parameter.valueType,
                        GraphiaSharedTypes.AttributeFlag.DisableDuringTransform,
                        unavailableAttributeNames);

                    parameterData.initialAttributeName = parameter.attributeName;

                    let parameterObject = createTransformParameter(document,
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
