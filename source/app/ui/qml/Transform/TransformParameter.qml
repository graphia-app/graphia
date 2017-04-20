import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import ".."
import "../Utils.js" as Utils

GridLayout
{
    id: root

    property int valueType
    property bool hasRange
    property bool hasMinimumValue
    property bool hasMaximumValue
    property real minimumValue
    property real maximumValue
    property string initialValue
    property string value

    property bool updateValueImmediately: false
    property int direction: Qt.Horizontal

    flow: (direction === Qt.Horizontal) ? GridLayout.LeftToRight : GridLayout.TopToBottom

    property int _maxWidth: 160
    property int _inputElementWidth: (direction === Qt.Horizontal) ? 90 : _maxWidth

    function typedValue(n)
    {
        if(!Utils.isNumeric(n))
            console.log("typedValue called with non-numeric: " + n);

        if(valueType === ValueType.Int)
            return Math.round(n);
        else if(valueType === ValueType.Float && Utils.isInt(n))
            return Number(n).toFixed(1);

        return n;
    }

    SpinBox
    {
        id: spinBox
        Layout.preferredWidth: _inputElementWidth
        visible: (valueType === ValueType.Int || valueType === ValueType.Float)

        decimals:
        {
            if(valueType === ValueType.Float)
                return Utils.decimalPointsForRange(minimumValue, maximumValue);

            return 0;
        }

        stepSize: hasRange ? (maximumValue - minimumValue) / 100.0 : 1.0

        function updateValue()
        {
            var v = typedValue(value);

            if(slider.visible)
                slider.value = v;

            root.value = v;
        }

        onValueChanged:
        {
            if(updateValueImmediately && !slider.pressed)
                updateValue();
        }

        onEditingFinished: { updateValue(); }
    }

    Slider
    {
        id: slider
        Layout.preferredWidth: _maxWidth
        visible: ((valueType === ValueType.Int || valueType === ValueType.Float) && hasRange)

        onValueChanged:
        {
            if(pressed)
                spinBox.value = typedValue(value);
        }

        onPressedChanged:
        {
            if(!pressed)
                root.value = typedValue(value);
        }
    }

    TextField
    {
        id: textField
        Layout.preferredWidth: _inputElementWidth
        visible: (valueType === ValueType.String || valueType === ValueType.Unknown)
        enabled: valueType !== ValueType.Unknown

        function updateValue()
        {
            root.value = "\"" + text + "\"";
        }

        onTextChanged:
        {
            if(updateValueImmediately)
                updateValue();
        }

        onEditingFinished: { updateValue(); }
    }

    function configure(data)
    {
        for(var property in data)
        {
            if(this[property] !== undefined)
                this[property] = data[property];
        }

        if(valueType === ValueType.Unknown)
            value = "";
        else if(valueType !== ValueType.String)
        {
            if(initialValue.length === 0)
            {
                // Make up a plausible value if an initialValue isn't given
                if(hasRange)
                    value = (minimumValue + maximumValue) * 0.5;
                else if(hasMinimumValue)
                    value = minimumValue;
                else if(hasMaximumValue)
                    value = maximumValue;
                else
                    value = 0;
            }
            else
                value = initialValue;

            value = typedValue(value);
        }
        else
            value = "\"" + initialValue + "\"";

        var floatValue = parseFloat(value);

        if(!isNaN(floatValue))
        {
            if(hasMinimumValue)
            {
                if(floatValue < minimumValue)
                    floatValue = minimumValue;

                spinBox.minimumValue = slider.minimumValue = minimumValue;
            }
            else
                spinBox.minimumValue = slider.minimumValue = Number.MIN_VALUE;

            if(hasMaximumValue)
            {
                if(floatValue > maximumValue)
                    floatValue = maximumValue;

                spinBox.maximumValue = slider.maximumValue = maximumValue;
            }
            else
                spinBox.maximumValue = slider.maximumValue = Number.MAX_VALUE;

            spinBox.value = floatValue;
            slider.value = floatValue;
        }

        textField.text = initialValue;
    }

    function reset()
    {
        configure({valueType: ValueType.Unknown,
                   hasRange: false,
                   hasMinimumValue: false,
                   hasMaximumValue: false,
                   initialValue: ""});
    }

    Component.onCompleted: { reset(); }
}
