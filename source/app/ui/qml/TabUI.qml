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

import QtQml 2.15
import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtQuick.Window 2.15
import QtQuick.Dialogs 1.2

import Qt.labs.platform 1.0 as Labs

import SortFilterProxyModel 0.2

import app.graphia 1.0
import app.graphia.Controls 1.0
import app.graphia.Shared 1.0
import app.graphia.Shared.Controls 1.0

import "Transform"
import "Visualisation"

Item
{
    id: root

    property Application application

    property url url
    property url savedFileUrl
    property string savedFileSaver

    property bool hasBeenSaved: { return Qt.resolvedUrl(savedFileUrl).toString().length > 0; }

    property string baseFileName:
    {
        if(hasBeenSaved)
            return QmlUtils.baseFileNameForUrl(root.savedFileUrl);
        else if(Qt.resolvedUrl(root.url).toString().length > 0)
            return QmlUtils.baseFileNameForUrl(root.url);

        return "";
    }

    property string title:
    {
        let text;

        if(hasBeenSaved)
        {
            // Don't display the file extension when it's a native file
            text = QmlUtils.baseFileNameForUrlNoExtension(root.savedFileUrl);
        }
        else
            text = baseFileName;

        if(baseFileName.length > 0 && saveRequired)
            text = "*" + text;

        return text;
    }

    readonly property string appName: application !== null ? application.name : ""

    property alias document: _document

    property bool pluginPoppedOut: false
    property bool pluginMinimised: false

    onPluginMinimisedChanged:
    {
        if(pluginMinimised)
            pluginToolBarContainer.minimisingOrMinimised = true;

        pluginToolBarContainer.transitioning = true;
    }

    property alias pluginMenu0: pluginMenu0
    property alias pluginMenu1: pluginMenu1
    property alias pluginMenu2: pluginMenu2
    property alias pluginMenu3: pluginMenu3
    property alias pluginMenu4: pluginMenu4

    property var previousAction: find.previousAction
    property var nextAction: find.nextAction

    property bool saveRequired: _document.loadComplete &&
        (!hasBeenSaved || _document.saveRequired || plugin.saveRequired) &&
        !tutorial.activated // The tutorial file should never be saved

    readonly property bool panelVisible: find.showing ||
        addBookmark.showing || layoutSettings.showing

    SortFilterProxyModel
    {
        id: sharedValuesProxyModel

        property var model: null
        sourceModel: model

        filters:
        [
            ValueFilter
            {
                roleName: "hasSharedValues"
                value: true
            }
        ]
    }

    property int numAttributesWithSharedValues:
    {
        return sharedValuesProxyModel.count;
    }

    onNumAttributesWithSharedValuesChanged:
    {
        // If (for whatever reason) we end up with no shared attribute values, and
        // FBAV is showing, we need to hide it or the UI will end up in a weird state
        if(root.findType === Find.ByAttribute && numAttributesWithSharedValues <= 0)
            hideFind();
    }

    property var sharedValuesAttributeNames:
    {
        let attributeNames = [];

        for(let i = 0; i < sharedValuesProxyModel.count; i++)
            attributeNames.push(sharedValuesProxyModel.get(i, "display"));

        return attributeNames;
    }

    property string _lastSharedValueAttributeName

    function _refreshAttributesWithSharedValues()
    {
        sharedValuesProxyModel.model =
            _document.availableAttributesModel(ElementType.Node);

        if(!_document.attributeExists(_lastSharedValueAttributeName))
            _lastSharedValueAttributeName = "";
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property string backgroundColor
    }

    property bool _darkBackground: { return Qt.colorEqual(_document.contrastingColor, "white"); }
    property bool _brightBackground: { return Qt.colorEqual(_document.contrastingColor, "black"); }

    function hexToRgb(hex)
    {
        hex = hex.replace("#", "");
        let bigint = parseInt(hex, 16);
        let r = ((bigint >> 16) & 255) / 255.0;
        let g = ((bigint >> 8) & 255) / 255.0;
        let b = (bigint & 255) / 255.0;

        return { r: r, b: b, g: g };
    }

    function colorDiff(a, b)
    {
        if(a === undefined || a === null || a.length === 0)
            return 1.0;

        a = hexToRgb(a);

        let ab = 0.299 * a.r + 0.587 * a.g + 0.114 * a.b;
        let bb = 0.299 * b +   0.587 * b +   0.114 * b;

        return ab - bb;
    }

    property var _lesserContrastingColors:
    {
        let colors = [];

        if(_brightBackground)
            colors = [0.0, 0.4, 0.8];

        colors = [1.0, 0.6, 0.3];

        let color1Diff = colorDiff(visuals.backgroundColor, colors[1]);
        let color2Diff = colorDiff(visuals.backgroundColor, colors[2]);

        // If either of the colors are very similar to the background color,
        // move it towards one of the others, depending on whether it's
        // lighter or darker
        if(Math.abs(color1Diff) < 0.15)
        {
            if(color1Diff < 0.0)
                colors[1] = (colors[0] + colors[1]) * 0.5;
            else
                colors[1] = (colors[1] + colors[2]) * 0.5;
        }
        else if(Math.abs(color2Diff) < 0.15)
        {
            if(color2Diff < 0.0)
                colors[2] = (colors[2]) * 0.5;
            else
                colors[2] = (colors[1] + colors[2]) * 0.5;
        }

        return colors;
    }

    property color lessContrastingColor: { return Qt.rgba(_lesserContrastingColors[1],
                                                          _lesserContrastingColors[1],
                                                          _lesserContrastingColors[1], 1.0); }
    property color leastContrastingColor: { return Qt.rgba(_lesserContrastingColors[2],
                                                           _lesserContrastingColors[2],
                                                           _lesserContrastingColors[2], 1.0); }

    function openUrl(url, type, pluginName, parameters)
    {
        if(!_document.openUrl(url, type, pluginName, parameters))
            return false;

        root.url = url;

        if(type === "Native")
        {
            root.savedFileUrl = url;
            root.savedFileSaver = appName;
        }

        return true;
    }

    function saveAsNamedFile(desiredFileUrl, saverName)
    {
        let uiData =
        {
            lastAdvancedFindAttributeName: find.lastAdvancedFindAttributeName,
            lastFindByAttributeName: find.lastFindByAttributeName
        };

        uiData = JSON.stringify(uiData);

        let pluginUiData = plugin.save();

        if(typeof(pluginUiData) === "object")
            pluginUiData = JSON.stringify(pluginUiData);

        _document.saveFile(desiredFileUrl, saverName, uiData, pluginUiData);
    }

    Component
    {
        // We use a Component here because for whatever reason, the Labs FileDialog only seems
        // to allow you to set currentFile once. From looking at the source code it appears as
        // if setting currentFile adds to the currently selected files, rather than replaces
        // the currently selected files with a new one. Until this is fixed, we work around
        // it by simply recreating the FileDialog everytime we need one.

        id: fileSaveDialogComponent

        Labs.FileDialog
        {
            id: saveDialog

            title: qsTr("Save File…")
            fileMode: Labs.FileDialog.SaveFile
            defaultSuffix: selectedNameFilter.extensions[0]
            nameFilters:
            {
                let filters = [];
                let saverTypes = application.saverFileTypes();
                for(let i = 0; i < saverTypes.length; i++)
                    filters.push(saverTypes[i].name + " files (*." + saverTypes[i].extension + ")");

                filters.push("All files (*)");
                return filters;
            }

            onAccepted:
            {
                let saverName = "";
                let saverTypes = application.saverFileTypes();

                // If no saver is found fall back to default saver (All Files filter will do this)
                if(selectedNameFilter.index < saverTypes.length)
                    saverName = saverTypes[selectedNameFilter.index].name;
                else
                    saverName = appName;

                misc.fileSaveInitialFolder = folder.toString();
                saveAsNamedFile(file, saverName);
            }

            Connections
            {
                target: saveDialog.selectedNameFilter

                function onIndexChanged()
                {
                    let saverTypes = application.saverFileTypes();

                    if(QmlUtils.urlIsValid(saveDialog.currentFile) > 0 &&
                        selectedNameFilter.index < saverTypes.length &&
                        selectedNameFilter.extensions.length > 0)
                    {
                        saveDialog.currentFile = QmlUtils.replaceExtension(
                            saveDialog.currentFile, selectedNameFilter.extensions[0]);
                    }
                }
            }
        }
    }

    function saveAsFile()
    {
        let initialFileUrl;

        if(!hasBeenSaved)
        {
            initialFileUrl = QmlUtils.replaceExtension(root.url,
                application.nativeExtension);
        }
        else
            initialFileUrl = root.savedFileUrl;

        let fileSaveDialogObject = fileSaveDialogComponent.createObject(mainWindow,
        {
            "currentFile": initialFileUrl,
            "folder": misc.fileSaveInitialFolder !== undefined ? misc.fileSaveInitialFolder: ""
        });
        fileSaveDialogObject.open();
    }

    function saveFile()
    {
        if(!hasBeenSaved)
            saveAsFile();
        else
            saveAsNamedFile(root.savedFileUrl, savedFileSaver);
    }

    MessageDialog
    {
        id: saveConfirmDialog

        property string fileName
        property var onSaveConfirmedFunction

        title: qsTr("File Changed")
        text: qsTr("Do you want to save changes to '") + baseFileName + qsTr("'?")
        icon: StandardIcon.Warning
        standardButtons: StandardButton.Save | StandardButton.Discard | StandardButton.Cancel

        onAccepted:
        {
            if(onSaveConfirmedFunction === null)
            {
                // For some reason, onAccepted gets called twice, possibly due to QTBUG-35933?
                // Anyway, on the second call onSaveConfirmedFunction will have been nulled,
                // so we can hackily use it to detect the second call and ignore it
                return;
            }

            // Capture onSaveConfirmedFunction so that it doesn't get overwritten before
            // we use it (though in theory that can't happen)
            let proxyFn = function(fn)
            {
                return function()
                {
                    _document.saveComplete.disconnect(proxyFn);
                    fn();
                };
            }(onSaveConfirmedFunction);
            onSaveConfirmedFunction = null;

            _document.saveComplete.connect(proxyFn);
            saveFile();
        }

        onDiscard:
        {
            onSaveConfirmedFunction();
            onSaveConfirmedFunction = null;
        }
    }

    function confirmSave(onSaveConfirmedFunction)
    {
        if(saveRequired)
        {
            saveConfirmDialog.onSaveConfirmedFunction = onSaveConfirmedFunction;
            saveConfirmDialog.open();
        }
        else
            onSaveConfirmedFunction();
    }

    function selectSources()
    {
        _lastSelectType = TabUI.LS_Sources;
        _document.selectSources();
    }

    function selectSourcesOf(nodeId)
    {
        _lastSelectType = TabUI.LS_Sources;
        _document.selectSourcesOf(nodeId);
    }

    function selectTargets()
    {
        _lastSelectType = TabUI.LS_Targets;
        _document.selectTargets();
    }

    function selectTargetsOf(nodeId)
    {
        _lastSelectType = TabUI.LS_Targets;
        _document.selectTargetsOf(nodeId);
    }

    function selectNeighbours()
    {
        _lastSelectType = TabUI.LS_Neighbours;
        _document.selectNeighbours();
    }

    function selectNeighboursOf(nodeId)
    {
        _lastSelectType = TabUI.LS_Neighbours;
        _document.selectNeighboursOf(nodeId);
    }

    function selectBySharedAttributeValue(attributeName, nodeId)
    {
        _lastSelectType = TabUI.LS_BySharedValue;
        _lastSharedValueAttributeName = attributeName;

        if(typeof(nodeId) !== "undefined")
            _document.selectBySharedAttributeValue(attributeName, nodeId);
        else
            _document.selectBySharedAttributeValue(attributeName);
    }

    enum LastSelectType
    {
        LS_None,
        LS_Neighbours,
        LS_Sources,
        LS_Targets,
        LS_BySharedValue
    }

    property int _lastSelectType: TabUI.LS_None

    property bool canRepeatLastSelection:
    {
        if(_document.nodeSelectionEmpty)
            return false;

        if(root._lastSelectType === TabUI.LS_BySharedValue)
            return _lastSharedValueAttributeName.length > 0;

        return root._lastSelectType !== TabUI.LS_None;
    }

    property string repeatLastSelectionMenuText:
    {
        switch(root._lastSelectType)
        {
        default:
        case TabUI.LS_None:
            break;
        case TabUI.LS_Neighbours:
            return qsTr("Repeat Last Selection (Neighbours)");
        case TabUI.LS_Sources:
            return qsTr("Repeat Last Selection (Sources)");
        case TabUI.LS_Targets:
            return qsTr("Repeat Last Selection (Targets)");
        case TabUI.LS_BySharedValue:
            if(_lastSharedValueAttributeName.length > 0)
            {
                return qsTr("Repeat Last Selection (") +
                    _lastSharedValueAttributeName + qsTr(" Value)");
            }
            break;
        }

        return qsTr("Repeat Last Selection");
    }

    function repeatLastSelection()
    {
        switch(root._lastSelectType)
        {
        default:
        case TabUI.LS_None:
            return;
        case TabUI.LS_Neighbours:
            selectNeighbours();
            return;
        case TabUI.LS_Sources:
            selectSources();
            return;
        case TabUI.LS_Targets:
            selectTargets();
            return;
        case TabUI.LS_BySharedValue:
            selectBySharedAttributeValue(_lastSharedValueAttributeName);
            return;
        }
    }

    function screenshot() { captureScreenshot.show(); }

    function copyImageToClipboard()
    {
        graph.grabToImage(function(result)
        {
            application.copyImageToClipboard(result.image);
            _document.status = qsTr("Copied Viewport To Clipboard");
        });
    }

    function showAddBookmark()
    {
        addBookmark.show();
    }

    function gotoBookmark(name)
    {
        hideFind();
        _document.gotoBookmark(name);
    }

    function gotoAllBookmarks()
    {
        hideFind();
        _document.gotoAllBookmarks();
    }

    Labs.FileDialog
    {
        id: exportNodePositionsFileDialog
        visible: false
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: selectedNameFilter.extensions[0]
        selectedNameFilter.index: 1
        title: qsTr("Export Node Positions")
        nameFilters: ["JSON File (*.json)"]
        onAccepted:
        {
            misc.fileSaveInitialFolder = folder.toString();
            _document.saveNodePositionsToFile(file);
        }
    }

    function exportNodePositions()
    {
        exportNodePositionsFileDialog.folder = misc.fileSaveInitialFolder !== undefined ?
            misc.fileSaveInitialFolder : "";

        exportNodePositionsFileDialog.open();
    }

    function searchWebForNode(nodeId)
    {
        let nodeName = _document.nodeName(nodeId);
        let url = misc.webSearchEngineUrl.indexOf("%1") >= 0 ?
            misc.webSearchEngineUrl.arg(nodeName) : "";

        if(QmlUtils.userUrlStringIsValid(url))
            Qt.openUrlExternally(QmlUtils.urlFrom(url));
    }

    CaptureScreenshot
    {
        id: captureScreenshot
        graphView: graph
        application: root.application
    }

    ColorDialog
    {
        id: backgroundColorDialog
        title: qsTr("Select a Colour")
        onColorChanged:
        {
            visuals.backgroundColor = color;
        }
    }

    Action
    {
        id: deleteNodeAction
        icon.name: "edit-delete"
        text: qsTr("&Delete '") + contextMenu.clickedNodeName + qsTr("'")
        property bool visible: _document.editable && contextMenu.nodeWasClicked
        enabled: !_document.busy && visible
        onTriggered: { _document.deleteNode(contextMenu.clickedNodeId); }
    }

    Action
    {
        id: selectSourcesOfNodeAction
        text: qsTr("Select Sources of '") + contextMenu.clickedNodeName + qsTr("'")
        property bool visible: _document.directed && contextMenu.nodeWasClicked
        enabled: !_document.busy && visible
        onTriggered: { selectSourcesOf(contextMenu.clickedNodeId); }
    }

    Action
    {
        id: selectTargetsOfNodeAction
        text: qsTr("Select Targets of '") + contextMenu.clickedNodeName + qsTr("'")
        property bool visible: _document.directed && contextMenu.nodeWasClicked
        enabled: !_document.busy && visible
        onTriggered: { selectTargetsOf(contextMenu.clickedNodeId); }
    }

    Action
    {
        id: selectNeighboursOfNodeAction
        text: qsTr("Select Neigh&bours of '") + contextMenu.clickedNodeName + qsTr("'")
        property bool visible: contextMenu.nodeWasClicked
        enabled: !_document.busy && visible
        onTriggered: { selectNeighboursOf(contextMenu.clickedNodeId); }
    }

    SplitView
    {
        id: splitView

        anchors.fill: parent
        orientation: Qt.Vertical

        Item
        {
            id: graphItem

            SplitView.fillHeight: true
            SplitView.minimumHeight: 200

            Graph
            {
                id: graph
                anchors.fill: parent
                enabled: !_document.graphChanging

                property bool _inComponentMode: !inOverviewMode && numComponents > 1

                PlatformMenu
                {
                    id: contextMenu

                    TextMetrics
                    {
                        id: elidedNodeName

                        elide: Text.ElideMiddle
                        elideWidth: 150
                        text: contextMenu.clickedNodeId !== undefined ?
                            _document.nodeName(contextMenu.clickedNodeId) : ""
                    }

                    property var clickedNodeId
                    property string clickedNodeName: elidedNodeName.elidedText
                    property bool nodeWasClicked: clickedNodeId !== undefined ? !clickedNodeId.isNull : false
                    property bool clickedNodeIsSameAsSelection:
                    {
                        return _document.numHeadNodesSelected === 1 &&
                            nodeWasClicked &&
                            _document.nodeIsSelected(clickedNodeId);
                    }

                    PlatformMenuItem { id: delete1; visible: deleteNodeAction.visible; action: deleteNodeAction }
                    PlatformMenuItem { id: delete2; visible: deleteAction.visible && !contextMenu.clickedNodeIsSameAsSelection; action: deleteAction }
                    PlatformMenuSeparator { visible: delete1.visible || delete2.visible }

                    PlatformMenuItem { visible: _document.numNodesSelected < graph.numNodes; action: selectAllAction }
                    PlatformMenuItem { visible: _document.numNodesSelected < graph.numNodes && !graph.inOverviewMode; action: selectAllVisibleAction }
                    PlatformMenuItem { visible: !_document.nodeSelectionEmpty; action: selectNoneAction }
                    PlatformMenuItem { visible: !_document.nodeSelectionEmpty; action: invertSelectionAction }

                    PlatformMenuItem { visible: selectSourcesOfNodeAction.visible; action: selectSourcesOfNodeAction }
                    PlatformMenuItem { visible: selectTargetsOfNodeAction.visible; action: selectTargetsOfNodeAction }
                    PlatformMenuItem { visible: selectNeighboursOfNodeAction.visible; action: selectNeighboursOfNodeAction }
                    PlatformMenu
                    {
                        id: sharedValuesOfNodeContextMenu
                        enabled: !_document.busy && !hidden
                        hidden: numAttributesWithSharedValues === 0 || !contextMenu.nodeWasClicked
                        title: qsTr("Select Shared Values of '") + contextMenu.clickedNodeName + qsTr("'")
                        Instantiator
                        {
                            model: sharedValuesAttributeNames
                            PlatformMenuItem
                            {
                                text: modelData
                                onTriggered: { selectBySharedAttributeValue(text, contextMenu.clickedNodeId); }
                            }
                            onObjectAdded: sharedValuesOfNodeContextMenu.insertItem(index, object)
                            onObjectRemoved: sharedValuesOfNodeContextMenu.removeItem(object)
                        }
                    }

                    PlatformMenuItem { visible: !_document.nodeSelectionEmpty && !contextMenu.clickedNodeIsSameAsSelection &&
                        selectSourcesAction.visible; action: selectSourcesAction }
                    PlatformMenuItem { visible: !_document.nodeSelectionEmpty && !contextMenu.clickedNodeIsSameAsSelection &&
                        selectTargetsAction.visible; action: selectTargetsAction }
                    PlatformMenuItem { visible: !_document.nodeSelectionEmpty && !contextMenu.clickedNodeIsSameAsSelection &&
                        selectNeighboursAction.visible; action: selectNeighboursAction }
                    PlatformMenu
                    {
                        id: sharedValuesSelectionContextMenu
                        enabled: !_document.busy && !hidden
                        hidden: numAttributesWithSharedValues === 0 || _document.nodeSelectionEmpty ||
                            contextMenu.clickedNodeIsSameAsSelection
                        title: qsTr('Select Shared Values of Selection')
                        Instantiator
                        {
                            model: sharedValuesAttributeNames
                            PlatformMenuItem
                            {
                                text: modelData
                                onTriggered: { selectBySharedAttributeValue(text); }
                            }
                            onObjectAdded: sharedValuesSelectionContextMenu.insertItem(index, object)
                            onObjectRemoved: sharedValuesSelectionContextMenu.removeItem(object)
                        }
                    }
                    PlatformMenuItem { visible: repeatLastSelectionAction.enabled; action: repeatLastSelectionAction }

                    PlatformMenuSeparator { visible: searchWebMenuItem.visible }
                    PlatformMenuItem
                    {
                        id: searchWebMenuItem
                        visible: contextMenu.nodeWasClicked
                        text: qsTr("Search Web for '") + contextMenu.clickedNodeName + qsTr("'")
                        onTriggered: { root.searchWebForNode(contextMenu.clickedNodeId); }
                    }

                    PlatformMenuSeparator { visible: changeBackgroundColourMenuItem.visible }
                    PlatformMenuItem
                    {
                        id: changeBackgroundColourMenuItem
                        visible: !contextMenu.nodeWasClicked
                        text: qsTr("Change Background &Colour")
                        onTriggered:
                        {
                            backgroundColorDialog.color = visuals.backgroundColor;
                            backgroundColorDialog.open();
                        }
                    }
                }

                onClicked:
                {
                    if(button === Qt.RightButton)
                    {
                        contextMenu.clickedNodeId = nodeId;
                        contextMenu.popup();
                    }
                }

                Label
                {
                    id: emptyGraphLabel
                    text: qsTr("Empty Graph")
                    font.pixelSize: 48
                    color: _document.contrastingColor
                    opacity: 0.0

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter

                    states: State
                    {
                        name: "showing"
                        when: plugin.loaded && graph.numNodes <= 0
                        PropertyChanges
                        {
                            target: emptyGraphLabel
                            opacity: 1.0
                        }
                    }

                    transitions: Transition
                    {
                        to: "showing"
                        reversible: true
                        NumberAnimation
                        {
                            properties: "opacity"
                            duration: 1000
                            easing.type: Easing.InOutSine
                        }
                    }
                }

                BusyIndicator
                {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter

                    width: parent.height * 0.3
                    height: width

                    visible: !graph.initialised && plugin.loaded
                }
            }

            FloatingButton
            {
                property bool _visible: graph._inComponentMode

                Behavior on opacity { NumberAnimation { easing.type: Easing.InOutQuad } }
                opacity: _visible ? 1.0 : 0.0
                visible: opacity > 0.0

                anchors.verticalCenter: graph.verticalCenter
                anchors.left: graph.left
                anchors.margins: 20

                iconName: "go-previous"
                text: qsTr("Goto Previous Component");

                onClicked: { _document.gotoPrevComponent(); }
            }

            FloatingButton
            {
                property bool _visible: graph._inComponentMode

                Behavior on opacity { NumberAnimation { easing.type: Easing.InOutQuad } }
                opacity: _visible ? 1.0 : 0.0
                visible: opacity > 0.0

                anchors.verticalCenter: graph.verticalCenter
                anchors.right: graph.right
                anchors.margins: 20

                iconName: "go-next"
                text: qsTr("Goto Next Component");

                onClicked: { _document.gotoNextComponent(); }
            }

            RowLayout
            {
                visible: graph._inComponentMode

                anchors.horizontalCenter: graph.horizontalCenter
                anchors.bottom: graph.bottom
                anchors.margins: 20

                FloatingButton
                {
                    iconName: "edit-undo"
                    text: qsTr("Return to Overview Mode")
                    onClicked: { _document.switchToOverviewMode(); }
                }

                ColumnLayout
                {
                    Text
                    {
                        Layout.alignment: Qt.AlignHCenter

                        text:
                        {
                            return qsTr("Component ") + graph.visibleComponentIndex +
                                qsTr(" of ") + graph.numComponents;
                        }

                        color: _document.contrastingColor
                    }

                    Text
                    {
                        Layout.alignment: Qt.AlignHCenter
                        visible: _document.numInvisibleNodesSelected > 0

                        text:
                        {
                            let nodeText = _document.numInvisibleNodesSelected === 1 ? qsTr("node") : qsTr("nodes");
                            let numNodes = QmlUtils.formatNumberSIPostfix(_document.numInvisibleNodesSelected);
                            return "<i>" + qsTr("(") + numNodes +
                                qsTr(" selected ") + nodeText + qsTr(" not currently visible)") + "</i>";
                        }

                        color: _document.contrastingColor

                        textFormat: Text.StyledText
                    }
                }
            }

            Label
            {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: Constants.margin

                visible: toggleFpsMeterAction.checked

                color: _document.contrastingColor

                horizontalAlignment: Text.AlignLeft
                text: _document.fps.toFixed(1) + qsTr(" fps")
            }

            Column
            {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: Constants.margin

                GridLayout
                {
                    visible: plugin.loaded && toggleGraphMetricsAction.checked
                    anchors.left: parent.left

                    columns: 2
                    rows: 3

                    rowSpacing: 0

                    Label
                    {
                        color: _document.contrastingColor
                        text: qsTr("Nodes:")
                    }

                    Label
                    {
                        color: _document.contrastingColor
                        Layout.alignment: Qt.AlignRight
                        text:
                        {
                            let s = "";
                            let numNodes = graph.numNodes;
                            let numVisibleNodes = graph.numVisibleNodes;

                            if(numNodes >= 0)
                            {
                                s += QmlUtils.formatNumberSIPostfix(numNodes);
                                if(numVisibleNodes !== numNodes)
                                    s += " (" + QmlUtils.formatNumberSIPostfix(numVisibleNodes) + ")";
                            }

                            return s;
                        }
                    }

                    Label
                    {
                        color: _document.contrastingColor
                        text: qsTr("Edges:")
                    }

                    Label
                    {
                        color: _document.contrastingColor
                        Layout.alignment: Qt.AlignRight
                        text:
                        {
                            let s = "";
                            let numEdges = graph.numEdges;
                            let numVisibleEdges = graph.numVisibleEdges;

                            if(numEdges >= 0)
                            {
                                s += QmlUtils.formatNumberSIPostfix(numEdges);
                                if(numVisibleEdges !== numEdges)
                                    s += " (" + QmlUtils.formatNumberSIPostfix(numVisibleEdges) + ")";
                            }

                            return s;
                        }
                    }

                    Label
                    {
                        visible: !graph._inComponentMode
                        color: _document.contrastingColor
                        text: qsTr("Components:")
                    }

                    Label
                    {
                        visible: !graph._inComponentMode
                        color: _document.contrastingColor
                        Layout.alignment: Qt.AlignRight
                        text:
                        {
                            let s = "";

                            if(graph.numComponents >= 0)
                                s += QmlUtils.formatNumberSIPostfix(graph.numComponents);

                            return s;
                        }
                    }
                }
            }

            Item
            {
                clip: true

                anchors.fill: parent
                anchors.leftMargin: -Constants.margin
                anchors.rightMargin: -Constants.margin
                anchors.topMargin: -(Constants.margin * 4)
                anchors.bottomMargin: -(Constants.margin * 4)

                // @disable-check M300
                SlidingPanel
                {
                    id: findPanel

                    alignment: Qt.AlignTop|Qt.AlignLeft

                    anchors.left: parent.left
                    anchors.top: parent.top

                    initiallyOpen: false
                    disableItemWhenClosed: false

                    item: Find
                    {
                        id: find

                        document: _document

                        onShown: { findPanel.show(); }
                        onHidden: { findPanel.hide(); }
                    }
                }

                // @disable-check M300
                SlidingPanel
                {
                    id: bookmarkPanel

                    alignment: Qt.AlignTop

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top

                    initiallyOpen: false
                    disableItemWhenClosed: false

                    item: AddBookmark
                    {
                        id: addBookmark

                        document: _document

                        onShown: { bookmarkPanel.show(); }
                        onHidden: { bookmarkPanel.hide(); }
                    }
                }

                // @disable-check M300
                SlidingPanel
                {
                    id: layoutSettingsPanel

                    alignment: Qt.AlignBottom|Qt.AlignLeft

                    anchors.left: parent.left
                    anchors.bottom: parent.bottom

                    initiallyOpen: false
                    disableItemWhenClosed: false

                    item: LayoutSettings
                    {
                        id: layoutSettings

                        document: _document

                        onShown: { layoutSettingsPanel.show(); }
                        onHidden: { layoutSettingsPanel.hide(); }

                        onValueChanged: { _document.resumeLayout(); }
                    }
                }
            }

            Transforms
            {
                id: transforms
                visible: plugin.loaded
                enabled: !_document.busy

                anchors.right: parent.right
                anchors.top: parent.top

                enabledTextColor: _document.contrastingColor
                disabledTextColor: root.lessContrastingColor
                heldColor: root.leastContrastingColor

                document: _document
            }

            Visualisations
            {
                id: visualisations
                visible: plugin.loaded
                enabled: !_document.busy

                anchors.right: parent.right
                anchors.bottom: parent.bottom

                enabledTextColor: _document.contrastingColor
                disabledTextColor: root.lessContrastingColor
                heldColor: root.leastContrastingColor

                document: _document
            }
        }

        Item
        {
            id: pluginToolBarContainer
            visible: plugin.loaded && !root.pluginPoppedOut

            SplitView.fillWidth: true

            SplitView.minimumHeight:
            {
                let minimumHeight = toolBar.height;

                if(pluginToolBarContainer.minimisingOrMinimised)
                    return minimumHeight;

                if(plugin.content !== undefined && plugin.content.minimumHeight !== undefined)
                    minimumHeight += plugin.content.minimumHeight;
                else
                    minimumHeight += defaultPluginContent.minimumHeight;

                return minimumHeight;
            }

            SplitView.maximumHeight: root.pluginMinimised && !pluginToolBarContainer.transitioning ?
                toolBar.height : Number.POSITIVE_INFINITY;

            SplitView.onPreferredHeightChanged: { window.pluginSplitSize = SplitView.preferredHeight; }

            property real _lastUnminimisedHeight: 0

            onHeightChanged:
            {
                if(!pluginToolBarContainer.minimisingOrMinimised)
                {
                    _lastUnminimisedHeight = height;

                    // This is a hack to avoid some jarring visuals. In summary, Layouts seem to
                    // rely on first setting an item's dimensions past its minima/maxima, and then
                    // correcting. With most controls this is completely fine, but with the
                    // CorrelationPlot and WebView, this results in them being momentarily visible
                    // at the pre-corrected dimensions. This is probably because these items are
                    // rendered in threads, so there is a window where the uncorrected dimensions
                    // are visible. Here we delay setting the height on the plugin by a frame,
                    // so that it has had an event loop to update to the limits.
                    Qt.callLater(function()
                    {
                        let newHeight = height;

                        if(!root.pluginPoppedOut)
                            newHeight -= toolBar.height;

                        pluginContainer.height = newHeight;
                    });
                }
            }

            property bool minimisingOrMinimised: false
            property bool transitioning: false

            Behavior on SplitView.preferredHeight
            {
                // Only enable when actually transitioning or this will interfere
                // when dragging the SplitView manually
                enabled: !plugin.enabled && pluginToolBarContainer.transitioning

                SequentialAnimation
                {
                    NumberAnimation { easing.type: Easing.InOutQuad }

                    ScriptAction
                    {
                        script:
                        {
                            if(!root.pluginMinimised)
                                pluginToolBarContainer.minimisingOrMinimised = false;

                            pluginToolBarContainer.transitioning = false;
                        }
                    }
                }
            }

            ToolBar
            {
                id: toolBar

                // Stick it to the top
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                topPadding: Constants.padding
                bottomPadding: Constants.padding

                RowLayout
                {
                    anchors.fill: parent

                    Label { text: _document.pluginName }

                    Item
                    {
                        id: pluginContainerToolStrip
                        enabled: !root.pluginMinimised && !_document.busy
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    RowLayout
                    {
                        enabled: !pluginToolBarContainer.transitioning

                        ToolBarButton { action: togglePluginMinimiseAction }
                        ToolBarButton { action: togglePluginWindowAction }
                    }
                }
            }

            Item
            {
                id: pluginContainer

                anchors.top: toolBar.bottom
                // anchors.bottom is not bound, instead the height is set manually
                // in the parent's onHeightChanged; see the hack explanation above

                anchors.left: parent.left
                anchors.right: parent.right
            }
        }

        onResizingChanged:
        {
            if(!resizing && typeof plugin.content.onResized === "function")
                plugin.content.onResized();
        }
    }

    Preferences
    {
        id: window
        section: "window"
        property var pluginX
        property var pluginY
        property var pluginWidth
        property var pluginHeight
        property var pluginMaximised
        property int pluginSplitSize
        property bool pluginPoppedOut: false
    }

    Preferences
    {
        id: misc
        section: "misc"

        property var fileSaveInitialFolder
        property string webSearchEngineUrl
        property bool hasSeenTutorial
    }

    // This is only here to get at the default values of its properties
    PluginContent { id: defaultPluginContent }

    Item
    {
        id: plugin

        enabled: !pluginToolBarContainer.minimisingOrMinimised || root.pluginPoppedOut

        anchors.fill: parent

        property var model: _document.plugin
        property var content
        property bool loaded: false

        onLoadedChanged:
        {
            if(root.pluginPoppedOut)
                popOutPlugin();
            else
                popInPlugin();

            plugin.resetSaveRequired();
        }

        property bool saveRequired:
        {
            if(loaded && content !== undefined)
            {
                if(typeof(content.saveRequired) === "boolean")
                    return content.saveRequired;
                else if(typeof(content.saveRequired) === "function")
                    return content.saveRequired();
            }

            return false;
        }

        function resetSaveRequired()
        {
            if(typeof(content.saveRequired) === "boolean")
                content.saveRequired = false;
        }

        function initialise()
        {
            if(typeof(content.initialise) === "function")
                content.initialise();
        }

        function save()
        {
            plugin.resetSaveRequired();

            if(typeof(content.save) === "function")
                return content.save();

            return {};
        }

        function load(data, version)
        {
            // This is a separate function because of QTBUG-62523
            function tryParseJson(data)
            {
                // It might be JSON, or it might be a plain string; try both
                try
                {
                    data = JSON.parse(data);
                }
                catch(e)
                {
                    data = data.toString();
                }

                return data;
            }

            data = tryParseJson(data);

            if(typeof(content.load) === "function")
                content.load(data, version);
        }

        // At least one enabled direct child
        property bool enabledChildren:
        {
            for(let i = 0; i < children.length; i++)
            {
                if(children[i].enabled)
                    return true;
            }

            return false;
        }
    }

    readonly property int pluginX: pluginWindow.x
    readonly property int pluginY: pluginWindow.y

    function loadPluginWindowState()
    {
        if(window.pluginPoppedOut === undefined)
            return;

        root.pluginPoppedOut = window.pluginPoppedOut;

        if(!window.pluginPoppedOut)
            return;

        if(window.pluginWidth !== undefined &&
           window.pluginHeight !== undefined)
        {
            pluginWindow.x = window.pluginX;
            pluginWindow.y = window.pluginY;
            pluginWindow.width = window.pluginWidth;
            pluginWindow.height = window.pluginHeight;
        }

        if(window.pluginMaximised !== undefined)
        {
            pluginWindow.visibility = Utils.castToBool(window.pluginMaximised) ?
                Window.Maximized : Window.Windowed;
        }
    }

    function savePluginWindowState()
    {
        window.pluginPoppedOut = root.pluginPoppedOut;

        if(!window.pluginPoppedOut)
            return;

        window.pluginMaximised = pluginWindow.maximised;

        if(!pluginWindow.maximised)
        {
            window.pluginX = pluginWindow.x;
            window.pluginY = pluginWindow.y;
            window.pluginWidth = pluginWindow.width;
            window.pluginHeight = pluginWindow.height;
        }
    }

    property bool destructing: false

    ApplicationWindow
    {
        id: pluginWindow
        width: 800
        height: 600
        minimumWidth: 480
        minimumHeight: 480
        title: application && _document.pluginName.length > 0 ?
                   _document.pluginName + " - " + appName : "";
        visible: root.visible && root.pluginPoppedOut && plugin.loaded
        property bool maximised: visibility === Window.Maximized

        onXChanged: { if(x < 0 || x >= Screen.desktopAvailableWidth)  x = 0; }
        onYChanged: { if(y < 0 || y >= Screen.desktopAvailableHeight) y = 0; }

        onClosing:
        {
            if(visible & !destructing)
                popInPlugin();
        }

        menuBar: MenuBar
        {
            PlatformMenu { id: pluginMenu0; hidden: true; enabled: !_document.busy }
            PlatformMenu { id: pluginMenu1; hidden: true; enabled: !_document.busy }
            PlatformMenu { id: pluginMenu2; hidden: true; enabled: !_document.busy }
            PlatformMenu { id: pluginMenu3; hidden: true; enabled: !_document.busy }
            PlatformMenu { id: pluginMenu4; hidden: true; enabled: !_document.busy }
        }

        header: ToolBar
        {
            id: pluginWindowToolStrip
            topPadding: Constants.padding
            bottomPadding: Constants.padding
            visible: plugin.content !== undefined && plugin.content.toolStrip
            enabled: !_document.busy
        }

        Item
        {
            id: pluginWindowContent
            anchors.fill: parent
        }

        onWidthChanged: { windowResizedTimer.start(); }
        onHeightChanged: { windowResizedTimer.start(); }

        Timer
        {
            id: windowResizedTimer
            interval: 500

            onTriggered:
            {
                if(typeof plugin.content.onResized === "function")
                    plugin.content.onResized();
            }
        }
        Component.onCompleted:
        {
            if(x == 0 && y == 0)
            {
                x = (pluginWindow.screen.width - width) * 0.5
                y = (pluginWindow.screen.height - height) * 0.5
            }

            //FIXME: For some reason, the runtime complains about versioning if
            // this property is set statically; possibly that will work in 5.14?
            pluginWindow.transientParent = null;
        }
    }

    Component.onCompleted:
    {
        loadPluginWindowState();
    }

    Component.onDestruction:
    {
        savePluginWindowState();

        // Mild hack to get the plugin window to close before the main window
        destructing = true;
        pluginWindow.close();
    }

    function toggleMinimise()
    {
        if(root.pluginMinimised)
        {
            // When un-minimising, ensure the animation starting point of the
            // SplitView is where it should be as if the user attempts to drag
            // the handle, it can be peturbed
            pluginToolBarContainer.SplitView.preferredHeight = toolBar.height;
        }

        root.pluginMinimised = !root.pluginMinimised;

        if(!root.pluginMinimised)
            pluginToolBarContainer.SplitView.preferredHeight = pluginToolBarContainer._lastUnminimisedHeight;
        else
            pluginToolBarContainer.SplitView.preferredHeight = toolBar.height;
    }

    function popOutPlugin()
    {
        root.pluginPoppedOut = true;
        plugin.parent = pluginWindowContent;

        if(plugin.content.toolStrip)
            plugin.content.toolStrip.parent = pluginWindowToolStrip.contentItem;

        pluginWindow.x = pluginX;
        pluginWindow.y = pluginY;
    }

    function popInPlugin()
    {
        plugin.parent = pluginContainer;

        if(plugin.content.toolStrip)
            plugin.content.toolStrip.parent = pluginContainerToolStrip;

        pluginToolBarContainer.SplitView.preferredHeight = window.pluginSplitSize;

        root.pluginPoppedOut = false;
    }

    function togglePop()
    {
        if(root.pluginPoppedOut)
            root.popInPlugin();
        else
            root.popOutPlugin();
    }

    function createPluginMenu(index, menu)
    {
        if(!plugin.loaded)
            return false;

        // Check the plugin implements createMenu
        if(typeof plugin.content.createMenu === "function")
            return plugin.content.createMenu(index, menu);

        return false;
    }

    readonly property int findType: find.type
    function showFind(findType)
    {
        find.show(findType);
    }

    function hideFind()
    {
        find.hide();
    }

    function showLayoutSettings()
    {
        layoutSettings.toggle();
    }

    MessageDialog
    {
        id: errorSavingFileMessageDialog
        icon: StandardIcon.Critical
        title: qsTr("Error Saving File")
    }

    Document
    {
        id: _document
        application: root.application
        graph: graph

        property bool nodeSelectionEmpty: numNodesSelected === 0
        property bool canDeleteSelection: editable && !nodeSelectionEmpty
        property bool significantCommandInProgress: commandInProgress && !commandTimer.running
        property bool canChangeComponent: !busy && graph.numComponents > 1
        property bool hasPluginUI: pluginQmlPath.length > 0

        function shading()
        {
            return projection() === Projection.TwoDee ? shading2D() : shading3D();
        }

        function setShading(_shading)
        {
            if(projection() === Projection.TwoDee)
                setShading2D(_shading);
            else
                setShading3D(_shading);
        }

        onUiDataChanged:
        {
            uiData = JSON.parse(uiData);

            if(uiData.lastAdvancedFindAttributeName !== undefined)
                find.lastAdvancedFindAttributeName = uiData.lastAdvancedFindAttributeName;

            if(uiData.lastFindByAttributeName !== undefined)
                find.lastFindByAttributeName = uiData.lastFindByAttributeName;
        }

        onPluginQmlPathChanged:
        {
            if(_document.pluginQmlPath.length > 0)
            {
                // Destroy anything already there
                while(plugin.children.length > 0)
                    plugin.children[0].destroy();

                let pluginComponent = Qt.createComponent(_document.pluginQmlPath);

                if(pluginComponent.status !== Component.Ready)
                {
                    console.log(pluginComponent.errorString());
                    return;
                }

                plugin.content = pluginComponent.createObject(plugin);
                plugin.content._mainWindow = mainWindow;

                if(plugin.content === null)
                {
                    console.log(_document.pluginQmlPath + ": failed to create instance");
                    return;
                }

                if(plugin.content.toolStrip)
                {
                    // If the plugin toolstrip is itself a ToolBar, we are actually interested
                    // in its contentItem, otherwise we would be adding a ToolBar to a ToolBar
                    if(plugin.content.toolStrip instanceof ToolBar)
                        plugin.content.toolStrip = plugin.content.toolStrip.contentChildren[0];
                }

                plugin.initialise();

                // Restore saved data, if there is any
                if(pluginUiDataVersion >= 0)
                    plugin.load(pluginUiData, pluginUiDataVersion);

                plugin.loaded = true;
                pluginLoadComplete();
            }
        }

        onSaveComplete:
        {
            if(!success)
            {
                errorSavingFileMessageDialog.text = QmlUtils.baseFileNameForUrl(root.url) +
                        qsTr(" could not be saved.");
                errorSavingFileMessageDialog.open();
            }
            else
            {
                savedFileUrl = fileUrl;
                savedFileSaver = saverName;
                mainWindow.addToRecentFiles(fileUrl);
            }
        }

        onAttributesChanged:
        {
            // If a new attribute has been created and advanced find or FBAV have
            // not yet been used, set the new attribute as the default for both
            while(addedNames.length > 0)
            {
                let lastAddedAttributeName = addedNames[addedNames.length - 1];
                let lastAddedAttribute = attribute(lastAddedAttributeName);

                if(lastAddedAttribute.valueType === ValueType.String)
                {
                    if(find.lastAdvancedFindAttributeName.length === 0)
                        find.lastAdvancedFindAttributeName = lastAddedAttributeName;

                    if(lastAddedAttribute.sharedValues.length > 0 &&
                        find.lastFindByAttributeName.length === 0)
                    {
                        find.lastFindByAttributeName = lastAddedAttributeName;
                    }

                    break;
                }

                addedNames.pop();
            }

            _refreshAttributesWithSharedValues();
        }

        onLoadComplete:
        {
            if(success)
                _refreshAttributesWithSharedValues();

            root.loadComplete(url, success);
        }

        onGraphChanged: { root._refreshAttributesWithSharedValues(); }

        property var _comandProgressSamples: []
        property int commandSecondsRemaining

        onCommandProgressChanged:
        {
            // Reset the sample buffer if the command progress is less than the latest sample (i.e. new command)
            if(_comandProgressSamples.length > 0 && commandProgress <
                _comandProgressSamples[_comandProgressSamples.length - 1].progress)
            {
                _comandProgressSamples.length = 0;
            }

            if(commandProgress < 0)
            {
                commandSecondsRemaining = 0;
                return;
            }

            let sample = {progress: commandProgress, seconds: new Date().getTime() / 1000.0};
            _comandProgressSamples.push(sample);

            // Only keep this many samples
            while(_comandProgressSamples.length > 10)
                _comandProgressSamples.shift();

            // Require a few samples before making the calculation
            if(_comandProgressSamples.length < 5)
            {
                commandSecondsRemaining = 0;
                return;
            }

            let earliestSample = _comandProgressSamples[0];
            let latestSample = _comandProgressSamples[_comandProgressSamples.length - 1];
            let percentDelta = latestSample.progress - earliestSample.progress;
            let timeDelta = latestSample.seconds - earliestSample.seconds;
            let percentRemaining = 100.0 - commandProgress;

            commandSecondsRemaining = percentRemaining * timeDelta / percentDelta;
        }

        onCommandInProgressChanged:
        {
            if(commandInProgress)
                commandTimer.start();
            else
            {
                commandTimer.stop();
                root.commandComplete();
            }
        }
    }

    signal loadComplete(url url, bool success)
    signal pluginLoadComplete()

    signal commandStarted();
    signal commandComplete();

    Timer
    {
        id: commandTimer
        interval: 200

        onTriggered:
        {
            stop();
            root.commandStarted();
        }
    }

    function startTutorial()
    {
        tutorial.start();
    }

    Tutorial
    {
        id: tutorial

        Hubble
        {
            title: qsTr("Introduction")
            x: 10
            y: 10
            Text
            {
                textFormat: Text.StyledText
                text:
                {
                    let s = "";

                    if(!misc.hasSeenTutorial)
                    {
                        s += qsTr("As this is your first time starting ") + appName +
                            qsTr(", we have opened an example graph.<br>");
                    }

                    s += qsTr("This example graph represents the <b>London Tube System and River Buses!</b>");

                    if(!misc.hasSeenTutorial)
                    {
                        s += qsTr("<br><br>If you choose to skip the tutorial at this point, it can " +
                            "be restarted later from the <i>Help</i> menu.");
                    }

                    s += qsTr("<br><br>Click <i>Next</i> to start the tutorial.");

                    return s;
                }
            }
        }

        Hubble
        {
            title: qsTr("Nodes and Edges")
            x: (root.width * 0.5) - childrenRect.width * 0.5
            y: 10
            RowLayout
            {
                spacing: Constants.spacing
                Column
                {
                    Image
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        fillMode: Image.PreserveAspectFit
                        width: 75
                        height: 75
                        source: "qrc:///imagery/node.png"
                    }
                    Text
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("A Node")
                    }
                    Image
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        fillMode: Image.PreserveAspectFit
                        width: 100
                        height: 100
                        source: "qrc:///imagery/edge.png"
                    }
                    Text
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("An Edge")
                    }
                }
                Text
                {
                    Layout.preferredWidth: 500
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text:
                    {
                        let preamble = qsTr("A graph consists of <b>Nodes</b> and <b>Edges</b>. " +
                          "In this example, the nodes represent stops, and the edges represent a connection between two stops. " +
                          "<b>Edges</b> always have a direction, however direction can be ignored if desired. " +
                          "<b>Nodes</b> and <b>Edges</b> can also have additional information associated with them. " +
                          "We refer to these as <b>Attributes</b>.<br><br>");

                        let regularMouse = qsTr("<b>Rotate:</b> Left Click and Drag<br>" +
                          "<b>Zoom:</b> Mouse Scrollwheel<br>" +
                          "<b>Pan:</b> Right Click and Drag");

                        let macOsTrackpad = qsTr("<b>Rotate:</b> Three Finger Drag <b>or</b> Left Click and Drag<br>" +
                          "<b>Zoom:</b> Pinch Gesture <b>or</b> Mouse Scrollwheel<br>" +
                          "<b>Pan:</b> Right Click and Drag");

                        return preamble + (Qt.platform.os === "osx" ? macOsTrackpad : regularMouse);
                    }
                }
            }
        }

        Hubble
        {
            title: qsTr("Overview Mode")
            x: (root.width * 0.5) - childrenRect.width * 0.5;
            y: 10
            RowLayout
            {
                Image
                {
                    Layout.alignment: Qt.AlignVCenter
                    source: "qrc:///imagery/overview.png"
                    mipmap: true
                    fillMode: Image.PreserveAspectFit
                }

                Text
                {
                    Layout.preferredWidth: 400
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("When a graph contains multiple disconnected graphs (<b>Components</b>) ") + appName +
                          qsTr(" opens the file in Overview mode. From Overview mode all components are visible. In this graph the " +
                          "left component is the <b>London Tube map</b>, while the right component is the <b>London Riverbus " +
                          "Network</b>.<br><br>" +
                          "To focus on a particular component and hide others, <b>Double Click</b> it. " +
                          "When viewing a component, on-screen buttons appear to help you navigate. Alternatively, <b>Double Click</b> " +
                          "the background or press <b>Esc</b> to return to Overview mode.<br><br>" +
                          "<b>Component Mode:</b> Double Click Component<br>" +
                          "<b>Overview Mode:</b> Double Click Background or Press Esc<br>" +
                          "<b>Scroll Through Components:</b> PageUp, PageDown<br>" +
                          "<b>Select Node:</b> Click a Node<br>" +
                          "<b>Select Additional Nodes:</b> Hold Shift and Click a Node<br>" +
                          "<b>Select Multiple Nodes:</b> Hold Shift, Click and Drag a Box Around Nodes<br>" +
                          "<b>Focus Node:</b> Double Click a Node")
                }
            }
        }

        Hubble
        {
            id: pluginHubble
            title: qsTr("Node Attributes")
            x: 10
            y: graph.height - height - 10
            RowLayout
            {
                Text
                {
                    Layout.preferredWidth: 500
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("Information associated with the <b>selected</b> node(s) will be displayed in the attribute table. " +
                          "Attributes are sourced from the input file or calculated through the use of transforms.<br><br>" +
                          "Select a node using <b>Left Click</b> and view the node's attributes in the table below.")
                }
                Image
                {
                    source: "qrc:///imagery/attributes.png"
                }
            }
        }

        Hubble
        {
            title: qsTr("Transforms")
            target: transforms
            alignment: Qt.AlignRight | Qt.AlignBottom
            edges: Qt.RightEdge | Qt.TopEdge
            RowLayout
            {
                spacing: Constants.spacing
                Column
                {
                    Image
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:///imagery/mcl.svg"
                        mipmap: true
                        fillMode: Image.PreserveAspectFit
                        width: 150
                    }
                    Text
                    {
                        text: qsTr("An clustering transform with a colour<br>visualisation applied")
                    }
                }
                Text
                {
                    Layout.preferredWidth: 500
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("Transforms are a powerful way to modify the graph and calculate additional attributes. " +
                          "They can be used to remove nodes or edges, calculate metrics and much more. " +
                          "Transforms will appear above. They are applied in order, from top to bottom.<br><br>" +
                          "Click <b>Add Transform</b> to create a new one.<br>" +
                          "Try selecting <b>MCL Cluster</b>.<br>" +
                          "Select <b>Colour</b> for the visualisation and then click <b>OK</b>.")
                }
            }
        }

        Hubble
        {
            title: qsTr("Visualisations")
            target: visualisations
            alignment: Qt.AlignRight | Qt.AlignTop
            edges: Qt.RightEdge | Qt.BottomEdge
            RowLayout
            {
                spacing: Constants.spacing
                Column
                {
                    Image
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:///imagery/visualisations.svg"
                        mipmap: true
                        fillMode: Image.PreserveAspectFit
                        width: 200
                    }
                    Text
                    {
                        text: qsTr("A graph with Colour, Size and Text <br>visualisations applied")
                    }
                }
                Text
                {
                    Layout.preferredWidth: 500
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("Visualisations allow for displaying attribute values by modifying the appearance of the graph elements. " +
                          "Node or edge <b>Colour</b>, <b>Size</b> and <b>Text</b> can all be linked to an attribute. " +
                          "Visualisations can be created here or existing ones modified.")
                }
            }
        }

        Hubble
        {
            title: qsTr("Search Graph")
            x: 10
            y: !findPanel.hidden ? find.y + find.height + 10 : 10
            RowLayout
            {
                Text
                {
                    Layout.preferredWidth: 400
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("To perform a simple search within the graph, click <b>Find</b> on the toolbar above.<br>" +
                          "Try finding <b>Paddington</b>.")
                }
                ToolBarButton
                {
                    icon.name: findAction.icon.name
                }
            }
        }

        Hubble
        {
            title: qsTr("Find By Attribute Value")
            x: 10
            y: !findPanel.hidden ? find.y + find.height + 10 : 10
            RowLayout
            {
                Text
                {
                    Layout.preferredWidth: 400
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("If you clustered the graph using MCL, click on " +
                        "<b>Find By Attribute Value</b> to examine individual clusters.<br>")
                }
                ToolBarButton
                {
                    icon.name: findByAttributeAction.icon.name
                }
            }
        }

        Hubble
        {
            title: qsTr("Conclusion")
            x: (root.width * 0.5) - childrenRect.width * 0.5;
            y: 10
            RowLayout
            {
                spacing: Constants.spacing
                Text
                {
                    Layout.preferredWidth: 600
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("This concludes our brief introduction of ") + appName + qsTr(" using the London transport network!<br>") +
                          appName + qsTr(" can support <b>millions</b> of nodes and edges, this network is just the beginning.<br><br>" +
                          "Utilising Transforms and Visualisations is key to getting the most from your graph.<br><br>" +
                          "If you want to explore some more, please have a look at our ") +
                          QmlUtils.redirectLink("example_datasets", qsTr("example datasets")) + qsTr(".")

                    PointingCursorOnHoverLink {}
                    onLinkActivated: { Qt.openUrlExternally(link); }
                }
            }
        }
    }
}
