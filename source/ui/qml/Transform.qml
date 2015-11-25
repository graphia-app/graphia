import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import com.kajeka 1.0

Item
{
    id: root
    width: rows.width
    height: rows.height

    RowLayout
    {
        id: rows

        ExclusiveGroup { id: buttonMenuGroup }

        ButtonMenu
        {
            id: transformMenu

            defaultText: qsTr("Add Transform")

            model: availableTransformNames
            visible: creationState === GraphTransformCreationState.Uncreated ||
                     creationState === GraphTransformCreationState.TransformSelected
            enabled: transformEnabled
            exclusiveGroup: buttonMenuGroup
            onSelectedValueChanged:
            {
                valueTextField.unfocusAndReset();
                name = selectedValue;
            }
        }

        Text
        {
            text:
            {
                switch(creationState)
                {
                case GraphTransformCreationState.TransformSelected:
                    return qsTr("where");
                case GraphTransformCreationState.FieldSelected:
                    return qsTr("%1 where").arg(name);
                case GraphTransformCreationState.OperationSelected:
                case GraphTransformCreationState.Created:
                    return qsTr("%1 where %2").arg(name).arg(fieldName);
                case GraphTransformCreationState.Fixed:
                    return qsTr("%1 where %2 %3 %4").arg(name).arg(fieldName).arg(op).arg(fieldValue);
                default:
                    return "";
                }
            }

            visible: creationState > GraphTransformCreationState.Uncreated
            enabled: transformEnabled
        }

        ButtonMenu
        {
            id: fieldMenu

            defaultText: qsTr("Select Field")

            model: availableDataFields
            visible: creationState === GraphTransformCreationState.TransformSelected ||
                     creationState === GraphTransformCreationState.FieldSelected
            enabled: transformEnabled
            exclusiveGroup: buttonMenuGroup
            onSelectedValueChanged:
            {
                valueTextField.unfocusAndReset();
                fieldName = selectedValue;
            }
        }

        ButtonMenu
        {
            id: opMenu

            defaultText: qsTr("Select Operation")

            model: avaliableConditionFnOps
            visible: creationState === GraphTransformCreationState.FieldSelected ||
                     creationState === GraphTransformCreationState.OperationSelected ||
                     creationState === GraphTransformCreationState.Created
            enabled: transformEnabled
            exclusiveGroup: buttonMenuGroup
            onSelectedValueChanged:
            {
                valueTextField.unfocusAndReset();
                op = selectedValue;
            }
        }

        Slider
        {
            id: valueSlider
            Layout.preferredWidth: 75
            visible: type !== GraphTransformType.String &&
                     hasFieldRange &&
                     (creationState === GraphTransformCreationState.OperationSelected ||
                     creationState === GraphTransformCreationState.Created)
            enabled: transformEnabled
            minimumValue: minFieldValue
            maximumValue: maxFieldValue
            stepSize: type === GraphTransformType.Int ? 1 : 0

            onValueChanged:
            {
                if(pressed)
                    valueTextField.setTextAndFormat(value);
            }

            onPressedChanged:
            {
                if(pressed)
                {
                    valueTextField.unfocusAndReset();
                    valueTextField.setTextAndFormat(value);
                }
                else
                    fieldValue = value;
            }
        }

        TextField
        {
            id: valueTextField

            Layout.preferredWidth: 50
            visible: creationState === GraphTransformCreationState.OperationSelected ||
                     creationState === GraphTransformCreationState.Created
            enabled: transformEnabled

            onEditingFinished:
            {
                fieldValue = text;

                if(type !== GraphTransformType.String)
                    valueSlider.value = parseFloat(text);
            }

            function setTextAndFormat(newText)
            {
                if(type === GraphTransformType.Float)
                {
                    // Format the number for human consumption; don't question it, just accept it
                    text = parseFloat(parseFloat(newText).toFixed(3)).toString();
                }
                else
                    text = newText;
            }

            function resetToLastValidValue()
            {
                text = fieldValue;
            }

            function unfocusAndReset()
            {
                // If the value field is focused and some other control changes state, the value
                // can change when onEditingFinished fires, which in turn means several parameters
                // change at the same time, so where necessary we call this to avoid that
                if(focus)
                {
                    resetToLastValidValue();
                    focus = false;
                }
            }

            onFocusChanged:
            {
                if(!focus)
                    resetToLastValidValue();
            }

            validator:
            {
                var validator;

                switch(type)
                {
                case GraphTransformType.Int:
                    validator = Qt.createQmlObject("import QtQuick 2.5; IntValidator { bottom: minFieldValue; top: maxFieldValue }",
                                valueTextField, "IntValidator");
                    break;

                case GraphTransformType.Float:
                    validator = Qt.createQmlObject("import QtQuick 2.5; DoubleValidator { bottom: minFieldValue; top: maxFieldValue }",
                                valueTextField, "DoubleValidator");

                    validator.decimals = 3;
                    break;

                default:
                    // Anything (non-empty) goes
                    validator = Qt.createQmlObject("import QtQuick 2.5; RegExpValidator { regExp: /\\S+/ }",
                                valueTextField, "RegExpValidator");
                    break;
                }

                return validator;
            }
        }

        CheckBox
        {
            id: enabledCheckBox

            visible: creationState === GraphTransformCreationState.Created

            onCheckedStateChanged:
            {
                valueTextField.unfocusAndReset();
                transformEnabled = checked;
            }
        }

        Button
        {
            id: removeButton
            text: "X"

            visible: creationState > GraphTransformCreationState.Uncreated

            onClicked: { document.removeGraphTransform(index); }
        }
    }

    Component.onCompleted:
    {
        // Set the initial values
        transformMenu.selectedValue = name;
        fieldMenu.selectedValue = fieldName;
        opMenu.selectedValue = op;

        if(type !== GraphTransformType.String)
            valueSlider.value = fieldValue;

        valueTextField.setTextAndFormat(fieldValue);
        enabledCheckBox.checked = transformEnabled;
    }
}
