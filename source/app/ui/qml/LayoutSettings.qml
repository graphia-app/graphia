/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

import QtQuick 2.12
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import app.graphia 1.0

import "Controls"
import "../../../shared/ui/qml/Constants.js" as Constants
import "../../../shared/ui/qml/Utils.js" as Utils

Rectangle
{
    id: root

    property var document

    readonly property bool showing: _visible
    property bool _visible: false

    readonly property int _buttonMenuOffset: 4

    Preferences
    {
        id: visuals
        section: "visuals"

        property double defaultNormalNodeSize
        property double defaultNormalEdgeSize

        property var layoutPresets: ""
    }

    property var layoutPresets:
    {
        try
        {
            return JSON.parse(visuals.layoutPresets);
        }
        catch(e) {}

        return {};
    }

    property bool isDefault: true
    property string matchingPreset: ""

    onMatchingPresetChanged:
    {
        presetChooser.selectedValue = matchingPreset;
    }

    property bool isCustom: !isDefault && matchingPreset.length === 0

    onIsCustomChanged:
    {
        newPresetTextField.reset();
    }

    function settingsAreDefault()
    {
        for(let layoutSettingName of root.document.layoutSettingNames)
        {
            let s = root.document.layoutSetting(layoutSettingName);

            if(Utils.floatCompare(s.value, s.defaultValue) !== 0)
                return false;
        }

        return Utils.floatCompare(root.document.nodeSize, visuals.defaultNormalNodeSize) === 0 &&
            Utils.floatCompare(root.document.edgeSize, visuals.defaultNormalEdgeSize) === 0;
    }

    function evaluateSettings()
    {
        root.isDefault = root.settingsAreDefault();

        let newMatchingPresetName = "";

        for(let presetName in root.layoutPresets)
        {
            let preset = root.layoutPresets[presetName];

            if(preset.layoutName !== root.document.layoutName)
                continue;

            let matching = true;

            for(let layoutSettingName in preset.layoutSettings)
            {
                let presetValue = preset.layoutSettings[layoutSettingName];
                let currentValue = root.document.layoutSetting(layoutSettingName).value;

                if(Utils.floatCompare(presetValue, currentValue) !== 0)
                {
                    matching = false;
                    break;
                }
            }

            if(!matching)
                continue;

            if(Utils.floatCompare(preset.nodeSize, root.document.nodeSize) !== 0)
                continue;

            if(Utils.floatCompare(preset.edgeSize, root.document.edgeSize) !== 0)
                continue;

            newMatchingPresetName = presetName;
            break;
        }

        root.matchingPreset = newMatchingPresetName;
    }

    function addPreset(presetName)
    {
        if(presetName.length === 0 || presetName === qsTr("Default"))
            return;

        let layoutSettings = {};
        for(let layoutSettingName of root.document.layoutSettingNames)
        {
            let s = root.document.layoutSetting(layoutSettingName);
            layoutSettings[layoutSettingName] = s.value;
        }

        let preset =
        {
            "layoutName": root.document.layoutName,
            "layoutSettings": layoutSettings,
            "nodeSize": root.document.nodeSize,
            "edgeSize": root.document.edgeSize
        };

        let presets = root.layoutPresets;

        presets[presetName] = preset;
        visuals.layoutPresets = JSON.stringify(presets);

        newPresetTextField.reset();
        root.evaluateSettings();
    }

    function removePreset(presetName)
    {
        let presets = root.layoutPresets;

        delete presets[presetName];
        visuals.layoutPresets = JSON.stringify(presets);

        root.evaluateSettings();
    }

    function renamePreset(oldName, newName)
    {
        let presets = root.layoutPresets;

        presets[newName] = presets[oldName];
        delete presets[oldName];

        visuals.layoutPresets = JSON.stringify(presets);

        newPresetTextField.reset();
        root.evaluateSettings();
    }

    Connections
    {
        target: root.document

        function onLayoutSettingChanged()   { root.evaluateSettings(); }
        function onNodeSizeChanged()        { root.evaluateSettings(); }
        function onEdgeSizeChanged()        { root.evaluateSettings(); }
    }

    width: row.width
    height: row.height

    border.color: document.contrastingColor
    border.width: 1
    radius: 4
    color: "white"

    Action
    {
        id: closeAction
        text: qsTr("Close")
        iconName: "emblem-unreadable"

        onTriggered:
        {
            _visible = false;
            hidden();
        }
    }

    Shortcut
    {
        enabled: _visible
        sequence: "Esc"
        onActivated: { closeAction.trigger(); }
    }

    SystemPalette { id: systemPalette }

    RowLayout
    {
        id: row

        // The ColumnLayout in a RowLayout is just a hack to get some padding
        ColumnLayout
        {
            Layout.topMargin: Constants.padding
            Layout.bottomMargin: Constants.padding - root.parent.parent.anchors.bottomMargin
            Layout.leftMargin: Constants.padding + Constants.margin
            Layout.rightMargin: Constants.padding

            RowLayout
            {
                Item
                {
                    // This Item is here to provide a layer of indirection so
                    // that we can get the presetChooser to effectively fillWidth,
                    // but not force the layout wider, as would be the case with
                    // very long names
                    Layout.fillWidth: true
                    implicitHeight: presetChooser.implicitHeight

                    ButtonMenu
                    {
                        id: presetChooser
                        visible: !newPresetTextField.visible
                        anchors.fill: parent
                        hoverColor: systemPalette.highlight

                        text:
                        {
                            if(root.isCustom)
                                return qsTr("Custom");

                            if(root.isDefault)
                                return qsTr("Default");

                            // In this case selectedValue will take precedence
                            return "";
                        }

                        font.italic: root.isCustom

                        textAlignment: Text.AlignLeft

                        model:
                        {
                            let presetNames = [];

                            for(let presetName in root.layoutPresets)
                            {
                                if(root.layoutPresets[presetName].layoutName !== root.document.layoutName)
                                    continue;

                                presetNames.push(presetName);
                            }

                            return [qsTr("Default"), ...presetNames];
                        }

                        onSelectedValueChanged:
                        {
                            if(selectedValue.length === 0)
                                return;

                            if(!root.layoutPresets.hasOwnProperty(selectedValue))
                            {
                                // Reset everything to default
                                for(let layoutSettingName of root.document.layoutSettingNames)
                                    root.document.resetLayoutSettingValue(layoutSettingName);

                                root.document.resetNodeSize();
                                root.document.resetEdgeSize();

                                presetChooser.selectedValue = "";
                            }
                            else
                            {
                                let preset = root.layoutPresets[selectedValue];

                                for(let layoutSettingName in preset.layoutSettings)
                                {
                                    let presetValue = preset.layoutSettings[layoutSettingName];
                                    root.document.setLayoutSettingValue(layoutSettingName, presetValue);
                                }

                                root.document.nodeSize = preset.nodeSize;
                                root.document.edgeSize = preset.edgeSize;
                            }
                        }

                        onHeld:
                        {
                            if(root.matchingPreset.length > 0)
                            {
                                // Configure TextField for renaming
                                newPresetTextField.text = root.matchingPreset;
                                newPresetTextField.selectAll();
                                newPresetTextField.visible = true;
                                newPresetTextField.forceActiveFocus();
                            }
                        }
                    }
                }

                TextField
                {
                    id: newPresetTextField
                    visible: false
                    Layout.fillWidth: true

                    onAccepted:
                    {
                        if(newPresetTextField.text.length > 0)
                        {
                            if(root.matchingPreset.length === 0)
                                root.addPreset(newPresetTextField.text);
                            else
                                root.renamePreset(root.matchingPreset, newPresetTextField.text);
                        }
                    }

                    function reset()
                    {
                        visible = false;
                        text = "";
                    }
                }

                FloatingButton
                {
                    visible: root.matchingPreset.length > 0 && !newPresetTextField.visible
                    text: qsTr("Remove Preset")
                    iconName: "list-remove"

                    onClicked:
                    {
                        root.removePreset(root.matchingPreset);
                    }
                }

                FloatingButton
                {
                    visible: root.isCustom
                    text: qsTr("Add Preset")
                    iconName: "list-add"

                    onClicked:
                    {
                        if(!newPresetTextField.visible)
                        {
                            newPresetTextField.visible = true;
                            newPresetTextField.forceActiveFocus();
                        }
                        else
                            root.addPreset(newPresetTextField.text);
                    }
                }

                FloatingButton { action: closeAction }
            }

            Repeater
            {
                model: document.layoutSettingNames

                LayoutSetting
                {
                    id: layoutSetting

                    Layout.leftMargin: _buttonMenuOffset
                    onValueChanged:
                    {
                        root.document.setLayoutSettingNormalisedValue(modelData, value);
                        root.valueChanged();
                    }

                    onReset:
                    {
                        root.document.resetLayoutSettingValue(modelData);
                        let setting = root.document.layoutSetting(modelData);
                        value = setting.normalisedValue;
                    }

                    Component.onCompleted:
                    {
                        let setting = root.document.layoutSetting(modelData);
                        name = setting.displayName;
                        value = setting.normalisedValue;
                    }

                    Connections
                    {
                        target: root.document

                        function onLayoutSettingChanged(name, value)
                        {
                            if(name !== modelData)
                                return;

                            let setting = root.document.layoutSetting(name);
                            layoutSetting.value = setting.normalisedValue;
                        }
                    }
                }
            }

            ToolBarSeparator
            {
                Layout.fillWidth: true
                Layout.leftMargin: _buttonMenuOffset + Constants.margin
                Layout.rightMargin: Constants.margin
                orientation: Qt.Horizontal
                visible: root.document.layoutSettingNames.length > 0
            }

            LayoutSetting
            {
                id: nodeSizeSetting
                Layout.leftMargin: _buttonMenuOffset
                name: qsTr("Nodes")

                onValueChanged: { root.document.nodeSize = value; }
                onReset: { root.document.resetNodeSize(); }
            }

            LayoutSetting
            {
                id: edgeSizeSetting
                Layout.leftMargin: _buttonMenuOffset
                name: qsTr("Edges")

                onValueChanged: { root.document.edgeSize = value; }
                onReset: { root.document.resetEdgeSize(); }
            }
        }
    }

    Connections
    {
        target: root.document

        function onNodeSizeChanged() { nodeSizeSetting.value = root.document.nodeSize; }
        function onEdgeSizeChanged() { edgeSizeSetting.value = root.document.edgeSize; }
    }

    function show()
    {
        root._visible = true;
        root.evaluateSettings();
        newPresetTextField.reset();
        shown();
    }

    function hide()
    {
        closeAction.trigger();
    }

    function toggle()
    {
        if(root._visible)
            hide();
        else
            show();
    }

    signal shown();
    signal hidden();

    signal valueChanged();
}
