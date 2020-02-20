/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import ".."
import "../../../../shared/ui/qml/Utils.js" as Utils

GridLayout
{
    id: root

    property int valueType
    property bool hasRange
    property bool hasMinimumValue
    property bool hasMaximumValue
    property real minimumValue
    property real maximumValue

    // In configure(...) we test properties for undefined, which is the default
    // for variant, so give initialValue some other (arbitrary) value
    property variant initialValue: ({})

    property string value
    property int initialIndex

    property bool updateValueImmediately: false
    property int direction: Qt.Horizontal

    flow: (direction === Qt.Horizontal) ? GridLayout.LeftToRight : GridLayout.TopToBottom

    property bool fillWidth: false
    property int _preferredWidth:
        (root.direction === Qt.Horizontal &&
         (valueType & (ValueType.StringList|ValueType.String)) === 0) ? // Always make the string types wide
            90 : 160
    implicitWidth: !fillWidth ? _preferredWidth : 0.0

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
        Layout.fillWidth: root.fillWidth
        Layout.preferredWidth: root._preferredWidth
        visible: (valueType === ValueType.Int || valueType === ValueType.Float)

        decimals:
        {
            if(valueType === ValueType.Float)
            {
                if(hasRange)
                    return Utils.decimalPointsForRange(root.minimumValue, root.maximumValue);

                return 3;
            }

            return 0;
        }

        stepSize:
        {
            if(hasRange)
            {
                var incrementSize = Utils.incrementForRange(root.minimumValue, root.maximumValue);

                if(valueType === ValueType.Int && incrementSize < 1.0)
                    return 1.0;

                return incrementSize;
            }

            return 1.0;
        }

        function updateValue()
        {
            var v = typedValue(value);

            if(slider.visible)
                slider.value = v;

            root.value = v;
        }

        property bool ignoreEdits: false

        onValueChanged:
        {
            if(updateValueImmediately && !slider.pressed)
                updateValue();

            ignoreEdits = false;
        }

        onEditingFinished:
        {
            if(!ignoreEdits)
            {
                updateValue();
                ignoreEdits = false;
            }
        }
    }

    Slider
    {
        id: slider
        Layout.fillWidth: root.fillWidth
        Layout.preferredWidth: root._preferredWidth
        visible: ((valueType === ValueType.Int || valueType === ValueType.Float) && hasRange)

        stepSize: Utils.incrementForRange(root.minimumValue, root.maximumValue);

        onValueChanged:
        {
            if(pressed)
            {
                spinBox.value = typedValue(value);
                spinBox.ignoreEdits = true;
            }
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
        Layout.fillWidth: root.fillWidth
        Layout.preferredWidth: root._preferredWidth
        visible: (valueType === ValueType.String || valueType === ValueType.Unknown)
        enabled: valueType !== ValueType.Unknown

        function updateValue()
        {
            root.value = "\"" + Utils.escapeQuotes(text) + "\"";
        }

        onTextChanged:
        {
            if(updateValueImmediately)
                updateValue();
        }

        onFocusChanged:
        {
            if(focus)
                selectAll();
        }

        onEditingFinished: { updateValue(); }
    }

    ComboBox
    {
        id: comboBox
        Layout.fillWidth: root.fillWidth
        Layout.preferredWidth: root._preferredWidth
        visible: valueType === ValueType.StringList
        enabled: valueType !== ValueType.Unknown

        function updateValue()
        {
            root.value = "\"" + currentText + "\"";
        }

        onCurrentTextChanged: { updateValue(); }
    }

    function updateValue()
    {
        if(textField.visible)
            textField.updateValue();
        else if(spinBox.visible)
            spinBox.updateValue();
    }

    function configure(data)
    {
        for(var property in data)
        {
            if(this[property] !== undefined)
                this[property] = data[property];
        }

        switch(valueType)
        {
        case ValueType.Unknown:
            value = "";
            break;

        case ValueType.Float:
        case ValueType.Int:
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
                    spinBox.minimumValue = slider.minimumValue = Number.NEGATIVE_INFINITY;

                if(hasMaximumValue)
                {
                    if(floatValue > maximumValue)
                        floatValue = maximumValue;

                    spinBox.maximumValue = slider.maximumValue = maximumValue;
                }
                else
                    spinBox.maximumValue = slider.maximumValue = Number.POSITIVE_INFINITY;

                spinBox.value = floatValue;
                slider.value = floatValue;
            }
            break;

        case ValueType.String:
            textField.text = initialValue;

            value = "\"" + Utils.escapeQuotes(initialValue) + "\"";
            break;

        case ValueType.StringList:
            comboBox.model = initialValue;
            comboBox.currentIndex = initialIndex;

            value = "\"" + Utils.escapeQuotes(comboBox.currentText) + "\"";
            break;
        }
    }

    function reset()
    {
        configure({valueType: ValueType.Unknown,
                   hasRange: false,
                   hasMinimumValue: false,
                   hasMaximumValue: false,
                   initialValue: "",
                   initialIndex: -1});
    }

    Component.onCompleted: { reset(); }
}
