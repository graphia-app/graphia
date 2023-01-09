/* Copyright © 2013-2023 Graphia Technologies Ltd.
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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import app.graphia
import app.graphia.Controls
import app.graphia.Utils
import app.graphia.Shared
import app.graphia.Shared.Controls

Window
{
    id: root

    property var document: null

    title: qsTr("Import Attributes")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    minimumWidth: 640
    minimumHeight: 400

    property bool _keysSelected: keyAttributeList.selectedValue !== undefined && keyHeaderComboBox.enabled
    property bool _validParameters: _keysSelected && headersList.selectedValues.length > 0 &&
        !importAttributesKeyDetection.busy

    onVisibleChanged:
    {
        if(!visible)
            keyAttributeList.model = null;
    }

    function open(fileUrl)
    {
        let attributeNames = document.availableAttributeNames(
            ElementType.NodeAndEdge, ValueType.Identifier);

        let nonUserDefinedAttributeNames = attributeNames.filter(
            attributeName => !document.attribute(attributeName).userDefined);

        keyAttributeList.model = document.availableAttributesModel(
            ElementType.NodeAndEdge, ValueType.Identifier,
            AttributeFlag.None, nonUserDefinedAttributeNames);

        loadingInfo.fileUrl = fileUrl;
        tabularDataParser.parse(fileUrl);
        listTabView.reset();

        importAttributesKeyDetection._performedAtLeastOnce = false;
        importAttributesKeyDetection.reset();

        root.show();
    }

    TabularDataParser
    {
        id: tabularDataParser

        onDataChanged:
        {
            let idHeaders = headers(ValueType.Identifier);

            // ComboBox gets upset if you change one or other of these
            // on live data, so reset them first
            keyHeaderComboBox.model = undefined;
            keyHeaderComboBox.textRole = "";

            if(!idHeaders || idHeaders.rowCount() === 0)
            {
                keyHeaderComboBox.model = [qsTr("No Suitable Keys")];
                keyHeaderComboBox.enabled = false;
            }
            else
            {
                keyHeaderComboBox.textRole = "display";
                keyHeaderComboBox.model = idHeaders;
                keyHeaderComboBox.enabled = true;
            }

            // Work around for (another) ComboBox quirk where no
            // initial selection is made
            keyHeaderComboBox.currentIndex = -1;
            keyHeaderComboBox.currentIndex = 0;

            headersList.updateModel();
        }
    }

    ColumnLayout
    {
        id: loadingInfo

        property string fileUrl: ""
        property string baseFileName: fileUrl && fileUrl.length > 0 ?
            QmlUtils.baseFileNameForUrl(fileUrl) : ""

        anchors.centerIn: parent
        visible: !tabularDataParser.complete || tabularDataParser.failed

        Text
        {
            text: Utils.format(tabularDataParser.failed ?
                qsTr("Failed to Load {0}.") : qsTr("Loading {0}…"), loadingInfo.baseFileName)
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignHCenter

            ProgressBar
            {
                visible: !tabularDataParser.failed
                value: tabularDataParser.progress >= 0.0 ? tabularDataParser.progress / 100.0 : 0.0
                indeterminate: tabularDataParser.progress < 0.0
            }

            Button
            {
                text: tabularDataParser.failed ? qsTr("Close") : qsTr("Cancel")
                onClicked: function(mouse) { root.rejected(); }
            }
        }
    }

    ImportAttributesKeyDetection
    {
        id: importAttributesKeyDetection
        tabularData: tabularDataParser.data
        document: root.document

        property bool _performedAtLeastOnce: false

        onResultChanged:
        {
            if(result.attributeName === undefined || result.column === undefined)
                return;

            if(result.attributeName.length > 0 && result.column >= 0)
            {
                let modelIndex = keyAttributeList.model.find(result.attributeName);

                if(modelIndex.valid)
                    keyAttributeList.select(modelIndex);

                keyHeaderComboBox.currentIndex = keyHeaderComboBox.model.indexOf(result.column).row;
            }
        }

        onBusyChanged: { _performedAtLeastOnce = true; }
    }

    ListTabView
    {
        id: listTabView
        anchors.fill: parent
        visible: !loadingInfo.visible
        finishEnabled: root._validParameters
        controlsEnabled: !importAttributesKeyDetection.busy

        ListTab
        {
            title: qsTr("Introduction")
            ColumnLayout
            {
                width: parent.width
                anchors.left: parent.left
                anchors.right: parent.right

                Text
                {
                    text: qsTr("<h2>Import Attributes</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                Text
                {
                    text: qsTr("Attribute data may be imported from an external source, and superimposed on the " +
                        "open graph. Any of the supported tabular data formats can be used. Following the import " +
                        "of new attributes, they can be used in transforms or visualised using the normal methods.")
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }
            }
        }

        ListTab
        {
            title: qsTr("Key Selection")
            ColumnLayout
            {
                anchors.left: parent.left
                anchors.right: parent.right

                Text
                {
                    text: qsTr("<h2>Key Selection</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                Text
                {
                    text: Utils.format(
                        qsTr("In order to import attribute data, it is necessary to identify an attribute and " +
                        "column pair on the graph and the imported table respectively. These are used to correlate " +
                        "rows in the table with graph elements. Please select these below. Alternatively, click " +
                        "<i>{0}</i> and the best match will be identified automatically."), autoDetectButton.text)
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }

                RowLayout
                {
                    Layout.topMargin: Constants.spacing
                    enabled: !importAttributesKeyDetection.busy

                    TreeComboBox
                    {
                        id: keyAttributeList
                        Layout.fillWidth: true
                        implicitWidth: 160

                        placeholderText: qsTr("Select a Key Attribute")

                        showSections: true
                        sortRoleName: "elementType"
                        prettifyFunction: Attribute.prettify

                        property string elementType:
                        {
                            if(document)
                            {
                                let attribute = document.attribute(keyAttributeList.selectedValue);
                                switch(attribute.elementType)
                                {
                                case ElementType.Node: return qsTr("Node");
                                case ElementType.Edge: return qsTr("Edge");
                                }
                            }

                            return qsTr("Unknown");
                        }

                        property string display:
                        {
                            if(keyAttributeList.selectedValue)
                                return keyAttributeList.prettifyFunction(keyAttributeList.selectedValue);

                            return "";
                        }

                        onCurrentIndexChanged:
                        {
                            if(!importAttributesKeyDetection.result.attributeName || !keyAttributeList.model)
                                return;

                            let detectedIndex = keyAttributeList.model.find(importAttributesKeyDetection.result.attributeName);
                            if(currentIndex !== detectedIndex)
                                importAttributesKeyDetection.reset();
                        }
                    }

                    Label { text: qsTr("Corresponds To") }

                    ComboBox
                    {
                        id: keyHeaderComboBox
                        Layout.fillWidth: true
                        implicitWidth: keyAttributeList.implicitWidth

                        onCurrentTextChanged: { headersList.updateModel(); }
                        onCurrentIndexChanged:
                        {
                            if(currentIndex !== importAttributesKeyDetection.result.column)
                                importAttributesKeyDetection.reset();
                        }
                    }
                }

                RowLayout
                {
                    enabled: !importAttributesKeyDetection.busy

                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: Constants.spacing * 2

                    Button
                    {
                        id: autoDetectButton

                        text: qsTr("Auto Detect")
                        onClicked: function(mouse) { importAttributesKeyDetection.start(); }
                    }

                    Text
                    {
                        text:
                        {
                            if(importAttributesKeyDetection.result.percent !== undefined)
                            {
                                if(importAttributesKeyDetection.result.percent > 0)
                                    return Utils.format(qsTr("Found {0}% Match"), importAttributesKeyDetection.result.percent);

                                return qsTr("No Match Found");
                            }

                            return "";
                        }

                        visible: !importAttributesKeyDetection.busy
                    }
                }
            }
        }

        ListTab
        {
            title: qsTr("Attribute Selection")

            onActiveChanged:
            {
                if(active && !keyAttributeList.selectedValue && !importAttributesKeyDetection._performedAtLeastOnce)
                    importAttributesKeyDetection.start();
            }

            RowLayout
            {
                anchors.fill: parent

                ColumnLayout
                {
                    Layout.maximumWidth: 250
                    Layout.fillHeight: true

                    Text
                    {
                        text: qsTr("<h2>Attribute Selection</h2>")
                        Layout.alignment: Qt.AlignLeft
                        textFormat: Text.StyledText
                    }

                    Text
                    {
                        text: qsTr("Please select the required attributes from the list on the right. Multiple " +
                            "attributes may be selected by holding down <i>Ctrl</i> or a range may be selected " +
                            "by holding <i>Shift</i>, then clicking as desired.")
                        wrapMode: Text.WordWrap
                        textFormat: Text.StyledText
                        Layout.fillWidth: true
                    }

                    CheckBox
                    {
                        id: replaceCheckbox
                        text: qsTr("Replace Existing Attributes")
                    }

                    Item { Layout.fillHeight: true }
                }

                ColumnLayout
                {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    enabled: !importAttributesKeyDetection.busy

                    RowLayout
                    {
                        Layout.fillWidth: true

                        Button
                        {
                            Layout.fillWidth: true

                            text: qsTr("Select All")
                            action: selectAllAction
                        }

                        Button
                        {
                            Layout.fillWidth: true

                            text: qsTr("Select None")
                            action: selectNoneAction
                        }
                    }

                    ListBox
                    {
                        id: headersList

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        allowMultipleSelection: true

                        function updateModel()
                        {
                            let headers = tabularDataParser.headers(ValueType.All,
                                keyHeaderComboBox.currentText);
                            model = headers;
                            selectAll();
                        }

                        Action
                        {
                            id: selectAllAction
                            onTriggered: { headersList.selectAll(); }
                            shortcut: "Ctrl+A"
                        }

                        Action
                        {
                            id: selectNoneAction
                            onTriggered: { headersList.clearSelection(); }
                            shortcut: "Ctrl+N"
                        }
                    }
                }
            }
        }

        ListTab
        {
            title: qsTr("Summary")

            onActiveChanged:
            {
                if(active && !keyAttributeList.selectedValue && !importAttributesKeyDetection._performedAtLeastOnce)
                    importAttributesKeyDetection.start();
            }

            ColumnLayout
            {
                anchors.fill: parent

                Text
                {
                    text: qsTr("<h2>Summary</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                Text
                {
                    text:
                    {
                        if(importAttributesKeyDetection.busy)
                            return qsTr("Detecting keys…");

                        if(keyAttributeList.selectedValue !== undefined && keyHeaderComboBox.enabled)
                        {
                            return Utils.format(qsTr("The following <b>{0}</b> " +
                                "attribute(s) will be imported using the attribute <b>{1}</b> " +
                                "and the column <b>{2}</b> as the keys:"),
                                keyAttributeList.elementType, keyAttributeList.display,
                                keyHeaderComboBox.currentText);
                        }

                        return qsTr("<font color=\"red\">Attributes cannot be " +
                            "imported unless keys are selected.</font>");
                    }
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                    Layout.fillWidth: true
                }

                ScrollableTextArea
                {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    readOnly: true

                    text:
                    {
                        if(!root._keysSelected)
                            return "";

                        let summary = "";

                        for(let attribute of headersList.selectedValues)
                        {
                            if(summary.length !== 0)
                                summary += "\n";

                            summary += attribute;
                        }

                        return summary;
                    }
                }
            }
        }

        onAccepted:
        {
            if(!root._validParameters)
            {
                console.log("ImportAttributesDialog accepted with invalid parameters.");
                return;
            }

            let keyAttributeName = keyAttributeList.selectedValue;
            let keyColumnIndex = keyHeaderComboBox.model.columnIndexFor(
                    keyHeaderComboBox.model.index(keyHeaderComboBox.currentIndex, 0));

            let attributeColumnIndices = [];
            for(let selectedIndex of headersList.selectedIndices)
            {
                let columnIndex = headersList.model.columnIndexFor(
                    headersList.model.index(selectedIndex, 0));
                attributeColumnIndices.push(columnIndex);
            }

            document.importAttributesFromTable(
                keyAttributeName, tabularDataParser.data,
                keyColumnIndex, attributeColumnIndices,
                replaceCheckbox.checked);

            root.accepted();
        }

        onRejected: { root.rejected(); }
    }

    Rectangle
    {
        anchors.centerIn: parent
        visible: importAttributesKeyDetection.busy

        width: 150
        height: 150

        color: palette.light
        border.width: 1
        border.color: palette.dark
        radius: 5

        ColumnLayout
        {
            anchors.fill: parent
            anchors.margins: Constants.margin

            Text
            {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Detecting Keys")
            }

            BusyIndicator
            {
                visible: parent.visible

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
                running: true
            }

            Button
            {
                id: cancelButton
                Layout.alignment: Qt.AlignHCenter

                text: qsTr("Cancel")

                enabled: importAttributesKeyDetection.busy
                onClicked: function(mouse) { importAttributesKeyDetection.cancel(); }
            }
        }
    }

    onAccepted: { root.close(); }

    onRejected:
    {
        if(!tabularDataParser.complete)
            tabularDataParser.cancelParse();

        root.close();
    }

    signal accepted()
    signal rejected()
}
