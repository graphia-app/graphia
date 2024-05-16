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

import QtQuick.Controls
import QtQuick
import QtQml
import QtQuick.Layouts
import QtQuick.Window

import app.graphia.Controls
import app.graphia.Plugins
import app.graphia.Utils
import app.graphia.Loading

BaseParameterDialog
{
    id: root

    title: qsTr("Pairwise Parameters")

    minimumWidth: 700
    minimumHeight: 500

    property bool _graphEstimatePerformed: false

    TabularDataParser
    {
        id: tabularDataParser

        onDataLoaded:
        {
            root.parameters.data = tabularDataParser.data;

            // If the third column is numerical, assume it's an edge weight
            if(model.columnCount() > 2 && model.columnIsNumerical(2))
                root.parameters.columns[2] = { "type": PairwiseColumnType.EdgeAttribute, "name": qsTr("Edge Weight") };
        }
    }

    function columnTypeRequiresName(type)
    {
        switch(type)
        {
        case PairwiseColumnType.EdgeAttribute:
        case PairwiseColumnType.SourceNodeAttribute:
        case PairwiseColumnType.TargetNodeAttribute:
            return true;
        }

        return false;
    }

    ColumnLayout
    {
        id: loadingInfo

        anchors.fill: parent
        anchors.margins: Constants.margin
        visible: !tabularDataParser.complete || tabularDataParser.failed

        Item { Layout.fillHeight: true }

        Text
        {
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            color: palette.buttonText

            text:
            {
                if(tabularDataParser.failed)
                {
                    let failureMessage = Utils.format(qsTr("Failed to Load {0}"), NativeUtils.baseFileNameForUrl(url));

                    if(tabularDataParser.failureReason.length > 0)
                        failureMessage += Utils.format(qsTr(":\n\n{0}"), tabularDataParser.failureReason);
                    else
                        failureMessage += qsTr(".");

                    return failureMessage;
                }

                return Utils.format(qsTr("Loading {0}…"), NativeUtils.baseFileNameForUrl(url));
            }
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
                onClicked: function(mouse)
                {
                    if(!tabularDataParser.failed)
                        tabularDataParser.cancelParse();

                    root.close();
                }
            }
        }

        Item { Layout.fillHeight: true }
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        visible: !loadingInfo.visible

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
                color: palette.buttonText

                text: qsTr("A graph can be created from pairwise information, that is " +
                    "to say two columns in a table may be used to indicate pairs of nodes " +
                    "which are connected by an edge. These columns are selected by clicking " +
                    "on the table headers below. Furthermore, other columns can be " +
                    "interpreted as attributes, associated either with the edge or its " +
                    "connecting nodes.")
            }

            Image
            {
                Layout.minimumWidth: 80
                Layout.minimumHeight: 80

                sourceSize.width: 80
                sourceSize.height: 80
                source: "table.svg"
            }
        }

        CheckBox
        {
            id: useFirstRowAsHeaderCheckbox
            text: qsTr("Treat First Row as Header")

            onCheckedChanged:
            {
                root.parameters.firstRowIsHeader = checked;

                // If there are any unnamed columns, use the row header for those
                if(root.parameters.firstRowIsHeader)
                {
                    let newParameters = Object.assign({}, root.parameters);

                    for(const index in newParameters.columns)
                    {
                        let type = newParameters.columns[index].type;

                        if(type === undefined || type === PairwiseColumnType.Unused)
                            continue;

                        if(type === PairwiseColumnType.SourceNode || type === PairwiseColumnType.TargetNode)
                            continue;

                        if(newParameters.columns[index].name.length === 0)
                        {
                            let headerIndex = tabularDataParser.model.index(0, index);
                            let headerText = tabularDataParser.model.data(headerIndex);
                            newParameters.columns[index].name = headerText;
                        }
                    }

                    root.parameters = newParameters;
                }
            }
        }

        DataTable
        {
            id: dataTable

            Layout.fillWidth: true
            Layout.fillHeight: true

            useFirstRowAsHeader: useFirstRowAsHeaderCheckbox.checked
            model: tabularDataParser.model

            headerDelegate: RowLayout
            {
                height: editButton.implicitHeight

                function onReused()
                {
                    attributeNameTextField.visible = false;
                    attributeNameTextField.text = "";
                }

                Label
                {
                    Layout.fillWidth: true
                    Layout.leftMargin: Constants.padding
                    Layout.rightMargin: !attributeNameTextField.visible ? Constants.padding : 0
                    Layout.preferredWidth: attributeNameTextField.visible ? 150 : -1

                    maximumLineCount: 1
                    elide: Text.ElideRight
                    textFormat: Text.StyledText

                    text:
                    {
                        if(typeof(modelColumn) === "undefined")
                            return;

                        let column = root.parameters.columns[modelColumn];
                        if(column === undefined || column.type === undefined)
                            return qsTr("⨯ <i>Unused</i>");

                        let columnName = column.name ? column.name : "";

                        if(columnName.length === 0)
                            columnName = "<b><font color=\"red\">Name Required</font></b>";

                        switch(column.type)
                        {
                        case PairwiseColumnType.Unused:                 return qsTr("⨯ <i>Unused</i>");
                        case PairwiseColumnType.SourceNode:             return qsTr("◯ <i>Source Node</i>");
                        case PairwiseColumnType.TargetNode:             return qsTr("⬤ <i>Target Node</i>");
                        case PairwiseColumnType.EdgeAttribute:          return Utils.format(qsTr("► {0}"), columnName);
                        case PairwiseColumnType.SourceNodeAttribute:    return Utils.format(qsTr("◇ {0}"), columnName);
                        case PairwiseColumnType.TargetNodeAttribute:    return Utils.format(qsTr("◆ {0}"), columnName);
                        }

                        console.log("PairwiseParameters headerDelegate unable to determine text");
                        return "";
                    }

                    // Extend the background so it fills the cell
                    leftInset: -128
                    rightInset: -128
                    topInset: -128
                    bottomInset: -128

                    background: Rectangle
                    {
                        color: headerMouseArea.containsMouse ?
                            palette.highlight : palette.button;

                        MouseArea
                        {
                            id: headerMouseArea

                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }

                    color: headerMouseArea.containsMouse ?
                        palette.highlightedText : palette.buttonText
                    padding: Constants.padding

                    PlatformMenu
                    {
                        id: typeMenu

                        PlatformMenuItem { text: qsTr("⨯ Unused");                onTriggered: { typeMenu.onTypeSelected(PairwiseColumnType.Unused); } }
                        PlatformMenuItem { text: qsTr("○ Source Node");           onTriggered: { typeMenu.onTypeSelected(PairwiseColumnType.SourceNode); } }
                        PlatformMenuItem { text: qsTr("● Target Node");           onTriggered: { typeMenu.onTypeSelected(PairwiseColumnType.TargetNode); } }
                        PlatformMenuItem { text: qsTr("► Edge Attribute");        onTriggered: { typeMenu.onTypeSelected(PairwiseColumnType.EdgeAttribute); } }
                        PlatformMenuItem { text: qsTr("◇ Source Node Attribute"); onTriggered: { typeMenu.onTypeSelected(PairwiseColumnType.SourceNodeAttribute); } }
                        PlatformMenuItem { text: qsTr("◆ Target Node Attribute"); onTriggered: { typeMenu.onTypeSelected(PairwiseColumnType.TargetNodeAttribute); } }

                        function onTypeSelected(selectedType)
                        {
                            let newParameters = Object.assign({}, root.parameters);

                            // Unset any existing source/target columns that match the column type being set
                            if(selectedType === PairwiseColumnType.SourceNode || selectedType === PairwiseColumnType.TargetNode)
                            {
                                for(const index in newParameters.columns)
                                {
                                    if(newParameters.columns[index].type === selectedType)
                                        delete newParameters.columns[index];
                                }
                            }

                            if(selectedType === PairwiseColumnType.Unused)
                                delete newParameters.columns[modelColumn];
                            else
                            {
                                if(!newParameters.columns.hasOwnProperty(modelColumn))
                                    newParameters.columns[modelColumn] = { "type": selectedType, "name": "" };
                                else
                                    newParameters.columns[modelColumn].type = selectedType;

                                if(root.columnTypeRequiresName(selectedType))
                                {
                                    if(newParameters.columns[modelColumn].name.length === 0 && newParameters.firstRowIsHeader)
                                    {
                                        let headerIndex = tabularDataParser.model.index(0, modelColumn);
                                        let headerText = tabularDataParser.model.data(headerIndex);
                                        attributeNameTextField.text = headerText;
                                    }
                                    else
                                        attributeNameTextField.text = newParameters.columns[modelColumn].name;

                                    attributeNameTextField.activate();
                                }
                                else
                                    newParameters.columns[modelColumn].name = "";
                            }

                            root.parameters = newParameters;

                            dataTable.resizeColumnToHeader(modelColumn);
                        }
                    }

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked: function(mouse)
                        {
                            typeMenu.popup(parent, 0, parent.height + 8/*padding*/);
                        }
                    }

                    TextField
                    {
                        id: attributeNameTextField

                        anchors.fill: parent

                        selectByMouse: true
                        placeholderText: qsTr("Attribute Name")

                        visible: false

                        validator: RegularExpressionValidator { regularExpression: new RegExp(NativeUtils.validAttributeNameRegex) }
                        color: attributeNameTextField.length === 0 || acceptableInput ? palette.text : Qt.red

                        function activate()
                        {
                            attributeNameTextField.selectAll();
                            attributeNameTextField.forceActiveFocus();
                            attributeNameTextField.visible = true;
                        }

                        onEditingFinished:
                        {
                            text = NativeUtils.sanitiseAttributeName(text);

                            let newParameters = Object.assign({}, root.parameters);
                            newParameters.columns[modelColumn].name = text;
                            root.parameters = newParameters;

                            attributeNameTextField.visible = false;
                            dataTable.resizeColumnToHeader(modelColumn);
                        }
                    }
                }

                FloatingButton
                {
                    id: editButton

                    Layout.rightMargin: Constants.padding

                    visible:
                    {
                        let column = root.parameters.columns[modelColumn];
                        if(column === undefined || column.type === undefined)
                            return false;

                        switch(column.type)
                        {
                        case PairwiseColumnType.EdgeAttribute:          return true;
                        case PairwiseColumnType.SourceNodeAttribute:    return true;
                        case PairwiseColumnType.TargetNodeAttribute:    return true;
                        }

                        return false;
                    }

                    icon.name: "accessories-text-editor"

                    onClicked: function(mouse)
                    {
                        if(attributeNameTextField.visible)
                        {
                            attributeNameTextField.editingFinished();
                            return;
                        }

                        attributeNameTextField.text = root.parameters.columns[modelColumn].name;
                        attributeNameTextField.activate();
                    }
                }
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            Layout.topMargin: Constants.spacing

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("OK")
                enabled: root.parametersAreValid
                onClicked: function(mouse)
                {
                    accepted();
                    root.close();
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Cancel")
                onClicked: function(mouse)
                {
                    rejected();
                    root.close();
                }
            }
        }
    }

    onInitialised:
    {
        parameters =
        {
            firstRowIsHeader: false,
            columns:
            {
                "0": { "type": PairwiseColumnType.SourceNode, "name": "" },
                "1": { "type": PairwiseColumnType.TargetNode, "name": "" }
            }
        };
    }

    property bool parametersAreValid:
    {
        let sourceNodeChosen = false;
        let targetNodeChosen = false;

        if(root.parameters.columns === undefined)
            return false;

        for(const index in root.parameters.columns)
        {
            let type = root.parameters.columns[index].type;
            let nameEmpty = root.parameters.columns[index].name.length === 0;

            if(type === undefined || nameEmpty === undefined)
                continue;

            switch(type)
            {
            case PairwiseColumnType.Unused:     continue;
            case PairwiseColumnType.SourceNode: sourceNodeChosen = true; break;
            case PairwiseColumnType.TargetNode: targetNodeChosen = true; break;

            case PairwiseColumnType.EdgeAttribute:
            case PairwiseColumnType.SourceNodeAttribute:
            case PairwiseColumnType.TargetNodeAttribute:
                if(nameEmpty)
                    return false;
            }
        }

        return sourceNodeChosen && targetNodeChosen;
    }

    onVisibleChanged:
    {
        if(visible)
        {
            if(NativeUtils.urlIsValid(root.url))
                tabularDataParser.parse(root.url);
            else
                console.log("ERROR: url is invalid");
        }
    }
}
