import QtQuick.Controls 1.5
import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Private 1.0 // StyleItem
import QtGraphicalEffects 1.0
import QtQml.Models 2.13
import QtQuick.Shapes 1.13

import Qt.labs.platform 1.0 as Labs

import com.kajeka 1.0

import "Controls"
import "../../../shared/ui/qml/Utils.js" as Utils
import "../../../shared/ui/qml/Constants.js" as Constants

Item
{
    id: root

    // External name
    property alias model: root._nodeAttributesTableModel
    property var defaultColumnWidth: 120
    property var selectedRows: []
    property alias rowCount: tableView.rows
    property alias sortIndicatorColumn: proxyModel.sortColumn
    property alias sortIndicatorOrder: proxyModel.sortOrder

    // Internal name
    property var _nodeAttributesTableModel

    function resizeColumnsToContents()
    {
        // Resizing columns to contents is difficult with TableView2. The solution here
        // is to connect the tableview delegates to a signal so they report their implicitWidths
        // when the fetchColumnSizes signal is emitted. This will only fetch the sizes for the delegates
        // currently on screen. We store previous widths calculated in the currentColumnWidths array
        // and partially update them each time this function is called to best-guess offscreen columns
        tableView.userColumnWidths = [];
        tableView.fetchColumnSizes();
        if(tableView.columnWidths.length > 0)
        {
            // Update array
            for(let i=0; i<tableView.columns; i++)
            {
                let newValue = tableView.columnWidths[i];
                if(newValue !== undefined)
                    tableView.currentColumnWidths[i] = newValue;
            }

            let tempArr = tableView.currentColumnWidths;
            tableView.currentColumnWidths = [];
            tableView.currentColumnWidths = tempArr;

            tableView.columnWidths = []
        }
        // We update the property target width here to stop the columns constantly
        // adjusting due to the   binding to tableView.width
        tableView.targetTotalColumnWidth = tableView.width;

        tableView.currentTotalColumnWidth = 0;
        for(let i=0; i<tableView.columns; i++)
        {
            var tempCalculatedWidth = tableView.calculateMinimumColumnWidth(i);
            tableView.currentTotalColumnWidth += tempCalculatedWidth;
        }

        resizeColumnHeaders();

        tableView.forceLayoutSafe();
    }
    function resizeColumnHeaders()
    {
        for(let i=0; i<displayedGroup.count; i++)
        {
            var columnHeader = columnHeaderRepeater.itemAt(i);
            columnHeader.width = tableView.columnWidthProvider(i);
        }
    }

    property var hiddenColumns: []

    property bool columnSelectionMode: false
    onColumnSelectionModeChanged:
    {
        let indexArray = Array.from(new Array(_nodeAttributesTableModel.columnNames.length).keys());


        if(columnSelectionMode)
        {
            columnSelectionControls.show();
            headerDelegateModel.filterOnGroup = "items"
        }
        else
        {
            columnSelectionControls.hide();

            headerDelegateModel.filterOnGroup = "displayed"
        }
        tableView._updateColumnVisibility();
        tableView.forceLayoutSafe();

        resizeColumnHeaders();

        //proxyModel.columnOrder = headerDelegateModel.columnOrder();
    }

    property alias viewport: tableView.childrenRect

    signal visibleRowsChanged();

    function setColumnVisibility(sourceColumnIndex, columnVisible)
    {
        if(columnVisible)
            hiddenColumns = Utils.setRemove(hiddenColumns, sourceColumnIndex);
        else
            hiddenColumns = Utils.setAdd(hiddenColumns, sourceColumnIndex);
    }

    function showAllColumns()
    {
        hiddenColumns = [];
    }

    function showAllCalculatedColumns()
    {
        var columns = hiddenColumns;
        hiddenColumns = [];
        plugin.model.nodeAttributeTableModel.columnNames.forEach(function(columnName, index)
        {
            if(root._nodeAttributesTableModel.columnIsCalculated(columnName))
                columns = Utils.setRemove(columns, index);
        });

        hiddenColumns = columns;
    }

    function hideAllColumns()
    {
        var columns = Array.from(new Array(_nodeAttributesTableModel.columnNames.length).keys());
        hiddenColumns = columns;
    }

    function hideAllCalculatedColumns()
    {
        var columns = hiddenColumns;
        hiddenColumns = [];
        plugin.model.nodeAttributeTableModel.columnNames.forEach(function(columnName, index)
        {
            if(root._nodeAttributesTableModel.columnIsCalculated(columnName))
                columns = Utils.setAdd(columns, index);
        });

        hiddenColumns = columns;
    }

    function populateTableMenu(menu)
    {
        if(menu === null)
            return;

        // Clear out any existing items
        while(menu.items.length > 0)
            menu.removeItem(menu.items[0]);

        menu.title = qsTr("&Table");

        menu.addItem("").action = resizeColumnsToContentsAction;
        menu.addItem("").action = selectColumnsAction;
        menu.addItem("").action = exportTableAction;
        menu.addSeparator();
        menu.addItem("").action = selectAllTableAction;

        tableView._tableMenu = menu;
        Utils.cloneMenu(menu, contextMenu);
    }

    function selectAll()
    {
        selectRows(0, proxyModel.rowCount() - 1);
    }

    Label
    {
        text: qsTr("No Visible Columns")
        visible: tableView.columns <= 0

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    Preferences
    {
        id: misc
        section: "misc"

        property var fileSaveInitialFolder
    }

    Labs.FileDialog
    {
        id: exportTableDialog
        visible: false
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: selectedNameFilter.extensions[0]
        title: qsTr("Export Table")
        nameFilters: ["CSV File (*.csv)", "TSV File (*.tsv)"]
        onAccepted:
        {
            misc.fileSaveInitialFolder = folder.toString();
            document.writeTableView2ToFile(tableView, file, defaultSuffix);
        }
    }

    property alias exportAction: exportTableAction

    Action
    {
        id: exportTableAction
        enabled: tableView.rows > 0
        text: qsTr("Export…")
        iconName: "document-save"
        onTriggered:
        {
            exportTableDialog.folder = misc.fileSaveInitialFolder !== undefined ?
                        misc.fileSaveInitialFolder : "";

            exportTableDialog.open();
        }
    }

    Action
    {
        id: selectAllTableAction
        text: qsTr("Select All")
        iconName: "edit-select-all"
        enabled: tableView.rows > 0

        onTriggered: { root.selectAll(); }
    }

    SystemPalette { id: sysPalette }

    function selectRows(inStartRow, inEndRow)
    {
        let less = Math.min(inStartRow, inEndRow);
        let max = Math.max(inStartRow, inEndRow);

        var range = proxyModel.buildRowSelectionRange(less, max);
        selectionModel.select([range], ItemSelectionModel.Rows | ItemSelectionModel.Select)

        root.selectedRows = selectionModel.selectedRows(0).map(index => proxyModel.mapToSourceRow(index.row));
    }

    function deselectRows(inStartRow, inEndRow)
    {
        let less = Math.min(inStartRow, inEndRow);
        let max = Math.max(inStartRow, inEndRow);

        var range = proxyModel.buildRowSelectionRange(less, max);
        selectionModel.select([range], ItemSelectionModel.Rows | ItemSelectionModel.Deselect)

        root.selectedRows = selectionModel.selectedRows(0).map(index => proxyModel.mapToSourceRow(index.row));
    }

    ItemSelectionModel
    {
        id: selectionModel
        model: proxyModel
        onSelectionChanged:
        {
            proxyModel.setSubSelection(selected, deselected);
        }
    }

    MouseArea
    {
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        property var previousRow: -1
        property var startRow: -1
        property var endRow: -1
        property var offsetMouseY: mouseY + anchors.topMargin
        anchors.topMargin: columnsHeader.implicitHeight
        anchors.fill: parent
        anchors.rightMargin: verticalTableViewScrollBar.width
        anchors.bottomMargin: horizontalTableViewScrollBar.height
        z: 3
        hoverEnabled: true
        visible: !columnSelectionMode

        onDoubleClicked:
        {
            var tableItem = tableView.getItem(mouseX, offsetMouseY);
            if(tableItem === false || !tableItem.hasOwnProperty('modelRow'))
                return;
            var mappedRow = proxyModel.mapToSourceRow(tableItem.modelRow);
            root._nodeAttributesTableModel.moveFocusToNodeForRowIndex(mappedRow);
        }

        onClicked:
        {
            if(mouse.button == Qt.RightButton)
                root.rightClick();
        }

        onPressed:
        {
            if(tableView.rows === 0)
                return;

            var tableItem = tableView.getItem(mouseX, offsetMouseY);
            if(tableItem === false || !tableItem.hasOwnProperty('modelRow'))
                return;
            startRow = tableItem.modelRow;

            if(mouse.modifiers & Qt.ShiftModifier)
            {
                if(endRow != -1)
                {
                    selectRows(startRow, endRow);
                }
            }
            else if(mouse.modifiers & Qt.ControlModifier)
            {

            }
            else
            {
                selectionModel.clear();
            }

            endRow = tableItem.modelRow;
            selectRows(startRow, startRow);
        }
        onReleased:
        {
            previousRow = -1;
        }
        onPositionChanged:
        {
            if(mouse.buttons == Qt.LeftButton)
            {
                var tableItem = tableView.getItem(mouseX, offsetMouseY);
                if(tableItem && tableItem.modelRow !== previousRow)
                {
                    if(previousRow != -1)
                        deselectRows(startRow, previousRow);

                    selectRows(startRow, tableItem.modelRow);

                    previousRow = tableItem.modelRow;
                    endRow = tableItem.modelRow
                }
            }
        }
    }

    ColumnLayout
    {
        anchors.fill: parent

        Item
        {
            Layout.fillWidth: true
            height: 15
            ScrollView
            {
                visible: columnSelectionMode
                anchors.fill: parent
                flickableItem.contentX: tableView.contentX
                flickableItem.contentY: 0
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                Row
                {
                    id: columnsHeader
                    z: 2
                    Repeater
                    {
                        id: columnHeaderRepeater
                        model: headerDelegateModel

                        onItemAdded:
                        {
                            tableView.forceLayoutSafe();
                        }
                    }
                }
            }

            TableView
            {
                id: headerView
                model: tableView.model
                width: parent.width
                height: 15
                visible: !columnSelectionMode
                interactive: false
                rowHeightProvider: function(row)
                {
                    return row > 0 ? 0 : -1;
                }
                columnWidthProvider: tableView.columnWidthProvider;
                delegate: Item
                {
                    property var modelIndex: index
                    implicitWidth: columnHeaderRepeater.itemAt(model.column).width
                    implicitHeight: headerLabelView.height

                    id: headerDelegate
                    Rectangle
                    {
                        anchors.fill: parent
                        color: sysPalette.light
                    }
                    Label
                    {
                        id: headerLabelView
                        clip: true
                        maximumLineCount: 1
                        width: parent.width
                        text:
                        {
                            let headerIndex = displayedGroup.get(model.column).model.modelData;
                            return root._nodeAttributesTableModel.columnNames[headerIndex];
                        }

                        color: sysPalette.text
                        font.pixelSize: 11
                        padding: 4
                        renderType: Text.NativeRendering
                    }
                    Rectangle
                    {
                        anchors.right: parent.right
                        height: parent.height
                        width: 1
                        color: sysPalette.midlight
                    }
                }
            }
        }

        TableView
        {
            id: tableView
            property var userColumnWidths: []
            property var currentColumnWidths: []
            property var currentTotalColumnWidth: 0
            property var targetTotalColumnWidth: 0
            property var columnWidths: []
            property var headerColumns: Array.from(new Array(_nodeAttributesTableModel.columnNames.length).keys())
            property var visibleColumns: headerColumns
            signal fetchColumnSizes;

            clip: true
            interactive: false
            visible: true
            Layout.fillHeight: true
            Layout.fillWidth: true

            onContentXChanged:
            {
                headerView.contentX = contentX;
            }

            QQC2.ScrollBar.vertical: QQC2.ScrollBar
            {
                z: 100
                id: verticalTableViewScrollBar
                policy: Qt.ScrollBarAsNeeded
                contentItem: Rectangle
                {
                    implicitWidth: 5
                    radius: width / 2
                    color: sysPalette.dark
                }
            }

            QQC2.ScrollBar.horizontal: QQC2.ScrollBar
            {
                id: horizontalTableViewScrollBar
            }

            model: TableProxyModel
            {
                id: proxyModel
                sourceModel: root._nodeAttributesTableModel
            }

            columnWidthProvider: function(col)
            {
                var calculatedWidth = 0;
                var userWidth = userColumnWidths[proxyModel.mapToSourceColumn(col)];

                /*
                console.log("CWP", col, "UW", userWidth,
                            "Map", proxyModel.mapToSourceColumn(col), "CMW", calculateMinimumColumnWidth(col),
                            "HeaderText", columnHeaderRepeater.itemAt(col).labelText);
       */

            // Use the user specified column width if available
            if(userWidth !== undefined)
                calculatedWidth = userWidth;
            else
            {
                calculatedWidth = calculateMinimumColumnWidth(col);

                // Scale columns to fill the width of the view if the totalMinimimum is less than the view
                if(tableView.currentTotalColumnWidth > 0 && targetTotalColumnWidth > 0 && tableView.currentTotalColumnWidth < targetTotalColumnWidth)
                {
                    let scaledWidth = (calculatedWidth / tableView.currentTotalColumnWidth) * targetTotalColumnWidth;
                    return scaledWidth;
                }
            }

            return calculatedWidth;
        }

        property var visibleColumnNames:
        {
            var columnNameList = [];
            for(var i=0; i<tableView.headerColumns.length; i++)
                columnNameList.push(root._nodeAttributesTableModel.columnNames[i])
            return columnNameList;
        }

        function forceLayoutSafe()
        {
            if(tableView.rows > 0 && tableView.columns > 0)
            {
                tableView.forceLayout();
                headerView.forceLayout();
            }
        }

        function getItem(mouseX, mouseY)
        {
            var tableViewContentContainsMouse = mouseY > columnsHeader.height && mouseY < tableView.height &&
                    mouseX < tableView.width && mouseX < tableView.contentWidth
                    && mouseY < tableView.contentHeight + columnsHeader.height;

            if(!tableViewContentContainsMouse)
                return false;

            var hoverItem = tableView.childAt(mouseX, mouseY);
            if(hoverItem !== null && (hoverItem === horizontalTableViewScrollBar || hoverItem === verticalTableViewScrollBar))
                return false;

            var tableItem = hoverItem.childAt(
                        mouseX + tableView.contentX,
                        mouseY + tableView.contentY);
            return tableItem;
        }

        function calculateMinimumColumnWidth(col)
        {
            var delegateWidth = tableView.currentColumnWidths[col];
            if(columnHeaderRepeater.itemAt(col) === null)
            {
                console.log("Null CMCW", columnHeaderRepeater.count, col);
                return 120;
            }
            if(delegateWidth === undefined)
                return Math.max(defaultColumnWidth, columnHeaderRepeater.itemAt(col).labelWidth);
            else
                return Math.max(delegateWidth, columnHeaderRepeater.itemAt(col).labelWidth);
        }

        DelegateModel
        {
            id: headerDelegateModel
            model: tableView.headerColumns
            filterOnGroup: "displayed"
            groups:
            [
                DelegateModelGroup
                {
                    id: displayedGroup
                    name: "displayed"
                    includeByDefault: true
                }
            ]

            delegate: DropArea
            {
                id: headerItem
                width: Math.max(defaultColumnWidth, labelWidth)
                Behavior on width
                {
                    enabled: !resizeHandleMouseArea.drag.active
                    PropertyAnimation { easing.type: Easing.InOutQuad }
                }
                Behavior on x { PropertyAnimation { easing.type: Easing.InOutQuad } }
                height: headerLabel.height
                property var modelIndex: index
                property var labelWidth: headerLabel.contentWidth + headerLabel.padding
                                         + headerLabel.padding + sortIndicator.marginWidth
                                         + sortIndicator.anchors.rightMargin;
                property int visualIndex: DelegateModel.itemsIndex
                property int displayedIndex: DelegateModel.displayedIndex
                property var labelText: headerLabel.text
                Binding { target: headerContent; property: "visualIndex"; value: visualIndex }
                Binding { target: headerContent; property: "displayedIndex"; value: displayedIndex }

                onEntered:
                {
                    let tempWidth = tableView.currentColumnWidths[displayedIndex];
                    tableView.currentColumnWidths[displayedIndex] = tableView.currentColumnWidths[drag.source.displayedIndex];
                    tableView.currentColumnWidths[drag.source.displayedIndex] = tempWidth;

                    headerDelegateModel.items.move(drag.source.visualIndex, visualIndex)

                    tableView.forceLayoutSafe();
                }
                Rectangle
                {
                    anchors.fill: parent
                    visible: dragHandler.active
                    color: Qt.lighter(sysPalette.highlight, 1.99)
                }

                Item
                {
                    id: headerContent
                    opacity: Drag.active ? 0.5 : 1
                    property int visualIndex: 0
                    property int displayedIndex: 0
                    property int sourceColumn: 0
                    width: headerItem.width
                    height: headerItem.height
                    anchors.left: parent.left
                    anchors.top: parent.top

                    states: [
                        State {
                            when: headerContent.Drag.active
                            ParentChange {
                                target: headerContent
                                parent: columnHeaderRepeater
                            }
                            AnchorChanges {
                                target: headerContent
                                anchors.left: undefined
                                anchors.top: undefined
                            }
                        }
                    ]

                    Rectangle
                    {
                        anchors.fill: parent
                        color: headerMouseArea.containsMouse ? Qt.lighter(sysPalette.highlight, 1.99) : sysPalette.light
                    }
                    CheckBox
                    {
                        id: checkbox

                        anchors.verticalCenter: parent.verticalCenter
                        visible: columnSelectionMode
                        text: root._nodeAttributesTableModel.columnNames[modelData]
                        height: headerLabel.height

                        function isChecked()
                        {
                            return !Utils.setContains(root.hiddenColumns, modelData);
                        }
                        checked: { return isChecked(); }
                        onCheckedChanged:
                        {
                            // Unbind to prevent binding loop
                            checked = checked;
                            root.setColumnVisibility(modelData, checked);

                            // Rebind so that the delegate doesn't hold the state
                            checked = Qt.binding(isChecked);
                            headerItem.DelegateModel.inDisplayed = checked
                        }
                    }
                    QQC2.Label
                    {
                        id: headerLabel
                        visible: !columnSelectionMode
                        clip: true
                        //elide: Text.ElideRight
                        maximumLineCount: 1
                        width: parent.width - (sortIndicator.marginWidth);
                        text: root._nodeAttributesTableModel.columnNames[modelData]
                        //text: "Test";
                        //text: modelData + " W: " + headerItem + " H:" + headerItem.width.toFixed(0) + " D:" //+ tableView.columnWidthProvider(headerItem.displayedIndex)
                        color: sysPalette.text
                        font.pixelSize: 11
                        padding: 4
                        renderType: Text.NativeRendering
                    }
                    Shape
                    {
                        id: sortIndicator
                        property var marginWidth: width + anchors.rightMargin
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 3
                        antialiasing: false
                        width: 7
                        height: 4
                        visible: proxyModel.sortColumn == modelData && !columnSelectionMode
                        transform: Rotation
                        {
                            origin.x: sortIndicator.width * 0.5
                            origin.y: sortIndicator.height * 0.5
                            angle: proxyModel.sortOrder == Qt.DescendingOrder ? 0 : 180
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

                    DragHandler
                    {
                        id: dragHandler
                        yAxis.enabled: false
                    }

                    Drag.active: dragHandler.active
                    Drag.source: headerContent
                    Drag.hotSpot.x: headerContent.width * 0.5
                    Drag.hotSpot.y: headerContent.height * 0.5
                    property bool dragActive: Drag.active
                    onDragActiveChanged:
                    {
                        if(!dragActive)
                        {
                            var columnOrder = [];
                            for(let i=0; i<headerDelegateModel.items.count; i++)
                            {
                                var modelItem = headerDelegateModel.items.get(i);
                                columnOrder.push(modelItem.model.modelData);
                            }
                            proxyModel.columnOrder = columnOrder;
                        }
                    }

                    MouseArea
                    {
                        id: headerMouseArea
                        enabled: !columnSelectionMode
                        anchors.fill: headerContent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton

                        onClicked:
                        {
                            if(proxyModel.sortColumn == modelData)
                                proxyModel.sortOrder = proxyModel.sortOrder ? Qt.AscendingOrder : Qt.DescendingOrder;
                            else
                                proxyModel.sortColumn = modelData;

                        }
                    }

                    Rectangle
                    {
                        anchors.right: parent.right
                        height: parent.height
                        width: 1
                        color: sysPalette.midlight
                        MouseArea
                        {
                            id: resizeHandleMouseArea
                            cursorShape: Qt.SizeHorCursor
                            width: 5
                            height: parent.height
                            anchors.horizontalCenter: parent.horizontalCenter
                            drag.target: parent
                            drag.axis: Drag.XAxis

                            onMouseXChanged:
                            {
                                if(drag.active)
                                {
                                    tableView.userColumnWidths[modelData] = Math.max(30, headerItem.width + mouseX);
                                    headerItem.width = tableView.userColumnWidths[modelData];
                                    tableView.forceLayoutSafe();
                                }
                            }
                        }
                    }
                }
            }
        }

        // Ripped more or less verbatim from qtquickcontrols/src/controls/Styles/Desktop/TableViewStyle.qml
        // except for the text property
        delegate: Item
        {
            // Based on Qt source for BaseTableView delegate
            implicitHeight: Math.max(16, label.implicitHeight)
            implicitWidth: label.implicitWidth + 16

            clip: true

            property var modelColumn: model.column
            property var modelRow: model.row
            property var modelIndex: model.index

            TableView.onReused:
            {
                tableView.fetchColumnSizes.connect(updateColumnWidths)
            }

            TableView.onPooled:
            {
                tableView.fetchColumnSizes.disconnect(updateColumnWidths)
            }

            Component.onCompleted:
            {
                tableView.fetchColumnSizes.connect(updateColumnWidths)
            }

            function updateColumnWidths()
            {
                if(modelIndex === undefined)
                    return;
                var storedWidth = tableView.columnWidths[modelColumn];
                if(storedWidth !== undefined)
                    tableView.columnWidths[modelColumn] = Math.max(implicitWidth, storedWidth);
                else
                    tableView.columnWidths[modelColumn] = implicitWidth;
            }

            SystemPalette { id: systemPalette }

            Rectangle
            {
                width: parent.width

                anchors.centerIn: parent
                height: parent.height
                color:
                {
                    if(model.subSelected)
                    {
                        return systemPalette.highlight;
                    }
                    return model.row % 2 ? sysPalette.window : sysPalette.alternateBase;
                }

                Text
                {
                    id: label
                    objectName: "label"
                    elide: Text.ElideRight
                    width: parent.width
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 10
                    color: QmlUtils.contrastingColor(parent.color)

                    text:
                    {
                        let sourceColumn = proxyModel.mapToSourceColumn(modelColumn);

                        // This can happen during column removal
                        if(sourceColumn === undefined || sourceColumn < 0)
                        {
                            console.log("Model Column Unable to map", modelRow, modelColumn);
                            return "";
                        }

                        let columnName = root._nodeAttributesTableModel.columnHeaders(sourceColumn);

                        if(_nodeAttributesTableModel.columnIsFloatingPoint(columnName))
                            return QmlUtils.formatNumberScientific(model.display, 1);

                        // AbstractItemModel required empty values to return empty variant
                        // but TableView2 delgates cast them to undefined js objects.
                        // It's difficult to tell if the model is corrupted or accessing
                        // invalid data now as they both return undefined.
                        if(model.display === undefined)
                            return "";

                        return model.display;
                    }
                    renderType: Text.NativeRendering
                }
            }
        }

        function _updateColumnVisibility()
        {
            if(root.columnSelectionMode)
            {
                proxyModel.hiddenColumns = [];
            }
            else
            {
                proxyModel.hiddenColumns = hiddenColumns;
            }
        }

        function _resetSortFilterProxyModel()
        {
            // For reasons not fully understood, we seem to require the TableView's model to be
            // recreated whenever it has a structural change, otherwise the view and the model
            // get out of sync in exciting and unpredictable ways; hopefully we can get to the
            // bottom of this when we transition to the new TableView component
            //tableView.model = root._nodeAttributesTableModel;
        }

        Connections
        {
            target: plugin.model.nodeAttributeTableModel
            onSelectionChanged:
            {
                proxyModel.invalidateFilter();
                selectRows(0, proxyModel.rowCount() - 1);
                verticalTableViewScrollBar.position = 0;
            }
        }

        Item
        {
            clip: true

            anchors.fill: tableView
            //anchors.topMargin: columnsHeader.implicitHeight

            SlidingPanel
            {
                id: columnSelectionControls
                visible: tableView.visible

                alignment: Qt.AlignTop|Qt.AlignLeft

                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: -Constants.margin
                anchors.topMargin: -Constants.margin

                initiallyOpen: false
                disableItemWhenClosed: false

                item: Rectangle
                {
                    width: row.width
                    height: row.height

                    border.color: "black"
                    border.width: 1
                    radius: 4
                    color: "white"

                    RowLayout
                    {
                        id: row

                        // The RowLayout in a RowLayout is just a hack to get some padding
                        RowLayout
                        {
                            Layout.topMargin: Constants.padding + Constants.margin - 2
                            Layout.bottomMargin: Constants.padding
                            Layout.leftMargin: Constants.padding + Constants.margin - 2
                            Layout.rightMargin: Constants.padding

                            Button
                            {
                                text: qsTr("Show All")
                                onClicked: { root.showAllColumns(); }
                            }

                            Button
                            {
                                text: qsTr("Hide All")
                                onClicked: { root.hideAllColumns(); }
                            }

                            Button
                            {
                                text: qsTr("Show Calculated")
                                onClicked: { root.showAllCalculatedColumns(); }
                            }

                            Button
                            {
                                text: qsTr("Hide Calculated")
                                onClicked: { root.hideAllCalculatedColumns(); }
                            }

                            Button
                            {
                                text: qsTr("Done")
                                iconName: "emblem-unreadable"
                                onClicked: { columnSelectionMode = false; }
                            }
                        }
                    }
                }
            }
        }

        Component.onCompleted:
        {
            tableView._resetSortFilterProxyModel();

            populateTableMenu(tableView._tableMenu);

            root.resizeColumnsToContents();
        }

        //selectionMode: SelectionMode.ExtendedSelection

        // This is just a reference to the menu, so we can repopulate it later as necessary
        property Menu _tableMenu
    }

    }
//    ShaderEffectSource
//    {
//        id: effectSource
//        visible: columnSelectionMode

//        x: tableView.x
//        y: tableView.y + tableView.topMargin
//        width: tableView.width
//        height: tableView.height

//        sourceItem: tableView
//        sourceRect: Qt.rect(x, y, width, height)
//    }

//    FastBlur
//    {
//        visible: columnSelectionMode
//        anchors.fill: effectSource
//        source: effectSource
//        radius: 32
//    }

    Menu { id: contextMenu }

    signal rightClick();
    onRightClick: { contextMenu.popup(); }

}
