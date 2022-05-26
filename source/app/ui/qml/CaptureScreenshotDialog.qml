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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt.labs.platform as Labs

import app.graphia
import app.graphia.Shared
import app.graphia.Shared.Controls

Window
{
    id: root
    title: qsTr("Save As Image")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    width: 800
    height: 600

    minimumWidth: 600
    minimumHeight: 400

    property var graphView
    property var application

    property bool loadingPreset: false
    property bool generatingPreview: false

    property real aspectRatio: 0.0
    readonly property real _MM_PER_INCH: 25.4

    Preferences
    {
        id: screenshot
        section: "screenshot"

        property alias dpi: dpiSpin.value
        property var path
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property string backgroundColor
    }

    Timer
    {
        id: refreshTimer
        interval: 300
        onTriggered: { requestPreview(); }
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        spacing: Constants.spacing

        RowLayout
        {
            Text { text: qsTr("Preset:") }

            ComboBox
            {
                id: presets

                Layout.fillWidth: true

                textRole: "text"

                model: ListModel
                {
                    id: presetsListModel

                    function update()
                    {
                        clear();

                        append({text: "Viewport Size (" + graphView.width + "x" + graphView.height + ")",
                                 dpi: 72,
                          pixelWidth: graphView.width,
                         pixelHeight: graphView.height});

                        append({text: "2x Viewport Size (" + graphView.width * 2 + "x" + graphView.height * 2 + ")",
                                 dpi: 72,
                          pixelWidth: graphView.width * 2,
                         pixelHeight: graphView.height * 2});

                        append({text: "4x Viewport Size (" + graphView.width * 4 + "x" + graphView.height * 4 + ")",
                                 dpi: 72,
                          pixelWidth: graphView.width * 4,
                         pixelHeight: graphView.height * 4});

                        append({text: "A5 Paper Portrait (300DPI)",
                                 dpi: 300,
                          printWidth: 148,
                         printHeight: 210});

                        append({text: "A4 Paper Portrait (300DPI)",
                                 dpi: 300,
                          printWidth: 210,
                         printHeight: 297});

                        append({text: "A3 Paper Portrait (300DPI)",
                                 dpi: 300,
                          printWidth: 297,
                         printHeight: 420});

                        append({text: "A2 Paper Portrait (300DPI)",
                                 dpi: 300,
                          printWidth: 420,
                         printHeight: 594});

                        append({text: "A5 Paper Landscape (300DPI)",
                                 dpi: 300,
                          printWidth: 210,
                         printHeight: 148});

                        append({text: "A4 Paper Landscape (300DPI)",
                                 dpi: 300,
                          printWidth: 297,
                         printHeight: 210});

                        append({text: "A3 Paper Landscape (300DPI)",
                                 dpi: 300,
                          printWidth: 420,
                         printHeight: 297});

                        append({text: "A2 Paper Landscape (300DPI)",
                                 dpi: 300,
                          printWidth: 594,
                         printHeight: 420});

                        append({text: "Custom…"});
                    }
                }

                onCurrentIndexChanged:
                {
                    loadPreset(currentIndex);
                }
            }
        }

        RowLayout
        {
            spacing: Constants.spacing * 2

            Rectangle
            {
                Layout.fillHeight: true
                Layout.fillWidth: true

                color: palette.midlight
                border.color: palette.dark
                border.width: 1

                Rectangle
                {
                    anchors.fill: previewImage
                    color: visuals.backgroundColor
                }

                Image
                {
                    id: previewImage
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter

                    property real w: parent.width - (Constants.margin * 2)
                    property real h: parent.height - (Constants.margin * 2)

                    width:  { return Math.min(h * aspectRatio, w); }
                    height: { return Math.min(w / aspectRatio, h); }

                    fillMode: Image.PreserveAspectFit
                    onWidthChanged: { refreshTimer.restart(); }
                    onHeightChanged: { refreshTimer.restart(); }
                }

                Rectangle
                {
                    anchors.fill: previewImage
                    color: "transparent"
                    border.color: QmlUtils.contrastingColor(visuals.backgroundColor)
                    border.width: 1
                }

                DelayedBusyIndicator
                {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    width: 64
                    height: 64
                    delayedRunning: root.generatingPreview
                }
            }

            ColumnLayout
            {
                GridLayout
                {
                    columns: 3

                    Text
                    {
                        text: qsTr("Width:")
                        Layout.alignment: Qt.AlignRight
                    }
                    SpinBox
                    {
                        id: pixelWidthSpin
                        from: 1
                        to: 99999
                        editable: true
                        Component.onCompleted: { contentItem.selectByMouse = true; }

                        onValueChanged: { root.updateValues(pixelWidthSpin); }
                    }
                    Text
                    {
                        text: qsTr("px")
                        Layout.alignment: Qt.AlignLeft
                    }

                    Text
                    {
                        text: qsTr("Height:")
                        Layout.alignment: Qt.AlignRight
                    }
                    SpinBox
                    {
                        id: pixelHeightSpin
                        from: 1
                        to: 99999
                        editable: true
                        Component.onCompleted: { contentItem.selectByMouse = true; }

                        onValueChanged: { root.updateValues(pixelHeightSpin); }
                    }
                    Text
                    {
                        text: qsTr("px")
                        Layout.alignment: Qt.AlignLeft
                    }

                    CheckBox
                    {
                        id: lockAspect
                        Layout.column: 1
                        Layout.row: 2
                        Layout.columnSpan: 2

                        text: qsTr("Lock Aspect Ratio")
                        checked: true
                        onCheckedChanged: { root.updateValues(lockAspect); }
                    }

                    Text
                    {
                        text: qsTr("DPI:")
                        Layout.alignment: Qt.AlignRight
                    }
                    SpinBox
                    {
                        id: dpiSpin
                        Layout.columnSpan: 2
                        from: 1
                        to: 99999
                        editable: true
                        Component.onCompleted: { contentItem.selectByMouse = true; }

                        onValueChanged: { root.updateValues(dpiSpin); }
                    }

                    Text
                    {
                        text: qsTr("Print Width:")
                        Layout.alignment: Qt.AlignRight
                    }
                    SpinBox
                    {
                        id: printWidthSpin
                        from: 1
                        to: 99999
                        editable: true
                        Component.onCompleted: { contentItem.selectByMouse = true; }

                        onValueChanged: { root.updateValues(printWidthSpin); }
                    }
                    Text
                    {
                        text: qsTr("mm")
                        Layout.alignment: Qt.AlignLeft
                    }

                    Text
                    {
                        text: "Print Height:"
                        Layout.alignment: Qt.AlignRight
                    }
                    SpinBox
                    {
                        id: printHeightSpin
                        from: 1
                        to: 99999
                        editable: true
                        Component.onCompleted: { contentItem.selectByMouse = true; }

                        onValueChanged: { root.updateValues(printHeightSpin); }
                    }
                    Text
                    {
                        text: qsTr("mm")
                        Layout.alignment: Qt.AlignLeft
                    }

                    RadioButton
                    {
                        id: fillSize
                        text: "Fill Size"
                        Layout.row: 6
                        Layout.column: 1
                        Layout.columnSpan: 2

                        checked: true
                        onCheckedChanged: { requestPreview(); }
                    }

                    RadioButton
                    {
                        id: fitSize
                        text: "Fit Size"
                        Layout.row: 7
                        Layout.column: 1
                        Layout.columnSpan: 2
                    }
                }
            }
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("Cancel")
                onClicked: function(mouse) { root.close(); }
            }

            Button
            {
                text: qsTr("Save…")
                onClicked: function(mouse)
                {
                    let path = QmlUtils.fileNameForUrl(screenshot.path) + "/" + application.name + "-capture-" +
                        new Date().toLocaleString(Qt.locale(), "yyyy-MM-dd-hhmmss");

                    let fileDialogObject = fileDialogComponent.createObject(root,
                        {"folder": screenshot.path, "currentFile": QmlUtils.urlForFileName(path)});

                    fileDialogObject.open();
                }
            }
        }
    }

    function setPrintWidthByPrintHeight()  { printWidthSpin.value  = Math.round(printHeightSpin.value * aspectRatio) }
    function setPrintHeightByPrintWidth()  { printHeightSpin.value = Math.round(printWidthSpin.value / aspectRatio) }
    function setPixelWidthByPixelHeight()  { pixelWidthSpin.value  = Math.round(pixelHeightSpin.value * aspectRatio) }
    function setPixelHeightByPixelWidth()  { pixelHeightSpin.value = Math.round(pixelWidthSpin.value / aspectRatio) }

    function setPrintWidthByPixelWidth()   { printWidthSpin.value  = Math.round((pixelWidthSpin.value / dpiSpin.value) * _MM_PER_INCH); }
    function setPrintHeightByPixelHeight() { printHeightSpin.value = Math.round((pixelHeightSpin.value / dpiSpin.value) * _MM_PER_INCH); }
    function setPixelWidthByPrintWidth()   { pixelWidthSpin.value  = Math.round((printWidthSpin.value / _MM_PER_INCH) * dpiSpin.value); }
    function setPixelHeightByPrintHeight() { pixelHeightSpin.value = Math.round((printHeightSpin.value / _MM_PER_INCH) * dpiSpin.value); }

    function syncPrintToPixel()
    {
        setPrintWidthByPixelWidth();
        setPrintHeightByPixelHeight();
    }

    function syncPixelToPrint()
    {
        setPixelWidthByPrintWidth();
        setPixelHeightByPrintHeight();
    }

    function updateValues(source)
    {
        if(updateValues.recursing || root.loadingPreset)
            return;

        updateValues.recursing = true;

        if(lockAspect.checked)
        {
            if(source !== pixelWidthSpin)  setPixelWidthByPixelHeight();
            if(source !== pixelHeightSpin) setPixelHeightByPixelWidth();

            if(source !== printWidthSpin)  setPrintWidthByPrintHeight();
            if(source !== printHeightSpin) setPrintHeightByPrintWidth();
        }

        switch(source)
        {
        case pixelWidthSpin:
        case pixelHeightSpin:
            syncPrintToPixel();
            break;

        case printWidthSpin:
        case printHeightSpin:
        case dpiSpin:
            syncPixelToPrint();
            break;
        }

        if(!lockAspect.checked)
            aspectRatio = pixelWidthSpin.value / pixelHeightSpin.value;

        updateValues.recursing = false;
    }

    Component
    {
        // We use a Component here because for whatever reason, the Labs FileDialog only seems
        // to allow you to set currentFile once. From looking at the source code it appears as
        // if setting currentFile adds to the currently selected files, rather than replaces
        // the currently selected files with a new one. Until this is fixed, we work around
        // it by simply recreating the FileDialog everytime we need one.

        id: fileDialogComponent

        Labs.FileDialog
        {
            id: fileDialog
            title: qsTr("Save As Image")

            onAccepted:
            {
                root.close();
                screenshot.path = folder.toString();
                graphView.captureScreenshot(pixelWidthSpin.value, pixelHeightSpin.value,
                    file, dpiSpin.value, fillSize.checked);
            }

            fileMode: Labs.FileDialog.SaveFile
            defaultSuffix: selectedNameFilter.extensions[0]
            selectedNameFilter.index: 0

            function updateFileExtension()
            {
                currentFile = QmlUtils.replaceExtension(currentFile, selectedNameFilter.extensions[0]);
            }

            Component.onCompleted:
            {
                updateFileExtension();
            }

            Connections
            {
                target: fileDialog.selectedNameFilter

                function onIndexChanged()
                {
                    if(fileDialog.visible)
                        fileDialog.updateFileExtension();
                }
            }

            nameFilters: ["PNG Image (*.png)" ,"JPEG Image (*.jpg)", "Bitmap Image (*.bmp)"]
        }
    }

    onAspectRatioChanged:
    {
        if(!loadingPreset)
            refreshTimer.restart();
    }

    onVisibleChanged:
    {
        if(visible)
        {
            presetsListModel.update();
            presets.currentIndex = 0;
            loadPreset(presets.currentIndex);

            requestPreview();
        }
    }

    Connections
    {
        target: graphView

        function onWidthChanged()
        {
            if(root.visible)
                presetsListModel.update();

            loadPreset(presets.currentIndex);
        }

        function onHeightChanged()
        {
            if(root.visible)
                presetsListModel.update();

            loadPreset(presets.currentIndex);
        }

        function onPreviewComplete(previewBase64)
        {
            previewImage.source = "data:image/png;base64," + previewBase64;
            root.generatingPreview = false;
        }
    }

    function requestPreview()
    {
        if(root.visible)
        {
            root.generatingPreview = true;
            graphView.requestPreview(previewImage.width, previewImage.height, fillSize.checked);
        }
    }

    function loadPreset(currentIndex)
    {
        if(currentIndex < 0)
            return;

        loadingPreset = true;

        lockAspect.checked = false;

        let preset = presetsListModel.get(currentIndex);
        if(preset !== undefined && preset.dpi > 0)
        {
            lockAspect.enabled =
                pixelWidthSpin.enabled =
                pixelHeightSpin.enabled =
                dpiSpin.enabled =
                printWidthSpin.enabled =
                printHeightSpin.enabled = false;

            dpiSpin.value = preset.dpi;

            if(preset.pixelWidth > 0 && preset.pixelHeight > 0)
            {
                pixelWidthSpin.value = preset.pixelWidth;
                pixelHeightSpin.value = preset.pixelHeight;
                setPrintWidthByPixelWidth();
                setPrintHeightByPixelHeight();
            }
            else if(preset.printWidth > 0 && preset.printHeight > 0)
            {
                printWidthSpin.value = preset.printWidth;
                printHeightSpin.value = preset.printHeight;
                setPixelWidthByPrintWidth();
                setPixelHeightByPrintHeight();
            }
        }
        else
        {
            lockAspect.enabled =
                pixelWidthSpin.enabled =
                pixelHeightSpin.enabled =
                dpiSpin.enabled =
                printWidthSpin.enabled =
                printHeightSpin.enabled = true;
        }

        aspectRatio = pixelWidthSpin.value / pixelHeightSpin.value;

        lockAspect.checked = true;

        loadingPreset = false;
        requestPreview();
    }
}
