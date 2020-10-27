/*
 * Copyright © 2013-2020 Graphia Technologies Ltd.
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

function addLabelTo(text, parent)
{
    text = text.trim();
    Qt.createQmlObject("import QtQuick 2.7\n" +
        "import QtQuick.Controls 1.5\n" +
        "Label { text: \"" +
        Utils.normaliseWhitespace(text) +
        "\"; color: root.textColor }", parent);
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

function Create(transformIndex, transform)
{
    var parameterIndex = 1;

    this.action = transform.action;
    this.flags = transform.flags;

    // Escape '%' to '%!' so that when the template is actually employed, a literal
    // % isn't potentially misinterpreted as a template substitution parameter
    this.template = "\"" + transform.action.replace(/%/g, "%!") + "\"";

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
            if(text[0] === '$')
            {
                text = text.substring(1);
                return "$\"" + text + "\"";
            }

            return "%" + parameterIndex++;
        }

        var template = "";

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

        for(var i = 0; i < transform.attributes.length; i++)
        {
            var attributeName = transform.attributes[i];
            var attribute = document.attribute(attributeName);
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

        var firstParam = true;

        for(var index in transform.parameters)
        {
            // Put whitespace between the parameters
            if(!firstParam)
                appendToElements(this._elements, " ");
            firstParam = false;

            var parameter = transform.parameters[index];
            this.template += " \"" + parameter.name + "\" = %" + parameterIndex++;
            appendConditionToElements(this._elements, {lhs: "$" + parameter.name, op:"=", rhs: parameter.value});
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
            addLabelTo(labelText, parent);
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
                        if(operand[0] === '$')
                            labelText += AttributeUtils.prettify(operand);
                        else
                        {
                            var parameterData = {};

                            if(opposite !== undefined && opposite.length > 0 && opposite[0] === '$')
                            {
                                var parameterName = opposite.substring(1);
                                parameterData = document.findTransformParameter(that.action, parameterName);
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

                            var parameterObject = createTransformParameter(document,
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

                    var parameterData = {};
                    parameterData.valueType = Graphia.ValueType.Attribute;

                    // Don't allow the user to choose attributes that don't exist at the point
                    // in time when the transform is executed
                    var unavailableAttributeNames =
                        document.createdAttributeNamesAtTransformIndexOrLater(transformIndex);

                    parameterData.initialValue = document.availableAttributesModel(
                        parameter.elementType, parameter.valueType,
                        Graphia.AttributeFlag.DisableDuringTransfom,
                        unavailableAttributeNames);

                    parameterData.initialAttributeName = attributeName;

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
