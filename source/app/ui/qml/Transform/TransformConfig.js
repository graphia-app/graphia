/*
 * Copyright © 2013-2022 Graphia Technologies Ltd.
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

.import "../../../../shared/ui/qml/Utils.js" as Utils
.import "../AttributeUtils.js" as AttributeUtils
.import app.graphia 1.0 as Graphia

function createTransformParameter(document, parent, parameterData, onParameterChanged)
{
    let component = Qt.createComponent("TransformParameter.qml");
    if(component === null)
    {
        console.log("Failed to create parameterComponent");
        return null;
    }

    let object = component.createObject(parent);
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

function addLabelTo(text, parent)
{
    text = text.trim();
    Qt.createQmlObject("import QtQuick 2.7\n" +
        "import QtQuick.Controls 2.12\n" +
        "Label { text: \"" +
        Utils.normaliseWhitespace(text) +
        "\"; color: root.textColor }", parent);
}

function sanitiseOp(text)
{
    let replacements =
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

    for(let i = 0; i < replacements.length; i += 2)
    {
        let regexLiteral = replacements[i].replace(/[-\/\\^$*+?.()|[\]{}]/g, "\\$&");
        text = text.replace(new RegExp("^" + regexLiteral + "$"), replacements[i + 1]);
    }

    return text;
}

function Create(transformIndex, transform)
{
    let parameterIndex = 1;

    this.action = transform.action;
    this.flags = transform.flags;

    // Escape '%' to '%!' so that when the template is actually employed, a literal
    // % isn't potentially misinterpreted as a template substitution parameter
    this.template = "\"" + transform.action.replace(/%/g, "%!") + "\"";

    this._elements = [];

    function appendToElements(elements, value)
    {
        let last = elements.length - 1;
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
            if(text[0] === '$')
            {
                text = text.substring(1);
                return "$\"" + text + "\"";
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

    // Action
    appendToElements(this._elements, transform.action);

    // Attribute Parameters
    if(transform.attributes.length > 0)
    {
        this.template += " using";
        appendToElements(this._elements, " using ");

        for(let i = 0; i < transform.attributes.length; i++)
        {
            let attributeName = transform.attributes[i];
            let attribute = document.attribute(attributeName);
            this.template += " $%" + parameterIndex++;

            appendToElements(this._elements,
                {
                    attributeName: attributeName,
                    elementType: attribute.elementType,
                    valueType: attribute.valueType
                });
        }
    }

    // Parameters
    if(transform.parameters.length > 0)
    {
        this.template += " with";
        appendToElements(this._elements, " with ");

        let firstParam = true;

        for(let index in transform.parameters)
        {
            // Put whitespace between the parameters
            if(!firstParam)
                appendToElements(this._elements, " ");
            firstParam = false;

            let parameter = transform.parameters[index];
            this.template += " \"" + parameter.name + "\" = %" + parameterIndex++;
            appendConditionToElements(this._elements,
                {type: "parameter", lhs: "$" + parameter.name, op: "=", rhs: parameter.value});
        }
    }

    // Condition
    if(transform.condition !== undefined)
    {
        this.template += " where " + conditionToTemplate(transform.condition);

        let condition = {type: "condition"};
        condition = Object.assign(condition, transform.condition);

        appendToElements(this._elements, " where ");
        appendConditionToElements(this._elements, condition);
    }

    this.toComponents =
    function(document, parent, locked, onParameterChanged)
    {
        let labelText = "";

        function addLabel()
        {
            addLabelTo(labelText, parent);
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
                            labelText += AttributeUtils.prettify(operand);
                        else
                        {
                            let parameterData = {};

                            if(opposite !== undefined && opposite.length > 0 && opposite[0] === '$')
                            {
                                let parameterName = opposite.substring(1);

                                if(parameter.type === "parameter")
                                    parameterData = document.transformParameter(that.action, parameterName);
                                else if(parameter.type === "condition")
                                    parameterData = document.attribute(parameterName);
                                else
                                    console.log("Transform element parameter type unknown: " + parameter.type);
                            }
                            else
                            {
                                // We known nothing of the type, so just treat it as a string
                                parameterData.valueType = Graphia.ValueType.String;

                                parameterData.hasRange = false;
                                parameterData.hasMinimumValue = false;
                                parameterData.hasMaximumValue = false;
                            }

                            if(parameterData.valueType === Graphia.ValueType.StringList)
                                parameterData.initialIndex = parameterData.initialValue.indexOf(operand);
                            else
                                parameterData.initialValue = operand;

                            if(locked)
                            {
                                if(parameterData.valueType === Graphia.ValueType.String)
                                    labelText += "\\\"" + Utils.addSlashes(operand) + "\\\"";
                                else
                                    labelText += Graphia.QmlUtils.formatNumberScientific(operand);
                            }
                            else
                                addLabel();

                            let parameterObject = createTransformParameter(document,
                                locked ? null : parent, // If locked, still create the object, but don't display it
                                parameterData, onParameterChanged);

                            that.parameters.push(parameterObject);
                        }
                    }

                    addOperand(parameter.lhs, parameter.rhs);
                    labelText += " " + sanitiseOp(parameter.op);

                    if(parameter.rhs !== undefined)
                    {
                        labelText += " ";
                        addOperand(parameter.rhs, parameter.lhs);
                    }
                }
                else if(parameter.attributeName !== undefined)
                {
                    // The element is an attribute

                    if(locked)
                        labelText += AttributeUtils.prettify(parameter.attributeName);

                    addLabel();

                    let parameterData = {};
                    parameterData.valueType = Graphia.ValueType.Attribute;

                    // Don't allow the user to choose attributes that don't exist at the point
                    // in time when the transform is executed
                    let unavailableAttributeNames =
                        document.createdAttributeNamesAtTransformIndexOrLater(transformIndex);

                    parameterData.initialValue = document.availableAttributesModel(
                        parameter.elementType, parameter.valueType,
                        Graphia.AttributeFlag.DisableDuringTransform,
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
