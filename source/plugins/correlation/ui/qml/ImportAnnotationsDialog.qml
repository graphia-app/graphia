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

    property var pluginModel: null

    title: qsTr("Import Column Annotations")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    minimumWidth: 640
    minimumHeight: 400

    property bool _keysSelected: keyHeaderComboBox.enabled
    property bool _validParameters: _keysSelected && headersList.selectedValues.length > 0 &&
        !importAnnotationsKeyDetection.busy

    onVisibleChanged:
    {
        replaceCheckbox.checked = false;
    }

    function open(fileUrl)
    {
        loadingInfo.fileUrl = fileUrl;
        tabularDataParser.parse(fileUrl);
        listTabView.reset();

        importAnnotationsKeyDetection._performedAtLeastOnce = false;
        importAnnotationsKeyDetection.reset();

        root.show();
    }

    TabularDataParser
    {
        id: tabularDataParser

        onDataChanged:
        {
            let idHeaders = columnHeaders(ValueType.Identifier);

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

    ImportAnnotationsKeyDetection
    {
        id: importAnnotationsKeyDetection
        tabularData: tabularDataParser.data
        plugin: root.pluginModel

        property bool _performedAtLeastOnce: false

        function startIfNotPerformed()
        {
            if(keyHeaderComboBox.enabled &&
                !importAnnotationsKeyDetection._performedAtLeastOnce)
            {
                importAnnotationsKeyDetection.start();
            }
        }

        onResultChanged:
        {
            if(result.row === undefined)
                return;

            if(result.row >= 0)
                keyHeaderComboBox.currentIndex = keyHeaderComboBox.model.modelIndexOf(result.row).row;
        }

        onBusyChanged: { _performedAtLeastOnce = true; }
    }

    ListTabView
    {
        id: listTabView
        anchors.fill: parent
        visible: !loadingInfo.visible
        finishEnabled: root._validParameters
        controlsEnabled: !importAnnotationsKeyDetection.busy

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
                    text: qsTr("<h2>Import Annotations</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                Text
                {
                    text: qsTr("Annotation data may be imported from an external source, and superimposed on the " +
                        "plot. Any of the supported tabular data formats can be used. Following the import " +
                        "of new annotations, they can be sorted and grouped in the usual way.")
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
                        qsTr("In order to import annotation data, it is necessary to identify the input data " +
                        "row which corresponds to the column names of the existing data. This is used to correlate " +
                        "columns in the table with columns in the plot. Please select the correct one below. " +
                        "Alternatively, click <i>{0}</i> and the best match will be identified automatically."),
                        autoDetectButton.text)
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }

                RowLayout
                {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: Constants.spacing
                    enabled: !importAnnotationsKeyDetection.busy

                    ComboBox
                    {
                        id: keyHeaderComboBox

                        onCurrentTextChanged: { headersList.updateModel(); }
                        onCurrentIndexChanged:
                        {
                            if(currentIndex !== importAnnotationsKeyDetection.result.row)
                                importAnnotationsKeyDetection.reset();
                        }
                    }

                    Button
                    {
                        id: autoDetectButton

                        text: qsTr("Auto Detect")
                        onClicked: function(mouse) { importAnnotationsKeyDetection.start(); }
                    }
                }

                Text
                {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: Constants.spacing * 2

                    text:
                    {
                        if(importAnnotationsKeyDetection.result.percent !== undefined)
                        {
                            if(importAnnotationsKeyDetection.result.percent > 0)
                                return Utils.format(qsTr("Found {0}% Match"), importAnnotationsKeyDetection.result.percent);

                            return qsTr("No Match Found");
                        }

                        return "";
                    }

                    visible: !importAnnotationsKeyDetection.busy
                }
            }
        }

        ListTab
        {
            title: qsTr("Annotation Selection")

            onActiveChanged:
            {
                if(active)
                    importAnnotationsKeyDetection.startIfNotPerformed();
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
                        text: qsTr("<h2>Annotation Selection</h2>")
                        Layout.alignment: Qt.AlignLeft
                        textFormat: Text.StyledText
                    }

                    Text
                    {
                        text: qsTr("Please select the required annotations from the list on the right. Multiple " +
                            "annotations may be selected by holding down <i>Ctrl</i>, or a range may be selected " +
                            "by holding <i>Shift</i>, then clicking as desired.")
                        wrapMode: Text.WordWrap
                        textFormat: Text.StyledText
                        Layout.fillWidth: true
                    }

                    CheckBox
                    {
                        id: replaceCheckbox
                        text: qsTr("Replace Existing Annotations")
                    }

                    Item { Layout.fillHeight: true }
                }

                ColumnLayout
                {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    enabled: !importAnnotationsKeyDetection.busy

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
                            let headers = tabularDataParser.columnHeaders(ValueType.All,
                                [keyHeaderComboBox.currentText]);
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
                if(active)
                    importAnnotationsKeyDetection.startIfNotPerformed();
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
                        if(importAnnotationsKeyDetection.busy)
                            return qsTr("Detecting key…");

                        if(keyHeaderComboBox.enabled)
                        {
                            return Utils.format(qsTr("The following " +
                                "annotation(s) will be imported using the " +
                                "row <b>{0}</b> as the key:"),
                                keyHeaderComboBox.currentText);
                        }

                        return qsTr("<font color=\"red\">Annotations cannot be " +
                            "imported unless a key is selected.</font>");
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

                        for(let annotation of headersList.selectedValues)
                        {
                            if(summary.length !== 0)
                                summary += "\n";

                            summary += annotation;
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
                console.log("ImportAnnotationsDialog accepted with invalid parameters.");
                return;
            }

            let keyRowIndex = keyHeaderComboBox.model.indexFor(
                keyHeaderComboBox.model.index(keyHeaderComboBox.currentIndex, 0));

            let annotationRowIndices = [];
            for(let selectedIndex of headersList.selectedIndices)
            {
                let rowIndex = headersList.model.indexFor(
                    headersList.model.index(selectedIndex, 0));
                annotationRowIndices.push(rowIndex);
            }

            pluginModel.importAnnotationsFromTable(
                tabularDataParser.data,
                keyRowIndex, annotationRowIndices,
                replaceCheckbox.checked);

            root.accepted();
        }

        onRejected: { root.rejected(); }
    }

    Rectangle
    {
        anchors.centerIn: parent
        visible: importAnnotationsKeyDetection.busy

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
                text: qsTr("Detecting Key")
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

                enabled: importAnnotationsKeyDetection.busy
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
