/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

import app.graphia
import app.graphia.Controls
import app.graphia.Utils

Window
{
    id: root
    title: qsTr("Save As Image")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    color: palette.window

    width: 800
    height: 600

    minimumWidth: 600
    minimumHeight: 400

    property var graphView
    property var application

    property bool loadingPreset: false
    property bool generatingPreview: false
    property bool _previewEnabled: false
    property bool _previewCompleted: false

    property real aspectRatio: 1.0
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

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        spacing: Constants.spacing

        RowLayout
        {
            Text
            {
                text: qsTr("Preset:")
                color: palette.buttonText
            }

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

                        append({text: Utils.format(qsTr("Viewport Size ({0}×{1})"), graphView.width, graphView.height),
                                 dpi: 72,
                          pixelWidth: graphView.width,
                         pixelHeight: graphView.height});

                        append({text: Utils.format(qsTr("2× Viewport Size ({0}×{1})"), graphView.width * 2, graphView.height * 2),
                                 dpi: 72,
                          pixelWidth: graphView.width * 2,
                         pixelHeight: graphView.height * 2});

                        append({text: Utils.format(qsTr("4× Viewport Size ({0}×{1})"), graphView.width * 4, graphView.height * 4),
                                 dpi: 72,
                          pixelWidth: graphView.width * 4,
                         pixelHeight: graphView.height * 4});

                        append({text: qsTr("A5 Paper Portrait (300DPI)"),
                                 dpi: 300,
                          printWidth: 148,
                         printHeight: 210});

                        append({text: qsTr("A4 Paper Portrait (300DPI)"),
                                 dpi: 300,
                          printWidth: 210,
                         printHeight: 297});

                        append({text: qsTr("A3 Paper Portrait (300DPI)"),
                                 dpi: 300,
                          printWidth: 297,
                         printHeight: 420});

                        append({text: qsTr("A2 Paper Portrait (300DPI)"),
                                 dpi: 300,
                          printWidth: 420,
                         printHeight: 594});

                        append({text: qsTr("A5 Paper Landscape (300DPI)"),
                                 dpi: 300,
                          printWidth: 210,
                         printHeight: 148});

                        append({text: qsTr("A4 Paper Landscape (300DPI)"),
                                 dpi: 300,
                          printWidth: 297,
                         printHeight: 210});

                        append({text: qsTr("A3 Paper Landscape (300DPI)"),
                                 dpi: 300,
                          printWidth: 420,
                         printHeight: 297});

                        append({text: qsTr("A2 Paper Landscape (300DPI)"),
                                 dpi: 300,
                          printWidth: 594,
                         printHeight: 420});

                        append({text: qsTr("Custom…")});
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

                color: palette.alternateBase
                border.color: ControlColors.outline
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
                    onWidthChanged: { requestPreviewDelayed(); }
                    onHeightChanged: { requestPreviewDelayed(); }
                }

                Rectangle
                {
                    anchors.fill: previewImage
                    color: "transparent"
                    border.color: ControlColors.outline
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
                        color: palette.buttonText
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
                        color: palette.buttonText
                    }

                    Text
                    {
                        text: qsTr("Height:")
                        Layout.alignment: Qt.AlignRight
                        color: palette.buttonText
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
                        color: palette.buttonText
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
                        color: palette.buttonText
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
                        color: palette.buttonText
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
                        color: palette.buttonText
                    }

                    Text
                    {
                        text: qsTr("Print Height:")
                        Layout.alignment: Qt.AlignRight
                        color: palette.buttonText
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
                        color: palette.buttonText
                    }

                    RadioButton
                    {
                        id: fillSize
                        text: qsTr("Fill Size")
                        Layout.row: 6
                        Layout.column: 1
                        Layout.columnSpan: 2

                        checked: true
                        onCheckedChanged: { requestPreview(); }
                    }

                    RadioButton
                    {
                        id: fitSize
                        text: qsTr("Fit Size")
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
                    let path = Utils.format(qsTr("{0}/{1}-capture-{2}"),
                        NativeUtils.fileNameForUrl(screenshot.path), application.name,
                        new Date().toLocaleString(Qt.locale(), "yyyy-MM-dd-hhmmss"));

                    fileDialog.currentFolder = screenshot.path;
                    fileDialog.selectedFile = NativeUtils.urlForFileName(path);
                    fileDialog.open();
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

    SaveFileDialog
    {
        id: fileDialog

        title: qsTr("Save As Image")
        nameFilters: [qsTr("PNG Image (*.png)"), qsTr("JPEG Image (*.jpg)"), qsTr("Bitmap Image (*.bmp)")]

        onAccepted:
        {
            root.close();
            screenshot.path = fileDialog.currentFolder.toString();
            graphView.captureScreenshot(pixelWidthSpin.value, pixelHeightSpin.value,
                fileDialog.selectedFile, dpiSpin.value, fillSize.checked);
        }
    }

    onAspectRatioChanged:
    {
        if(!loadingPreset)
            requestPreviewDelayed();
    }

    onVisibleChanged:
    {
        if(visible)
        {
            presetsListModel.update();
            presets.currentIndex = 0;
            loadPreset(presets.currentIndex);

            root._previewEnabled = true;
            requestPreview();
        }
        else
            root._previewEnabled = root._previewCompleted = false;
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
            root._previewCompleted = true;
        }
    }

    function requestPreview()
    {
        if(root._previewEnabled)
        {
            root.generatingPreview = true;
            graphView.requestPreview(previewImage.width, previewImage.height, fillSize.checked);
        }
    }

    Timer
    {
        id: refreshTimer
        interval: 300
        onTriggered: { requestPreview(); }
    }

    function requestPreviewDelayed()
    {
        if(!root._previewCompleted)
            requestPreview(); // Immediate if preview is yet to complete
        else
            refreshTimer.restart();
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
