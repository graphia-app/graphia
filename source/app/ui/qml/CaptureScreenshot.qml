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

import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import Qt.labs.platform 1.0 as Labs

import app.graphia 1.0
import app.graphia.Shared 1.0

Window
{
    id: root
    title: qsTr("Save As Image")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    width: 800
    height: 600

    minimumWidth: 400
    minimumHeight: 300

    // This is used to compare the aspect ratios of the screenshots
    property var graphView
    property var application

    property int screenshotWidth: pixelWidthSpin.value
    property int screenshotHeight: pixelHeightSpin.value
    property int dpi: dpiSpin.value
    property bool fill: fillSize.checked
    property bool loadingPreset: false;

    property real aspectRatio: 0
    readonly property real _MMTOINCH: 0.0393701;

    Preferences
    {
        id: screenshot
        section: "screenshot"
        property alias width: root.screenshotWidth
        property alias height: root.screenshotHeight
        property alias dpi: root.dpi
        property var path
    }

    Timer
    {
        id: refreshTimer
        interval: 300
        onTriggered:
        {
            requestPreview();
        }
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

            ColumnLayout
            {
                Rectangle
                {
                    id: previewHolder
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    width: 200
                    height: 200
                    border.color: palette.dark
                    border.width: 1

                    Canvas
                    {
                        id: underCanvas
                        height: parent.height
                        width: parent.width
                        onPaint: function(rect)
                        {
                            let ctx = getContext("2d");

                            ctx.save();
                            ctx.clearRect(0, 0, width, height);

                            ctx.fillStyle = "white"
                            let sizedWidth = Math.ceil(aspectRatio * canvas.height);
                            let sizedHeight = canvas.height;
                            if(sizedWidth > canvas.width)
                            {
                                sizedWidth = canvas.width;
                                sizedHeight = Math.ceil(canvas.width / aspectRatio);
                            }

                            ctx.fillRect(Math.ceil((width - sizedWidth) * 0.5),
                                         Math.ceil((height - sizedHeight) * 0.5),
                                         sizedWidth, sizedHeight);

                            ctx.restore();
                        }
                    }
                    Image
                    {
                        id: preview
                        visible: false
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: previewWidth()
                        height: previewHeight()
                        fillMode: Image.PreserveAspectFit
                        onWidthChanged: refreshTimer.restart()
                        onHeightChanged: refreshTimer.restart()
                    }
                    BusyIndicator
                    {
                        id: previewGenerating
                        anchors.horizontalCenter: previewHolder.horizontalCenter
                        anchors.verticalCenter: previewHolder.verticalCenter
                        width: parent.width / 4.0
                        height: parent.height / 4.0
                        running: !preview.visible
                    }
                    Canvas
                    {
                        id: canvas
                        height: parent.height
                        width: parent.width
                        onPaint: function(rect)
                        {
                            let ctx = getContext("2d");

                            ctx.save();
                            ctx.clearRect(0, 0, width, height);

                            ctx.strokeStyle = "black"
                            let sizedWidth = Math.ceil(aspectRatio * canvas.height);
                            let sizedHeight = canvas.height;
                            if(sizedWidth > canvas.width)
                            {
                                sizedWidth = canvas.width;
                                sizedHeight = Math.ceil(canvas.width / aspectRatio);
                            }
                            ctx.strokeRect(Math.ceil((width - sizedWidth) * 0.5),
                                           Math.ceil((height - sizedHeight) * 0.5) + 1,
                                           sizedWidth, sizedHeight - 1);
                            ctx.restore();
                        }
                    }
                }
            }

            ColumnLayout
            {
                GridLayout
                {
                    columns: 2
                    Text
                    {
                        text: qsTr("Width:")
                        Layout.alignment: Qt.AlignRight
                    }
                    SpinBox
                    {
                        id: pixelWidthSpin
                        to: 99999
                        from: 1
                        textFromValue: (value, locale) => qsTr("%1px").arg(value)
                        onValueChanged:
                        {
                            if(lockAspect.checked)
                                pixelHeightSpin.value = pixelWidthSpin.value / aspectRatio;
                            else
                                aspectRatio = pixelWidthSpin.value / pixelHeightSpin.value;

                            let pixelPrintSize = Math.round(printWidthSpin.value * dpiSpin.value * _MMTOINCH);
                            if(pixelPrintSize !== value)
                                printWidthSpin.value = (value / dpiSpin.value) / _MMTOINCH;
                        }
                    }

                    Text
                    {
                        text: qsTr("Height:")
                        Layout.alignment: Qt.AlignRight
                    }
                    SpinBox
                    {
                        id: pixelHeightSpin
                        to: 99999
                        from: 1
                        textFromValue: (value, locale) => qsTr("%1px").arg(value)
                        onValueChanged:
                        {
                            if(lockAspect.checked)
                                pixelWidthSpin.value = pixelHeightSpin.value * aspectRatio
                            else
                                aspectRatio = pixelWidthSpin.value / pixelHeightSpin.value;

                            let pixelPrintSize = Math.round(printHeightSpin.value * dpiSpin.value * _MMTOINCH);
                            if(pixelPrintSize !== value)
                                printHeightSpin.value = (value / dpiSpin.value) / _MMTOINCH;
                        }
                    }
                    CheckBox
                    {
                        id: lockAspect
                        Layout.column: 1
                        Layout.row: 2
                        text: qsTr("Lock Aspect Ratio")
                        checked: true
                        onCheckedChanged:
                        {
                            aspectRatio = pixelWidthSpin.value / pixelHeightSpin.value
                        }
                    }
                    Text
                    {
                        text: qsTr("DPI:")
                        Layout.alignment: Qt.AlignRight
                    }
                    SpinBox
                    {
                        id: dpiSpin
                        to: 9999
                        from: 1
                        onValueChanged:
                        {
                            let lockAspectWasChecked = lockAspect.checked;
                            lockAspect.checked = false;
                            pixelWidthSpin.value = printWidthSpin.value * _MMTOINCH * value;
                            pixelHeightSpin.value = printHeightSpin.value * _MMTOINCH * value;
                            lockAspect.checked = lockAspectWasChecked
                        }
                    }
                    Text { text: qsTr("Print Width:") }
                    SpinBox
                    {
                        id: printWidthSpin
                        to: 99999
                        from: 1
                        textFromValue: (value, locale) => qsTr("%1mm").arg(value)
                        onValueChanged:
                        {
                            // Prevent binding loops + losing value precision
                            let pixelPrintSize = Math.round((pixelWidthSpin.value / dpiSpin.value) / _MMTOINCH);
                            if(pixelPrintSize !== value)
                                pixelWidthSpin.value = value * dpiSpin.value * _MMTOINCH;
                        }
                    }

                    Text { text: "Print Height:" }
                    SpinBox
                    {
                        id: printHeightSpin
                        to: 99999
                        from: 1
                        textFromValue: (value, locale) => qsTr("%1mm").arg(value)
                        onValueChanged:
                        {
                            let pixelPrintSize = Math.round((pixelHeightSpin.value / dpiSpin.value) / _MMTOINCH);
                            if(pixelPrintSize !== value)
                                pixelHeightSpin.value = value * dpiSpin.value * _MMTOINCH;
                        }
                    }

                    RadioButton
                    {
                        id: fillSize
                        text: "Fill Size"
                        Layout.row: 6
                        Layout.column: 1

                        checked: true
                        onCheckedChanged:
                        {
                            preview.visible = false;
                            requestPreview();
                        }
                    }

                    RadioButton
                    {
                        id: fitSize
                        text: "Fit Size"
                        Layout.row: 7
                        Layout.column: 1
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
                graphView.captureScreenshot(screenshotWidth, screenshotHeight, file, dpi, fill);
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
        underCanvas.requestPaint();
        canvas.requestPaint();
        preview.visible = false;
        if(!loadingPreset)
            refreshTimer.restart();
    }

    onVisibleChanged:
    {
        if(visible)
        {
            presetsListModel.update();
            presets.currentIndex = 0;

            aspectRatio = screenshot.width / screenshot.height;
            pixelWidthSpin.value = screenshot.width;
            pixelHeightSpin.value = screenshot.height;

            loadPreset(presets.currentIndex);

            preview.visible = false;
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
    }

    Connections
    {
        target: graphView

        function onPreviewComplete(previewBase64)
        {
            preview.source = "data:image/png;base64," + previewBase64;
            preview.visible = true;
        }

        function onHeightChanged()
        {
            loadPreset(presets.currentIndex);
        }
    }

    function requestPreview()
    {
        if(root.visible)
        {
            let w = Math.ceil(aspectRatio * canvas.height);
            let h = canvas.height;
            if(w > canvas.width)
            {
                w = canvas.width;
                h = Math.ceil(canvas.width / aspectRatio);
            }
            graphView.requestPreview(w, h, fillSize.checked);
        }
    }

    function previewWidth()
    {
        return Math.min(previewHolder.height * aspectRatio, previewHolder.width);
    }

    function previewHeight()
    {
        return Math.min(previewWidth() / aspectRatio, previewHolder.height);
    }

    function loadPreset(currentIndex)
    {
        loadingPreset = true;
        pixelWidthSpin.enabled = false;
        pixelHeightSpin.enabled = false;
        dpiSpin.enabled = false;
        printHeightSpin.enabled = false;
        printWidthSpin.enabled = false;
        lockAspect.enabled = false;

        lockAspect.checked = false;

        if(currentIndex < 0)
            return;

        let preset = presetsListModel.get(currentIndex);
        if(preset !== undefined && preset.dpi > 0)
        {
            dpiSpin.value = preset.dpi;

            if(preset.pixelWidth > 0 && preset.pixelHeight > 0)
            {
                pixelWidthSpin.value = preset.pixelWidth;
                pixelHeightSpin.value = preset.pixelHeight;
            }
            else if(preset.printWidth > 0 && preset.printHeight > 0)
            {
                printWidthSpin.value = preset.printWidth;
                printHeightSpin.value = preset.printHeight;
            }
        }
        else
        {
            lockAspect.enabled =
                pixelWidthSpin.enabled =
                pixelHeightSpin.enabled =
                dpiSpin.enabled =
                printHeightSpin.enabled =
                printWidthSpin.enabled = true;
        }

        lockAspect.checked = true;

        loadingPreset = false;
        requestPreview();
    }
}
