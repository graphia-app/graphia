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

import QtQuick.Controls 2.12
import QtQuick 2.14
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.3
import Qt.labs.qmlmodels 1.0
import QtQuick.Shapes 1.13

import app.graphia 1.0
import app.graphia.Controls 1.0
import app.graphia.Utils 1.0
import app.graphia.Shared 1.0
import app.graphia.Shared.Controls 1.0

import SortFilterProxyModel 0.2

Window
{
    id: root

    property var document: null
    property string attributeName: ""

    property string selectedAttributeName:
    {
        if(attributeName.length === 0)
            return attributeList.selectedValue ? attributeList.selectedValue : "";

        return attributeName;
    }

    title: qsTr("Edit Attribute")
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 500
    minimumHeight: 600

    readonly property double _defaultColumnDivisor: 0.3

    function initialise()
    {
        if(document === null)
            return;

        attributeList.model = document.availableAttributesModel();
        editAllSharedValuesCheckbox.checked = false;
        editAttributeTableModel.attributeName =
            root.attributeName.length !== 0 ? root.attributeName : "";
        editAttributeTableModel.combineSharedValues = false;
        headerView.columnDivisorPosition = _defaultColumnDivisor;
        headerView.forceLayout();
        proxyModel.sortColumn = -1;
        proxyModel.ascendingSortOrder = true;
    }

    onDocumentChanged: { root.initialise(); }

    Connections
    {
        target: document
        function onLoadComplete() { root.initialise(); }
    }

    onVisibleChanged:
    {
        if(!visible)
            root.attributeName = "";

        root.initialise();
    }

    EditAttributeTableModel
    {
        id: editAttributeTableModel
        document: root.document

        onCombineSharedValuesChanged:
        {
            proxyModel.sortColumn = -1;
            proxyModel.ascendingSortOrder = true;
        }
    }

    PlatformMenu
    {
        id: contextMenu
        property int resetRow: -1

        PlatformMenuItem
        {
            text: qsTr("Reset")
            enabled: { return editAttributeTableModel.rowIsEdited(contextMenu.resetRow); }
            onTriggered:
            {
                editAttributeTableModel.resetRowValue(contextMenu.resetRow);
            }
        }

        PlatformMenuItem
        {
            text: qsTr("Reset All")
            enabled: editAttributeTableModel.hasEdits
            onTriggered:
            {
                editAttributeTableModel.resetAllEdits();
            }
        }
    }

    MessageDialog
    {
        id: confirmDiscard
        title: qsTr("Discard Edits")
        text: qsTr("Are you sure you want to discard existing edits?")
        icon: StandardIcon.Warning
        standardButtons: StandardButton.Yes | StandardButton.Cancel

        property string attributeToEdit: ""
        property bool toggleSharedValuesMode: false

        function resetFlags()
        {
            attributeToEdit = "";
            toggleSharedValuesMode = false;
        }

        onYes:
        {
            if(attributeToEdit.length > 0)
                editAttributeTableModel.attributeName = attributeToEdit;
            else if(toggleSharedValuesMode)
                editAttributeTableModel.combineSharedValues = editAllSharedValuesCheckbox.checked;

            resetFlags();
        }

        onRejected:
        {
            if(toggleSharedValuesMode)
                editAllSharedValuesCheckbox.checked = !editAllSharedValuesCheckbox.checked;

            resetFlags();
        }
    }

    ColumnLayout
    {
        enabled: root.document !== null && !root.document.busy

        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        RowLayout
        {
            Layout.fillWidth: true
            spacing: Constants.spacing

            Text
            {
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop

                text: (root.attributeName.length === 0 ? qsTr("Please select an attribute. ") : "") +
                    qsTr("Double click on a value to edit it. " +
                    "For attributes with shared values, selecting <i>Edit All Shared Values</i> " +
                    "allows for bulk editing many attribute values at once.")
            }

            NamedIcon
            {
                iconName: "document-properties"

                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                Layout.alignment: Qt.AlignTop
            }
        }

        TreeComboBox
        {
            id: attributeList

            visible: root.attributeName.length === 0

            Layout.fillWidth: true

            placeholderText: qsTr("Select an Attribute")

            showSections: sortRoleName !== "display"
            sortRoleName: "elementType"
            prettifyFunction: Attribute.prettify

            filters:
            [
                ValueFilter
                {
                    roleName: "editable"
                    value: true
                }
            ]

            onSelectedValueChanged:
            {
                if(selectedValue === undefined || editAttributeTableModel.attributeName === selectedValue)
                    return;

                if(editAttributeTableModel.hasEdits)
                {
                    confirmDiscard.attributeToEdit = selectedValue;
                    confirmDiscard.open();
                }
                else
                    editAttributeTableModel.attributeName = selectedValue;
            }
        }

        CheckBox
        {
            id: editAllSharedValuesCheckbox

            visible:
            {
                if(root.document === null)
                    return false;

                let attribute = document.attribute(root.selectedAttributeName);

                if(!attribute.isValid)
                    return false;

                return attribute.sharedValues.length > 0;
            }

            text: qsTr("Edit All Shared Values")

            onCheckedChanged:
            {
                if(confirmDiscard.toggleSharedValuesMode)
                    return;

                // Confirm edit discard when going from non-combined to combined
                if(checked && editAttributeTableModel.hasEdits)
                {
                    confirmDiscard.toggleSharedValuesMode = true;
                    confirmDiscard.open();
                    return;
                }

                editAttributeTableModel.combineSharedValues = checked;
            }
        }

        SystemPalette { id: sysPalette }
        FontMetrics { id: fontMetrics }

        Label
        {
            visible: !tableView.visible

            Layout.fillWidth: true
            Layout.fillHeight: true

            horizontalAlignment: Qt.AlignCenter
            verticalAlignment: Qt.AlignVCenter
            font.pixelSize: 16
            font.italic: true

            text: qsTr("Select an Attribute")
        }

        Frame
        {
            visible: root.selectedAttributeName.length > 0

            Layout.fillWidth: true
            Layout.fillHeight: true

            topPadding: 0
            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0

            ColumnLayout
            {
                anchors.fill: parent
                anchors.margins: 1

                spacing: 0

                //FIXME for reasons not understood, the header and table vanish
                // if this dummy Item isn't included
                Item { width: 1 }

                TableView
                {
                    id: headerView

                    Layout.fillWidth: true
                    Layout.preferredHeight: fontMetrics.height + 8

                    interactive: false
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    model: TableModel
                    {
                        TableModelColumn { display: "id" }
                        TableModelColumn { display: "name" }

                        rows:
                        {
                            let emptyHeader = [{ "id": "", "name": ""}];

                            if(root.document === null)
                                return emptyHeader;

                            let attribute = document.attribute(root.selectedAttributeName);

                            if(!attribute.isValid)
                                return emptyHeader;

                            if(attribute.elementType === ElementType.Node)
                            {
                                if(editAttributeTableModel.combineSharedValues)
                                    return [{ "id": qsTr("Number of Nodes"), "name": qsTr("Value")}];

                                return [{ "id": qsTr("Node Name"), "name": qsTr("Value")}];
                            }

                            if(attribute.elementType === ElementType.Edge)
                            {
                                if(editAttributeTableModel.combineSharedValues)
                                    return [{ "id": qsTr("Number of Edges"), "name": qsTr("Value")}];

                                return [{ "id": qsTr("Edge Identifier"), "name": qsTr("Value")}];
                            }
                        }
                    }

                    property double columnDivisorPosition: 0.5

                    onWidthChanged: { headerView.forceLayout(); }
                    columnWidthProvider: function(column)
                    {
                        return column === 0 ?
                            headerView.width * columnDivisorPosition :
                            headerView.width * (1.0 - columnDivisorPosition);
                    }

                    readonly property int sortIndicatorWidth: 7
                    readonly property int sortIndicatorHeight: 4
                    readonly property int sortIndicatorMargin: 3
                    readonly property int labelPadding: 4

                    delegate: Item
                    {
                        id: headerItem

                        implicitHeight: headerLabel.height + (headerView.labelPadding * 2)

                        Item
                        {
                            anchors.fill: parent
                            clip: true

                            Rectangle
                            {
                                anchors.fill: parent
                                color: headerMouseArea.containsMouse && headerMouseArea.cursorShape !== Qt.SizeHorCursor ?
                                    Qt.lighter(sysPalette.highlight, 2.0) : sysPalette.light
                            }

                            Text
                            {
                                id: headerLabel

                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.leftMargin: headerView.labelPadding

                                width: parent.width - (headerView.sortIndicatorMargin + headerView.sortIndicatorWidth)

                                clip: true
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                                renderType: Text.NativeRendering
                                color: sysPalette.text
                                text: model.display
                            }

                            Shape
                            {
                                id: sortIndicator
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: headerView.sortIndicatorMargin
                                antialiasing: false
                                width: headerView.sortIndicatorWidth
                                height: headerView.sortIndicatorHeight
                                visible: proxyModel.sortColumn === model.column
                                transform: Rotation
                                {
                                    origin.x: sortIndicator.width * 0.5
                                    origin.y: sortIndicator.height * 0.5
                                    angle: proxyModel.ascendingSortOrder ? 180 : 0
                                }

                                ShapePath
                                {
                                    miterLimit: 0
                                    strokeColor: sysPalette.mid
                                    fillColor: "transparent"
                                    strokeWidth: 2
                                    startY: sortIndicator.height - 1
                                    PathLine { x: Math.round((sortIndicator.width - 1) * 0.5); y: 0 }
                                    PathLine { x: sortIndicator.width - 1; y: sortIndicator.height - 1 }
                                }
                            }

                            Rectangle
                            {
                                visible: model.column === 0

                                anchors.right: parent.right
                                width: 1
                                height: parent.height
                                color: sysPalette.midlight
                            }

                            Rectangle
                            {
                                anchors.bottom: parent.bottom
                                width: parent.width
                                height: 1
                                color: sysPalette.midlight
                            }

                            MouseArea
                            {
                                id: headerMouseArea
                                anchors.fill: parent

                                property bool mouseOverResizeHandle:
                                {
                                    let halfWidth = 5;

                                    let atLeft = mouseX <= halfWidth;
                                    let atRight = mouseX >= (width - halfWidth);

                                    if(atLeft && model.column === 0)
                                        return false;

                                    if(atRight && model.column === (headerView.model.columnCount - 1))
                                        return false;

                                    return atLeft || atRight;
                                }

                                cursorShape: mouseOverResizeHandle || resizing ? Qt.SizeHorCursor : Qt.ArrowCursor

                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton|Qt.RightButton

                                onClicked:
                                {
                                    if(!mouseOverResizeHandle && mouse.button === Qt.LeftButton)
                                    {
                                        if(proxyModel.sortColumn === model.column)
                                            proxyModel.ascendingSortOrder = !proxyModel.ascendingSortOrder;
                                        else
                                            proxyModel.sortColumn = model.column;
                                    }
                                }

                                onDoubleClicked:
                                {
                                    if(mouseOverResizeHandle)
                                    {
                                        headerView.columnDivisorPosition = _defaultColumnDivisor;
                                        headerView.forceLayout();
                                    }
                                }

                                drag.target: parent
                                drag.axis: Drag.XAxis
                                drag.threshold: 0

                                property bool resizing: false

                                onMouseXChanged:
                                {
                                    if(drag.active)
                                    {
                                        if(mouseOverResizeHandle)
                                            resizing = true;
                                    }
                                    else
                                        resizing = false;

                                    if(resizing)
                                    {
                                        let d = (headerItem.x + mouseX) / headerView.width;
                                        d = Utils.clamp(d, 0.1, 0.9);

                                        headerView.columnDivisorPosition = d;
                                        headerView.forceLayout();
                                    }
                                }

                                onReleased: { resizing = false; }
                            }
                        }
                    }
                }

                TableView
                {
                    id: tableView

                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    ScrollBar.vertical: ScrollBar
                    {
                        z: 100
                        id: verticalTableViewScrollBar
                        policy: ScrollBar.AsNeeded
                        contentItem: Rectangle
                        {
                            implicitWidth: 5
                            radius: width / 2
                            color: sysPalette.dark
                        }
                        minimumSize: 0.1
                        visible: size < 1.0 && tableView.rows > 0
                    }

                    property var activeEditField: null

                    model: SortFilterProxyModel
                    {
                        id: proxyModel

                        property int sortColumn: -1

                        sourceModel: editAttributeTableModel

                        sorters: StringSorter
                        {
                            roleName:
                            {
                                switch(proxyModel.sortColumn)
                                {
                                case 0: return "label"
                                case 1: return "attribute";
                                }

                                return "";
                            }

                            sortOrder: proxyModel.ascendingSortOrder ?
                                Qt.AscendingOrder : Qt.DescendingOrder
                            numericMode: true
                        }
                    }

                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    syncView: headerView
                    syncDirection: Qt.Horizontal

                    delegate: Item
                    {
                        // Based on Qt source for BaseTableView delegate
                        implicitHeight: editField.implicitHeight + 1
                        implicitWidth: label.implicitWidth + 16

                        clip: false

                        Rectangle
                        {
                            anchors.fill: parent

                            color: model.row % 2 ? sysPalette.window : sysPalette.alternateBase

                            Text
                            {
                                id: label

                                visible: !editField.visible

                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.leftMargin: 10

                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                                color: QmlUtils.contrastingColor(parent.color)
                                font.bold: model.edited

                                text: model.display

                                MouseArea
                                {
                                    anchors.fill: parent

                                    acceptedButtons: Qt.LeftButton|Qt.RightButton

                                    onClicked:
                                    {
                                        if(mouse.button === Qt.RightButton)
                                        {
                                            let row = model.index % proxyModel.rowCount();
                                            contextMenu.resetRow = proxyModel.mapToSource(row);
                                            contextMenu.popup();
                                        }

                                        if(tableView.activeEditField !== null && tableView.activeEditField !== editField)
                                            tableView.activeEditField.cancel();
                                    }

                                    onDoubleClicked:
                                    {
                                        if(model.column !== 1)
                                            return;

                                        if(mouse.button === Qt.LeftButton)
                                            editField.visible = true;
                                    }
                                }
                            }

                            TextField
                            {
                                id: editField

                                selectByMouse: true

                                function cancel()
                                {
                                    text = model.display;
                                    visible = false;
                                }

                                visible: false
                                onVisibleChanged:
                                {
                                    if(visible)
                                    {
                                        text = model.display;
                                        selectAll();
                                        forceActiveFocus();

                                        tableView.activeEditField = editField;
                                    }
                                    else
                                        tableView.activeEditField = null;
                                }

                                onEditingFinished:
                                {
                                    if(visible)
                                    {
                                        let row = model.index % proxyModel.rowCount();
                                        row = proxyModel.mapToSource(row);
                                        editAttributeTableModel.editValue(row, text);
                                        visible = false;
                                    }
                                }

                                Keys.onEscapePressed: { cancel(); }

                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.leftMargin: 5
                                anchors.rightMargin: 10

                                property var _doubleValidator: DoubleValidator{}
                                property var _intValidator: IntValidator{}

                                validator:
                                {
                                    if(root.document === null)
                                        return null;

                                    let attribute = document.attribute(root.selectedAttributeName);
                                    if(attribute === null || !attribute.isValid)
                                        return null;

                                    switch(attribute.valueType)
                                    {
                                    case ValueType.Float:   return _doubleValidator;
                                    case ValueType.Int:     return _intValidator;
                                    }

                                    return null;
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout
        {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                enabled: root.selectedAttributeName.length > 0 && editAttributeTableModel.hasEdits

                onClicked:
                {
                    document.editAttribute(root.selectedAttributeName, editAttributeTableModel.edits);
                    root.close();
                }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: { root.close(); }
            }
        }
    }
}
