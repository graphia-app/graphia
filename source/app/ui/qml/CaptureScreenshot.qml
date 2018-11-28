import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import Qt.labs.platform 1.0 as Labs

import com.kajeka 1.0

import "../../../shared/ui/qml/Constants.js" as Constants

Dialog
{
    id: root
    title: qsTr("Save As Image")
    width: 600
    height: 400

    standardButtons: StandardButton.Cancel | StandardButton.Save

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

    SystemPalette { id: systemPalette }

    onAccepted:
    {
        var path = QmlUtils.fileNameForUrl(screenshot.path) + "/" + application.name + "-capture-" +
            new Date().toLocaleString(Qt.locale(), "yyyy-MM-dd-hhmmss");

        var fileDialogObject = fileDialogComponent.createObject(root,
            {"folder": screenshot.path, "currentFile": QmlUtils.urlForFileName(path)});

        fileDialogObject.open();
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
        spacing: Constants.spacing

        RowLayout
        {
            Text { text: qsTr("Preset:") }
            ComboBox
            {
                Layout.fillWidth: true
                id: presets

                Component.onCompleted:
                {
                    presetsListModel.update();
                    model = presetsListModel;
                }

                ListModel
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
                                 dpi: 72,
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
                    border.color: systemPalette.dark
                    border.width: 1

                    Canvas
                    {
                        id: underCanvas
                        height: parent.height
                        width: parent.width
                        onPaint:
                        {
                            var ctx = getContext("2d");

                            ctx.save();
                            ctx.clearRect(0, 0, width, height);

                            ctx.fillStyle = "white"
                            var sizedWidth = Math.ceil(aspectRatio * canvas.height);
                            var sizedHeight = canvas.height;
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
                        onPaint:
                        {
                            var ctx = getContext("2d");

                            ctx.save();
                            ctx.clearRect(0, 0, width, height);

                            ctx.strokeStyle = "black"
                            var sizedWidth = Math.ceil(aspectRatio * canvas.height);
                            var sizedHeight = canvas.height;
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
                        maximumValue: 99999
                        minimumValue: 1
                        suffix: "px"
                        onValueChanged:
                        {
                            if(lockAspect.checked)
                                pixelHeightSpin.value = pixelWidthSpin.value / aspectRatio;
                            else
                                aspectRatio = pixelWidthSpin.value / pixelHeightSpin.value;

                            var pixelPrintSize = Math.round(printWidthSpin.value * dpiSpin.value * _MMTOINCH);
                            if(pixelPrintSize != value)
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
                        maximumValue: 99999
                        minimumValue: 1
                        suffix: "px"
                        onValueChanged:
                        {
                            if(lockAspect.checked)
                                pixelWidthSpin.value = pixelHeightSpin.value * aspectRatio
                            else
                                aspectRatio = pixelWidthSpin.value / pixelHeightSpin.value;

                            var pixelPrintSize = Math.round(printHeightSpin.value * dpiSpin.value * _MMTOINCH);
                            if(pixelPrintSize != value)
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
                        maximumValue: 9999
                        minimumValue: 1
                        onValueChanged:
                        {
                            var lockAspectWasChecked = lockAspect.checked;
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
                        maximumValue: 99999
                        minimumValue: 1
                        suffix: "mm"
                        onValueChanged:
                        {
                            // Prevent binding loops + losing value precision
                            var pixelPrintSize = Math.round((pixelWidthSpin.value / dpiSpin.value) / _MMTOINCH);
                            if(pixelPrintSize != value)
                                pixelWidthSpin.value = value * dpiSpin.value * _MMTOINCH;
                        }
                    }

                    Text { text: "Print Height:" }
                    SpinBox
                    {
                        id: printHeightSpin
                        maximumValue: 99999
                        minimumValue: 1
                        suffix: "mm"
                        onValueChanged:
                        {
                            var pixelPrintSize = Math.round((pixelHeightSpin.value / dpiSpin.value) / _MMTOINCH);
                            if(pixelPrintSize != value)
                                pixelHeightSpin.value = value * dpiSpin.value * _MMTOINCH;
                        }
                    }

                    ExclusiveGroup
                    {
                        id: fitGroup
                        onCurrentChanged:
                        {
                            preview.visible = false;
                            requestPreview();
                        }
                    }
                    RadioButton
                    {
                        id: fillSize
                        text: "Fill Size"
                        exclusiveGroup: fitGroup
                        Layout.row: 6
                        Layout.column: 1
                        checked: true
                    }
                    RadioButton
                    {
                        id: fitSize
                        text: "Fit Size"
                        Layout.row: 7
                        Layout.column: 1
                        exclusiveGroup: fitGroup
                    }
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

                onIndexChanged:
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

    Component.onCompleted:
    {
        aspectRatio = screenshot.width / screenshot.height
        pixelWidthSpin.value = screenshot.width
        pixelHeightSpin.value = screenshot.height

        loadPreset(presets.currentIndex);
    }

    onVisibleChanged:
    {
        if(visible)
        {
            aspectRatio = screenshot.width / screenshot.height
            pixelWidthSpin.value = screenshot.width
            pixelHeightSpin.value = screenshot.height

            preview.visible = false;
            requestPreview();
        }
    }

    Connections
    {
        target: graphView
        onWidthChanged:
        {
            presetsListModel.update();
            loadPreset(presets.currentIndex);
        }
        onHeightChanged:
        {
            presetsListModel.update();
            loadPreset(presets.currentIndex);
        }
    }

    Connections
    {
        target: graphView
        onPreviewComplete:
        {
            preview.source = "data:image/png;base64," + previewBase64;
            preview.visible = true;
        }
        onHeightChanged:
        {
            loadPreset(presets.currentIndex);
        }
    }

    function requestPreview()
    {
        if(root.visible)
        {
            var previewWidth = Math.ceil(aspectRatio * canvas.height);
            var previewHeight = canvas.height;
            if(previewWidth > canvas.width)
            {
                previewWidth = canvas.width;
                previewHeight = Math.ceil(canvas.width / aspectRatio);
            }
            graphView.requestPreview(previewWidth, previewHeight, fillSize.checked);
        }
    }

    function previewWidth()
    {
        var previewWidth = Math.min(previewHolder.height * aspectRatio, previewHolder.width);
        return previewWidth;
    }

    function previewHeight()
    {
        var previewHeight = Math.min(previewWidth() / aspectRatio, previewHolder.height);
        return previewHeight;
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

        var preset = presetsListModel.get(currentIndex);
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
