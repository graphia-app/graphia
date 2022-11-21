/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

import QtQuick
import QtQuick.Controls

Control
{
    id: root

    property double value: 0.0
    property double from: 0.0
    property double to: 100.0
    property double stepSize: 1.0

    property alias editable: spinBox.editable
    property alias wrap: spinBox.wrap

    property int decimals: 2

    readonly property alias displayText: spinBox.displayText
    readonly property alias inputMethodComposing: spinBox.inputMethodComposing

    function decrease() { spinBox.decrease(); }
    function increase() { spinBox.increase(); }

    function _update()
    {
        spinBox._setting = true;
        spinBox.value = spinBox.intValue(root.value);
        spinBox._setting = false;
    }

    onValueChanged:
    {
        if(spinBox._setting)
            return;

        root._update();
    }

    // See comment below on forceTextFromValueUpdateHack()
    onFromChanged: { root._update(); spinBox.forceTextFromValueUpdateHack(); }
    onToChanged:   { root._update(); spinBox.forceTextFromValueUpdateHack(); }

    contentItem: SpinBox
    {
        id: spinBox

        //FIXME: this is set to avoid the contentItem control escaping the control
        // bounds, but in that case it overlaps the control decoration. OTOH if
        // clip is set on contentItem itself then it clips excessively; needs
        // another look after upgrading to Qt6 widget styles
        clip: true

        width: root.availableWidth
        height: root.availableHeight

        inputMethodHints: Qt.ImhFormattedNumbersOnly

        // Avoid ping-ponging between root.onValueChanged and spinBox.onValueChanged
        property bool _setting: false

        function doubleValue(i) { return root.from + ((root.to - root.from) / (to - from)) * (i - from); }
        function intValue(d)    { return from + ((to - from) / (root.to - root.from)) * (d - root.from); }

        from: -0x7FFFFFFF
        to: 0x7FFFFFFF
        stepSize: root.stepSize * ((spinBox.to - spinBox.from) / (root.to - root.from))

        Component.onCompleted:
        {
            value = intValue(root.value);
            contentItem.selectByMouse = true;
        }

        onActiveFocusChanged:
        {
            if(activeFocus)
                contentItem.selectAll();
        }

        property QtObject _doubleValidator: DoubleValidator
        {
            locale: spinBox.locale.name
            bottom: Math.min(root.from, root.to)
            top: Math.max(root.from, root.to)
        }

        property QtObject _intValidator: IntValidator
        {
            locale: spinBox.locale.name
            bottom: Math.round(Math.min(root.from, root.to))
            top: Math.round(Math.max(root.from, root.to))
        }

        validator: root.decimals > 0 ? _doubleValidator : _intValidator

        textFromValue: function(value, locale)
        {
            return Number(spinBox.doubleValue(value)).toLocaleString(locale, 'f', root.decimals);
        }

        function forceTextFromValueUpdateHack()
        {
            // The function bound to textFromValue is not called when the
            // variables the function references are changed; by momentarily
            // changing the control's locale, a call to textFromValue is forced
            // It's possible that this is a Qt bug
            let actualLocale = locale;
            locale = Qt.locale(locale.name + ".dummy");
            locale = actualLocale;
        }

        valueFromText: function(text, locale)
        {
            let d = Number.fromLocaleString(locale, text);
            d = Math.max(Math.min(d, root.to), root.from); // Clamp
            return spinBox.intValue(d);
        }

        onValueChanged:
        {
            if(spinBox._setting)
                return;

            spinBox._setting = true;
            root.value = doubleValue(value);
            spinBox._setting = false;
        }

        onValueModified: { root.valueModified(); }
    }

    signal valueModified()
}
